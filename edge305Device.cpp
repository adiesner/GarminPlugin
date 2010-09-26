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


Edge305Device::Edge305Device() : fitnessData(NULL)
{
    this->displayName = "Edge305";
    this->fitnessData = NULL;
	this->runType = 0;
}

Edge305Device::Edge305Device(string name) : fitnessData(NULL)
{
    this->displayName = name;
}

Edge305Device::~Edge305Device() {
    if (this->fitnessData != NULL) {
        delete(this->fitnessData);
        this->fitnessData = NULL;
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
    return getThreadState();
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
    } else if (this->workType == READFROMGPS) {
        this->readGpxDataFromDevice();
    } else {
        Log::err("Work Type not implemented!");
    }
}

void Edge305Device::readGpxDataFromDevice() {
    if (Log::enabledDbg()) { Log::dbg("Thread readGpxData started"); }
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

    string gpxDataXml = readGpxData();

    lockVariables();
    this->threadState = 3;
    this->gpxDataGpsXml = gpxDataXml;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessData finished"); }
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

string Edge305Device::readGpxData() {
    if (this->fitnessData == NULL) {
        this->fitnessData = readFitnessDataFromGarmin();
    }

    if (this->fitnessData != NULL) {
        transferSuccessful = true;
        TiXmlDocument * output = this->fitnessData->getGpxDocument();
        TiXmlPrinter printer;
        printer.SetIndent( "  " );
        output->Accept( &printer );
        string fitnessXml = printer.Str();
        delete(output);
        return fitnessXml;
    } else {
        transferSuccessful = false;
        return "";
    }
}

string Edge305Device::readFitnessData(bool readTrackData, string fitnessDetailId)
{
    if (this->fitnessData == NULL) {
        this->fitnessData = readFitnessDataFromGarmin();
    }

    if (this->fitnessData != NULL) {
        transferSuccessful = true;
        TiXmlDocument * output = this->fitnessData->getTcxDocument(readTrackData, fitnessDetailId);
        TiXmlPrinter printer;
        printer.SetIndent( "  " );
        output->Accept( &printer );
        string fitnessXml = printer.Str();
        delete(output);
        return fitnessXml;
    } else {
        transferSuccessful = false;
        return "";
    }
}

TcxBase * Edge305Device::readFitnessDataFromGarmin() {

   TcxBase * fitData = NULL;

   garmin_unit garmin;
   garmin_data *       data0;
   garmin_data *       data1;
   garmin_data *       data2;
   garmin_list *       runs   = NULL;
   garmin_list *       laps   = NULL;
   garmin_list *       tracks = NULL;
   if ( garmin_init(&garmin,0) != 0 ) {
        Log::dbg("Extracting data from Garmin "+this->displayName);
        garmin_data * fitnessdata = garmin_get(&garmin,GET_RUNS);
        //garmin_data * fitnessdata = garmin_load("/workout/2010/02/20100227T152346.gmn"); //Testing only

        if (fitnessdata != NULL ) {
            Log::dbg("Received data from Garmin, processing data...");

            fitData = new TcxBase();
            // Add author information
            TcxAuthor * author = new TcxAuthor();
            *(fitData)<<author;

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
                *(fitData) << printActivities(runs, laps, tracks, garmin);

                if (data0->type != data_Dlist) {
                    garmin_free_list_only(runs);
                }
                Log::dbg("Done processing data...");

            } else {
                Log::err("Some of the data read from the device was null (runs/laps/tracks)");
            }
        } else {
            Log::err("Unable to extract any data!");
        }

        garmin_free_data(fitnessdata);
        garmin_close(&garmin);
    } else {
        Log::err("Unable to open garmin device. Is it connected?");
    }

    return fitData;
}

TcxActivities * Edge305Device::printActivities(garmin_list * run, garmin_list * lap, garmin_list * track, const garmin_unit garmin) {

    TcxActivities * activities = new TcxActivities();

    garmin_list_node * runNode = run->head;

    while (runNode != NULL) {
        garmin_data *run = runNode->data;
        if ((run != NULL) && (run->type == data_D1009) && (run->data != NULL)) {
            D1009 * runData = (D1009*)run->data;

            TcxActivity * singleActivity = new TcxActivity("");
            *activities << singleActivity;
            *singleActivity << getCreator(garmin);

            switch (runData->sport_type) {
                case D1000_running:
                    this->runType = 1;
                    singleActivity->setSportType(TrainingCenterDatabase::Running);
                    break;
                case D1000_biking:
                    singleActivity->setSportType(TrainingCenterDatabase::Biking);
                    this->runType = 0;
                    break;
                default:
                    singleActivity->setSportType(TrainingCenterDatabase::Other);
                    this->runType = 2;
                    break;
            }

            bool firstLap = true;
            for ( garmin_list_node * n = lap->head; n != NULL; n = n->next ) {
                D1011 * lapData = NULL;
                if (n->data->type == data_D1011) { // Edge 305 uses this
                    lapData = (D1011*)n->data->data;
                } else if (n->data->type == data_D1015) { // Forerunner 205 uses this
                    lapData = (D1011*)n->data->data; // cast to wrong type - is safe because D1015 is identical, just a little bit longer
                }

                if (lapData != NULL) {
                    if ((lapData->index >= runData->first_lap_index) && (lapData->index <= runData->last_lap_index)) {

                        TcxLap * singleLap = getLapHeader(lapData);
                        *singleActivity<< singleLap;
                        if (firstLap) {
                            singleActivity->setId(print_dtime(lapData->start_time));
                            firstLap = false;
                        }

                        uint32 endTime = lapData->start_time + (lapData->total_time/100);

                        TcxTrack * singleTrack = new TcxTrack();
                        *singleLap << singleTrack;
                        for ( garmin_list_node * t = track->head; t != NULL; t = t->next ) {
                            if (t->data->type == data_D304) {
                                D304 * trackData = (D304 *)t->data->data;

                                if ((trackData->time >= lapData->start_time) && (trackData->time <= endTime)) {
                                    (*singleTrack) << getTrackPoint(trackData);
                                }
                            }
                        }
                    }

                } else {
                    Log::dbg("Unknown Lap Type found in data");
                }
            }

        } else {
            Log::dbg("Not a run :-(");
        }
        runNode = runNode->next;
    }

    return activities;
}

TcxCreator * Edge305Device::getCreator(const garmin_unit garmin) {
    TcxCreator *thisCreator = new TcxCreator();
    thisCreator->setName("GarminPlugin");
    stringstream ss;
    ss << garmin.id;
    thisCreator->setUnitId(ss.str());
    ss.str("");
    ss << garmin.product.product_id;
    thisCreator->setProductId(ss.str());

    int major = garmin.product.software_version / 100;
    int minor = garmin.product.software_version % 100;
    ss.str("");
    ss << major;
    stringstream ss2;
    ss2 << minor;
    thisCreator->setVersion(ss.str(),ss2.str());
    thisCreator->setBuild("0","0");
    return thisCreator;
}


TcxLap * Edge305Device::getLapHeader(D1011 * lapData) {

    TcxLap * singleLap = new TcxLap();

    //TODO: Think about letting TcxLap calculate that itself
    uint32 dur = lapData->total_time;
    stringstream ss;
    int  hun = dur % 100;
    dur -= hun;
    dur /= 100;
    ss << dur << "." << hun ;
    singleLap->setTotalTimeSeconds(ss.str());

    ss.str(""); ss << lapData->total_dist;
    singleLap->setDistanceMeters(ss.str());
    ss.str(""); ss << lapData->max_speed;
    singleLap->setMaximumSpeed(ss.str());
    ss.str(""); ss << lapData->calories;
    singleLap->setCalories(ss.str());

    if ( lapData->avg_heart_rate != 0 ) {
        ss.str("");
        ss << (unsigned int)(lapData->avg_heart_rate);
        singleLap->setAverageHeartRateBpm(ss.str());
    }
    if ( lapData->max_heart_rate != 0 ) {
        ss.str("");
        ss << (unsigned int)(lapData->max_heart_rate);
        singleLap->setMaximumHeartRateBpm(ss.str());
    }

    if (lapData->intensity == D1001_active) {
        singleLap->setIntensity(TrainingCenterDatabase::Active);
    } else {
        singleLap->setIntensity(TrainingCenterDatabase::Resting);
    }

    if (this->runType == 1) {
        singleLap->setCadenceSensorType(TrainingCenterDatabase::Footpod);
    } else {
        singleLap->setCadenceSensorType(TrainingCenterDatabase::Bike);
    }

    if ( lapData->avg_cadence != 0xff ) {
        ss.str("");
        ss << (unsigned int)(lapData->avg_cadence);
        singleLap->setCadence(ss.str());
    }

    switch (lapData->intensity) {
        case   D1011_manual: singleLap->setTriggerMethod(TrainingCenterDatabase::Manual); break;
        case   D1011_distance: singleLap->setTriggerMethod(TrainingCenterDatabase::Distance); break;
        case   D1011_location: singleLap->setTriggerMethod(TrainingCenterDatabase::Location); break;
        case   D1011_time: singleLap->setTriggerMethod(TrainingCenterDatabase::Time); break;
        case   D1011_heart_rate: singleLap->setTriggerMethod(TrainingCenterDatabase::HeartRate); break;
    }

    return singleLap;
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

TcxTrackpoint * Edge305Device::getTrackPoint ( D304 * p)
{
    TcxTrackpoint * singlePoint = new TcxTrackpoint(print_dtime(p->time));

    if (( p->posn.lat != 0x7fffffff ) && ( p->posn.lon != 0x7fffffff )) {
        stringstream lat;
        stringstream lon;
        lat << SEMI2DEG(p->posn.lat);
        lon << SEMI2DEG(p->posn.lon);
        singlePoint->setPosition(lat.str(), lon.str());
    }

    stringstream ss;
    if (p->alt < 1.0e24 ) {
        ss << p->alt;
        singlePoint->setAltitudeMeters(ss.str());
    }
    if (p->distance < 1.0e24 ) {
        ss.str("");
        ss << p->distance;
        singlePoint->setDistanceMeters(ss.str());
    }
    if ( p->heart_rate != 0 ) {
        ss.str("");
        ss << (unsigned int)(p->heart_rate);
        singlePoint->setHeartRateBpm(ss.str());
    }
    if (this->runType == 0) {
        singlePoint->setCadenceSensorType(TrainingCenterDatabase::Bike);
    } else {
        singlePoint->setCadenceSensorType(TrainingCenterDatabase::Footpod);
    }
    if ( p->cadence != 0xff ) {
        ss.str("");
        ss << (unsigned int)(p->cadence);
        singlePoint->setCadence(ss.str());
    }
    if ( p->sensor != 0 ) {
        singlePoint->setSensorState(TrainingCenterDatabase::Present);
    } else {
        singlePoint->setSensorState(TrainingCenterDatabase::Absent);
    }

    return singlePoint;
}

/*static*/
string Edge305Device::getAttachedDeviceName() {
    garmin_unit garmin;

    string deviceName = "";

    Log::dbg("Searching for garmin devices like Edge 305/Forerunner 305...");
    if ( garmin_init(&garmin,0) != 0 ) {
        if (garmin.product.product_description != NULL) // Vista HCx also gets detected by this, but returns NULL values
        {
            deviceName = filterDeviceName((string)((char*)garmin.product.product_description));
            Log::dbg("Found garmin device: "+deviceName);
        }
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
    this->transferSuccessful = false;
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
    return getThreadState();
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


int Edge305Device::startReadFromGps() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read gpx from garmin device: "+this->displayName);

    this->workType = READFROMGPS;
    this->threadState = 1;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge305Device::finishReadFromGps() {
    return getThreadState();
}

void Edge305Device::cancelReadFromGps() {
    if (Log::enabledDbg()) Log::dbg("Canceling thread to read gpx from garmin device: "+this->displayName);
    cancelThread();
}

string Edge305Device::getGpxData() {
    return this->gpxDataGpsXml;
}


int Edge305Device::getThreadState() {
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
