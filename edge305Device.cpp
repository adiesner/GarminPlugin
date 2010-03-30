/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2010 <andreas.diesner [AT] gmx [DOT] de>
 *
 * GarminPlugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GarminPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "gpsDevice.h"
#include "log.h"
#include "edge305Device.h"
#include <sstream>


Edge305Device::Edge305Device() : fitnessdata(NULL)
{
    this->displayName = "Edge305";
    this->fitnessdata = NULL;
}

Edge305Device::Edge305Device(string name) : fitnessdata(NULL)
{
    this->displayName = name;
}

Edge305Device::~Edge305Device() {
    if (fitnessdata != NULL) {
        garmin_free_data(fitnessdata);
    }
}


int Edge305Device::startReadFitnessData()
{
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device: "+this->displayName);

    this->workType = READFITNESS;
    this->threadState = 1;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge305Device::finishReadFitnessData()
{
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    Log::dbg("Inside Edge305Device::finishReadFitnessData"); //REMOVE

    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}


void Edge305Device::doWork() {
    if (this->workType == WRITEGPX) {
        Log::err("Write GPX to Edge305 not yet implemented!");
    } else if (this->workType == READFITNESS) {
        this->readFitnessDataFromDevice(true, "");
    } else if (this->workType == READFITNESSDIR) {
        this->readFitnessDataFromDevice(false, "");
    } else if (this->workType == READFITNESSDETAIL) {
        this->readFitnessDataFromDevice(true, readFitnessDetailId);
    } else {
        Log::err("Work Type not implemented!");
    }
}


void Edge305Device::readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId) {
    Log::dbg("Thread readFitnessData started");
/*
Thread-Status
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    lockVariables();
    this->threadState = 1;
    this->transferSuccessful = false;
    unlockVariables();


    string fitnessDataXml = readFitnessData(readTrackData, fitnessDetailId);

    lockVariables();
    this->threadState = 3;
    fitnessDataTcdXml = fitnessDataXml;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessData finished"); }
}


string Edge305Device::getFitnessData() {
    return fitnessDataTcdXml;
}

string Edge305Device::readFitnessData(bool readTrackData, string fitnessDetailId)
{
    garmin_unit garmin;
    garmin_data *       data0;
    garmin_data *       data1;
    garmin_data *       data2;
    garmin_list *       runs   = NULL;
    garmin_list *       laps   = NULL;
    garmin_list *       tracks = NULL;

    std::ostringstream xmlData;
    xmlData << getFitnessDataHeader();

    if ( garmin_init(&garmin,0) != 0 ) {
        Log::dbg("Extracting data from Garmin "+this->displayName);
        if (fitnessdata == NULL) {
            //fitnessdata = garmin_get(&garmin,GET_RUNS);
            //fitnessdata = garmin_load("/workout/2010/02/20100227T152346.gmn");
            fitnessdata = garmin_load("/home/andreas/Projekte/GeocacheDownloader/Firefox-Plugin/gpsbabel/2010/02/20100227T152346.gmn");
        } else {
            Log::dbg("Re-using fitnessdata from last read");
        }
        if (fitnessdata != NULL ) {
            Log::dbg("Received data from Garmin, processing data...");

            data0 = garmin_list_data(fitnessdata,0);
            data1 = garmin_list_data(fitnessdata,1);
            data2 = garmin_list_data(fitnessdata,2);

            if ( data0 != NULL && (data0->data != NULL) &&
                 data1 != NULL && (laps   = (garmin_list*)data1->data) != NULL &&
                 data2 != NULL && (tracks = (garmin_list*)data2->data) != NULL ) {
                if (data0->type == data_Dlist) {
                    runs = (garmin_list*)(data0->data);
                } else {
                   runs = garmin_list_append(NULL,data0);
                }
                xmlData << printActivities(runs, laps, tracks, garmin, readTrackData, fitnessDetailId);

                if (data0->type != data_Dlist) {
                    garmin_free_list_only(runs);
                }
                Log::dbg("Done processing data...");
                transferSuccessful = true;

            } else {
                Log::err("Some of the data read from the device was null (runs/laps/tracks)");
            }
        } else {
            Log::err("Unable to extract any data!");
        }
        garmin_close(&garmin);
    } else {
        Log::err("Unable to open garmin device. Is it connected?");
    }
    xmlData << getFitnessDataFooter();

    return xmlData.str();
}

string Edge305Device::printActivities(garmin_list * run, garmin_list * lap, garmin_list * track, const garmin_unit garmin, bool readTrackData, string fitnessDetailId) {
    std::ostringstream xmlData;

    xmlData << "<Activities>\n";

    garmin_list_node * runNode = run->head;

    while (runNode != NULL) {
        garmin_data *run = runNode->data;
        if ((run != NULL) && (run->type == data_D1009) && (run->data != NULL)) {
            D1009 * runData = (D1009*)run->data;
            std::ostringstream activityData;
            activityData << getRunHeader(runData);
            string currentLapId = "";
            bool firstLap = true;
            for ( garmin_list_node * n = lap->head; n != NULL; n = n->next ) {
                if (n->data->type == data_D1011) {
                    D1011 * lapData = (D1011*)n->data->data;

                    if ((lapData->index >= runData->first_lap_index) && (lapData->index <= runData->last_lap_index)) {

                        activityData << getLapHeader(lapData,firstLap, readTrackData);
                        if (firstLap) {
                            currentLapId = print_dtime(lapData->start_time);
                            firstLap = false;
                        }

                        if (readTrackData) {
                            uint32 endTime = lapData->start_time + (lapData->total_time/100);

                            for ( garmin_list_node * t = track->head; t != NULL; t = t->next ) {
                                if (t->data->type == data_D304) {
                                    D304 * trackData = (D304 *)t->data->data;

                                    if ((trackData->time >= lapData->start_time) && (trackData->time <= endTime)) {
                                        activityData << getTrackPoint(trackData);
                                    }
                                }
                            }
                        }

                        activityData << getLapFooter(readTrackData);
                    }

                } else {
                    Log::dbg("Unknown Lap Type found in data");
                }
            }

            activityData << getCreator(garmin);
            activityData << getRunFooter();

            if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(currentLapId) == 0)) {
                xmlData << activityData.str();
            }
        } else {
            Log::dbg("Not a run :-(");
        }
        runNode = runNode->next;
    }

    xmlData << "</Activities>\n";
    return xmlData.str();
}

string Edge305Device::getCreator(const garmin_unit garmin) {
    std::ostringstream xmlData;
    xmlData << "<Creator xsi:type=\"Device_t\">\n";
    xmlData << "<Name>Unknown</Name>\n";        // Where is that stored???
    xmlData << "<UnitId>" << garmin.id << "</UnitId>\n";
    xmlData << "<ProductID>" << garmin.product.product_id << "</ProductID>\n";
    xmlData << "<Version>\n";
    int major = garmin.product.software_version / 100;
    int minor = garmin.product.software_version % 100;
    xmlData << "<VersionMajor>"<< major <<"</VersionMajor>\n";
    xmlData << "<VersionMinor>"<< minor <<"</VersionMinor>\n";
    xmlData << "<BuildMajor>0</BuildMajor>\n"; // ??
    xmlData << "<BuildMinor>0</BuildMinor>\n"; // ??
    xmlData << "</Version>\n";
    xmlData << "</Creator>\n";
    return xmlData.str();
}

string Edge305Device::getFitnessDataHeader() {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<TrainingCenterDatabase xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2 http://www.garmin.com/xmlschemas/ActivityExtensionv2.xsd http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd\">\n\n";
}

string Edge305Device::getFitnessDataFooter() {
    return "</TrainingCenterDatabase>\n";
}

string Edge305Device::getRunHeader(D1009 * runData) {
    if (runData == NULL) { return ""; }
    std::ostringstream xmlData;
    xmlData << "<Activity Sport=\"";

    switch (runData->sport_type) {
        case D1000_running:
            xmlData << "Running";
            break;
        case D1000_biking:
            xmlData << "Biking";
            break;
        default:
            xmlData << "Other";
            break;
    }

    xmlData << "\">\n";
    return xmlData.str();
}

string Edge305Device::getRunFooter() {
    return "</Activity>\n";
}

string Edge305Device::getLapHeader(D1011 * lapData, bool firstLap, bool printTrackData) {
    std::ostringstream xmlData;

    if (firstLap) {
        xmlData << "<Id>";
        xmlData << print_dtime(lapData->start_time);
        xmlData << "</Id>\n";
    }

    xmlData << "<Lap StartTime=\"";
    xmlData << print_dtime(lapData->start_time);
    xmlData << "\">\n";

    uint32 dur = lapData->total_time;
    int  hun = dur % 100;
    dur -= hun;
    dur /= 100;
    xmlData << "<TotalTimeSeconds>" << dur << "." << hun << "</TotalTimeSeconds>\n";

    xmlData << "<DistanceMeters>" << lapData->total_dist << "</DistanceMeters>\n";
    xmlData << "<MaximumSpeed>" << lapData->max_speed << "</MaximumSpeed>\n";
    xmlData << "<Calories>" << lapData->calories << "</Calories>\n";

    if ( lapData->avg_heart_rate != 0 ) {
        xmlData << "<AverageHeartRateBpm xsi:type=\"HeartRateInBeatsPerMinute_t\">\n";
        xmlData << "<Value>" << (unsigned int)(lapData->avg_heart_rate) << "</Value>\n";
        xmlData << "</AverageHeartRateBpm>\n";
    }
    if ( lapData->max_heart_rate != 0 ) {
        xmlData << "<MaximumHeartRateBpm xsi:type=\"HeartRateInBeatsPerMinute_t\">\n";
        xmlData << "<Value>" << (unsigned int)(lapData->max_heart_rate) << "</Value>\n";
        xmlData << "</MaximumHeartRateBpm>\n";
    }

    xmlData << "<Intensity>";
    if (lapData->intensity == D1001_active) {
        xmlData << "Active";
    } else {
        xmlData << "Rest";
    }
    xmlData << "</Intensity>\n";

    if ( lapData->avg_cadence != 0xff ) {
        xmlData << "<Cadence>" << (unsigned int)(lapData->avg_cadence) << "</Cadence>\n";
    }
    xmlData << "<TriggerMethod>";
    switch (lapData->intensity) {
        case   D1011_manual: xmlData << "Manual"; break;
        case   D1011_distance: xmlData << "Distance"; break;
        case   D1011_location: xmlData << "Location"; break;
        case   D1011_time: xmlData << "Time"; break;
        case   D1011_heart_rate: xmlData << "Heart Rate"; break;
    }
    xmlData << "</TriggerMethod>\n";

    if (printTrackData) {
        xmlData << "<Track>\n";
    }

    return xmlData.str();
}


string Edge305Device::getLapFooter(bool printTrackData) {
    if (printTrackData) {
        return "</Track>\n</Lap>\n";
    }
    return "</Lap>\n";
}

string Edge305Device::print_dtime( uint32 t )
{
  time_t     tval;
  struct tm  tmval;
  char       buf[128];
  int        len;

  /*
                                  012345678901234567890123
     This will make, for example, 2007-04-20T23:55:01-0700, but that date
     isn't quite ISO 8601 compliant.  We need to stick a ':' in the time
     zone between the hours and the minutes.
  */

  tval = t + TIME_OFFSET;
  //localtime_r(&tval,&tmval);
  gmtime_r(&tval,&tmval);
  strftime(buf,sizeof(buf)-1,"%FT%TZ",&tmval);

  /*
     If the last character is a 'Z', don't do anything.  Otherwise, we
     need to move the last two characters out one and stick a colon in
     the vacated spot.  Let's not forget the trailing '\0' that needs to
     be moved as well.
  */

  len = strlen(buf);
  if ( len > 0 && buf[len-1] != 'Z' ) {
    memmove(buf+len-1,buf+len-2,3);
    buf[len-2] = ':';
  }

  return (string)buf;
}

string Edge305Device::getTrackPoint ( D304 * p)
{
    std::ostringstream xmlData;
    xmlData << "<Trackpoint>\n";
    xmlData << "<Time>" << print_dtime(p->time) << "</Time>\n";

    if (( p->posn.lat != 0x7fffffff ) && ( p->posn.lon != 0x7fffffff )) {
        xmlData << "<Position>\n";
        xmlData << "<LatitudeDegrees>" << SEMI2DEG(p->posn.lat) << "</LatitudeDegrees>\n";
        xmlData << "<LongitudeDegrees>" << SEMI2DEG(p->posn.lon) << "</LongitudeDegrees>\n";
        xmlData << "</Position>\n";
    }

    if (p->alt < 1.0e24 ) {
        xmlData << "<AltitudeMeters>" << p->alt << "</AltitudeMeters>\n";
    }
    if (p->distance < 1.0e24 ) {
        xmlData << "<DistanceMeters>" << p->distance << "</DistanceMeters>\n";
    }
    if ( p->heart_rate != 0 ) {
        xmlData << "<HeartRateBpm xsi:type=\"HeartRateInBeatsPerMinute_t\">\n";
        xmlData << "<Value>" << (unsigned int)(p->heart_rate) << "</Value>\n";
        xmlData << "</HeartRateBpm>\n";
    }
    if ( p->cadence != 0xff ) {
        xmlData << "<Cadence>" << (unsigned int)(p->cadence) << "</Cadence>\n";
    }
    if ( p->sensor != 0 ) {
        xmlData << "<SensorState>Present</SensorState>\n";
    } else {
        xmlData << "<SensorState>Absent</SensorState>\n";
    }

    if ( p->cadence != 0xff ) {
    xmlData << "<Extensions>\n";
    xmlData << "<TPX xmlns=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2\" CadenceSensor=\"Bike\"/>\n";
    xmlData << "</Extensions>\n";
    }
    xmlData << "</Trackpoint>\n";

    return xmlData.str();
}

/*static*/
string Edge305Device::getAttachedDeviceName() {
    garmin_unit garmin;

    string deviceName = "";

    Log::dbg("Searching for garmin devices like Edge 305/Forerunner 305...");
    if ( garmin_init(&garmin,0) != 0 ) {
        deviceName = filterDeviceName((string)((char*)garmin.product.product_description));
        Log::dbg("Found garmin device: "+deviceName);
        garmin_close(&garmin);
    }
    return deviceName;
}



string Edge305Device::getDeviceDescription() const {

    if (Log::enabledDbg()) Log::dbg("GpsDevice::getDeviceDescription() "+this->displayName);
/*

<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<Device xmlns="http://www.garmin.com/xmlschemas/GarminDevice/v2" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd">

  <Model>
    <PartNumber>006-B0450-00</PartNumber>
    <SoftwareVersion>320</SoftwareVersion>
    <Description>EDGE305 Software Version 3.20</Description>
  </Model>

  <Id>3305091776</Id>
  <DisplayName>Your name</DisplayName>

  <MassStorageMode>
    <DataType>
      <Name>GPSData</Name>
      <File>
        <Specification>
          <Identifier>http://www.topografix.com/GPX/1/1</Identifier>
          <Documentation>http://www.topografix.com/GPX/1/1/gpx.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>GPX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>FitnessHistory</Name>
      <File>
        <Specification>
          <Identifier>http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2</Identifier>
          <Documentation>http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>TCX</FileExtension>
        </Location>
        <TransferDirection>OutputFromUnit</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>FitnessUserProfile</Name>
      <File>
        <Specification>
          <Identifier>http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2</Identifier>
          <Documentation>http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd</Documentation>
        </Specification>
        <Location>
          <BaseName>UserProfile</BaseName>
          <FileExtension>TCX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>FitnessCourses</Name>
      <File>
        <Specification>
          <Identifier>http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2</Identifier>
          <Documentation>http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>TCX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>FitnessWorkouts</Name>
      <File>
        <Specification>
          <Identifier>http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2</Identifier>
          <Documentation>http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>TCX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    <UpdateFile>
      <PartNumber>006-B0450-00</PartNumber>
      <Version>
        <Major>0</Major>
        <Minor>0</Minor>
      </Version>
      <Description>Missing</Description>
    </UpdateFile>
    <UpdateFile>
      <PartNumber>006-B0478-00</PartNumber>
      <Version>
        <Major>0</Major>
        <Minor>0</Minor>
      </Version>
      <Description>Missing</Description>
    </UpdateFile>
  </MassStorageMode>

  <GarminMode>
    <Protocols>
      <Application Id="918">
        <DataType>918</DataType>
      </Application>
    </Protocols>
    <Extensions>
      <GarminModeExtension xmlns="http://www.garmin.com/xmlschemas/GarminDeviceExtensions/v3">
        <MemoryRegion>
          <Id>5</Id>
          <Version>
            <Major>0</Major>
            <Minor>0</Minor>
          </Version>
          <Description>Missing</Description>
          <ExpectedPartNumber>006-B0450-00</ExpectedPartNumber>
          <CurrentPartNumber>006-B0450-00</CurrentPartNumber>
          <IsErased>true</IsErased>
          <IsUserUpdateable>true</IsUserUpdateable>
        </MemoryRegion>
        <MemoryRegion>
          <Id>14</Id>
          <Version>
            <Major>3</Major>
            <Minor>20</Minor>
          </Version>
          <Description>EDGE305</Description>
          <ExpectedPartNumber>006-B0450-00</ExpectedPartNumber>
          <CurrentPartNumber>006-B0450-00</CurrentPartNumber>
          <IsUserUpdateable>true</IsUserUpdateable>
        </MemoryRegion>
        <MemoryRegion>
          <Id>246</Id>
          <Version>
            <Major>0</Major>
            <Minor>0</Minor>
          </Version>
          <Description>Missing</Description>
          <ExpectedPartNumber>006-B0478-00</ExpectedPartNumber>
          <CurrentPartNumber>006-B0478-00</CurrentPartNumber>
          <IsErased>true</IsErased>
          <IsUserUpdateable>true</IsUserUpdateable>
        </MemoryRegion>
      </GarminModeExtension>
    </Extensions>
  </GarminMode>

</Device>


*/
    garmin_unit garmin;
    if ( garmin_init(&garmin,0) != 0 ) {
        garmin_close(&garmin);
    } else {
        Log::err("Opening of garmin device failed. No longer attached!?");
        return "";
    }


    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
    doc.LinkEndChild( decl );

    /*<Device xmlns="http://www.garmin.com/xmlschemas/GarminDevice/v2"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd">*/

	TiXmlElement * device = new TiXmlElement( "Device" );
    device->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/GarminDevice/v2");
    device->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    device->SetAttribute("xsi:schemaLocation", "http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd");
    doc.LinkEndChild( device );

/*<Model>
    <PartNumber>006-B0450-00</PartNumber>
    <SoftwareVersion>320</SoftwareVersion>
    <Description>EDGE305 Software Version 3.20</Description>
  </Model> */
	TiXmlElement * model = new TiXmlElement( "Model" );
	TiXmlElement * partnumber = new TiXmlElement( "PartNumber" );
	partnumber->LinkEndChild(new TiXmlText("006-B0450-00"));
	TiXmlElement * version = new TiXmlElement( "SoftwareVersion" );
	std::stringstream ss;
	ss << garmin.product.software_version;
	version->LinkEndChild(new TiXmlText( ss.str() ));
	TiXmlElement * descr = new TiXmlElement( "Description" );
	descr->LinkEndChild(new TiXmlText(this->displayName));
	model->LinkEndChild(partnumber);
	model->LinkEndChild(version);
	model->LinkEndChild(descr);
    device->LinkEndChild( model );

/*  <Id>3333333333</Id> */
	TiXmlElement * id = new TiXmlElement( "Id" );
	ss.str(""); // empty stringstream
	ss << garmin.id;
	id->LinkEndChild(new TiXmlText(ss.str()));
	device->LinkEndChild(id);
/*  <DisplayName>Your name</DisplayName>*/
	TiXmlElement * dispName = new TiXmlElement( "DisplayName" );
	dispName->LinkEndChild(new TiXmlText(this->displayName));
	device->LinkEndChild(dispName);

    TiXmlElement * massStorage = new TiXmlElement( "MassStorageMode" );
    device->LinkEndChild(massStorage);

/*
    <DataType>
      <Name>GPSData</Name>
      <File>
        <Specification>
          <Identifier>http://www.topografix.com/GPX/1/1</Identifier>
          <Documentation>http://www.topografix.com/GPX/1/1/gpx.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>GPX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
*/

    TiXmlElement * dataTypes = new TiXmlElement( "DataType" );
    massStorage->LinkEndChild(dataTypes);
    TiXmlElement * name = new TiXmlElement( "Name" );
   	name->LinkEndChild(new TiXmlText("GPSData"));
    dataTypes->LinkEndChild(name);

    TiXmlElement * file = new TiXmlElement( "File" );
    dataTypes->LinkEndChild(file);
    TiXmlElement * spec = new TiXmlElement( "Specification" );
    file->LinkEndChild(spec);

    TiXmlElement * identifier = new TiXmlElement( "Identifier" );
    identifier->LinkEndChild(new TiXmlText("http://www.topografix.com/GPX/1/1"));
    spec->LinkEndChild(identifier);

    TiXmlElement * docu = new TiXmlElement( "Documentation" );
   	docu->LinkEndChild(new TiXmlText("http://www.topografix.com/GPX/1/1/gpx.xsd"));
    spec->LinkEndChild(docu);

    TiXmlElement * loc = new TiXmlElement( "Location" );
    file->LinkEndChild(loc);

    TiXmlElement * fileEx = new TiXmlElement( "FileExtension" );
   	fileEx->LinkEndChild(new TiXmlText("GPX"));
    loc->LinkEndChild(fileEx);

    TiXmlElement * transferDir = new TiXmlElement( "TransferDirection" );
    transferDir->LinkEndChild(new TiXmlText("InputOutput"));
    file->LinkEndChild(transferDir);


    /*
    <DataType>
      <Name>FitnessHistory</Name>
      <File>
        <Specification>
          <Identifier>http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2</Identifier>
          <Documentation>http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd</Documentation>
        </Specification>
        <Location>
          <FileExtension>TCX</FileExtension>
        </Location>
        <TransferDirection>OutputFromUnit</TransferDirection>
      </File>
    </DataType>
    */
    dataTypes = new TiXmlElement( "DataType" );
    massStorage->LinkEndChild(dataTypes);
    name = new TiXmlElement( "Name" );
   	name->LinkEndChild(new TiXmlText("FitnessHistory"));
    dataTypes->LinkEndChild(name);

    file = new TiXmlElement( "File" );
    dataTypes->LinkEndChild(file);

    spec = new TiXmlElement( "Specification" );
    file->LinkEndChild(spec);

    identifier = new TiXmlElement( "Identifier" );
    identifier->LinkEndChild(new TiXmlText("http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2"));
    spec->LinkEndChild(identifier);

    docu = new TiXmlElement( "Documentation" );
   	docu->LinkEndChild(new TiXmlText("http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd"));
    spec->LinkEndChild(docu);

    loc = new TiXmlElement( "Location" );
    file->LinkEndChild(loc);

    fileEx = new TiXmlElement( "FileExtension" );
   	fileEx->LinkEndChild(new TiXmlText("TCX"));
    loc->LinkEndChild(fileEx);

    transferDir = new TiXmlElement( "TransferDirection" );
    transferDir->LinkEndChild(new TiXmlText("InputOutput"));
    file->LinkEndChild(transferDir);




    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	doc.Accept( &printer );
    string str = printer.Str();

    if (Log::enabledDbg()) Log::dbg("GpsDevice::getDeviceDescription() Done: "+this->displayName );
    return str;


}

int Edge305Device::startWriteToGps(string filename, string xml) {
    Log::err("Write to Edge305 not yet implemented!");
    return 0;
}

int Edge305Device::finishWriteToGps() {
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    return 3;
}

void Edge305Device::cancelWriteToGps() {
}

int Edge305Device::getTransferSucceeded() {
    return this->transferSuccessful;
}

void Edge305Device::setStorageCommand(string cmd) {
}

MessageBox * Edge305Device::getMessage() {
    return NULL;
}

void Edge305Device::userAnswered(const int answer) {
}

bool Edge305Device::isDeviceAvailable() {
    garmin_unit garmin;
    if ( garmin_init(&garmin,0) != 0 ) {
        garmin_close(&garmin);
        return true;
    }
    return false;
}

// At least my garmin has unprintable characters at the end, sometimes even \0
// So cut all unprintable characters at the end
/*static*/
string Edge305Device::filterDeviceName(string name) {
    int cutBytes = 0;
    for (int i=name.length()-1; i >= 0 ; i--)
    {
        char ch = name[i];
        if ((((int)ch) < 32) || (((int)ch) > 127)) { // printable characters
            cutBytes ++;
        }
    }
    return name.substr(0,name.length()-cutBytes);
}



int Edge305Device::startReadFITDirectory() {
    // do nothing so far... startReadFitnessDirectory will be called anyway
    return 0;
}

int Edge305Device::startReadFitnessDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read fitness dir from garmin device: "+this->displayName);

    this->workType = READFITNESSDIR;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge305Device::finishReadFitnessDirectory() {
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void Edge305Device::cancelReadFitnessData() {
    cancelThread();
}

int Edge305Device::startReadFitnessDetail(string id) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read fitness detail from garmin device: "+this->displayName+ " Searching for "+id);

    this->workType = READFITNESSDETAIL;
    this->readFitnessDetailId = id;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge305Device::finishReadFitnessDetail() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void Edge305Device::cancelReadFitnessDetail() {
    cancelThread();
}

