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
#include <iostream>
#include <fstream>
#include "log.h"
#include <unistd.h>
#include <sys/stat.h>

#include <stdlib.h>
#include "oregonDevice.h"
#include <dirent.h>
#include "gpsFunctions.h"

OregonDevice::OregonDevice()
{
    this->displayName = "Oregon";
}

OregonDevice::~OregonDevice() {
    Log::dbg("OregonDevice destructor");
}

void OregonDevice::setPathesFromConfiguration() {
    GarminFilebasedDevice::setPathesFromConfiguration();

    TiXmlElement * massStorageNode = NULL; // needed later to add ReadFitnessData Functionality

    this->fitnessFile = this->baseDirectory+"/Garmin/gpx/current/Current.gpx"; // Fallback
    // Set fitness directory from configuration
    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("MassStorageMode"); massStorageNode = node; }
        if (node!=NULL) { node = node->FirstChildElement("DataType"); }
        while ( node != NULL) {
            TiXmlElement * node2 = node->FirstChildElement("Name");
            if (node2 != NULL) {
                string nameText = node2->GetText();
                if (nameText.compare("GPSData") == 0) {
                    node2 = node->FirstChildElement("File");
                    while (node2 != NULL) {

                        TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                        string transDir = transferDirection->GetText();
                        if ((transDir.compare("OutputFromUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
                            TiXmlElement * loc = NULL;
                            TiXmlElement * path = NULL;
                            TiXmlElement * basename = NULL;
                            TiXmlElement * ext = NULL;
                            if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                            if (loc!=NULL)   { path = loc->FirstChildElement("Path"); }
                            if (loc!=NULL)   { basename = loc->FirstChildElement("BaseName"); }
                            if (loc!=NULL)   { ext = loc->FirstChildElement("FileExtension"); }

                            if ((path != NULL) && (basename != NULL) && (ext != NULL)) {
                                this->fitnessFile = this->baseDirectory + "/" + path->GetText() +"/"+basename->GetText()+"."+ext->GetText();
                                Log::dbg("Fitness file is: "+this->fitnessFile);
                            }
                        }
                        node2 = node2->NextSiblingElement("File");
                    }
                }
            }
            node = node->NextSiblingElement( "DataType" );
        }
    }

    // Oregon officially does not support read fitness data.
    // It is planned in the future to read the current.gpx file
    // convert that file to fitness data
    if (massStorageNode != NULL) {
        TiXmlElement * dataTypes = new TiXmlElement( "DataType" );
        massStorageNode->LinkEndChild(dataTypes);
        TiXmlElement * name = new TiXmlElement( "Name" );
        name->LinkEndChild(new TiXmlText("FitnessHistory"));
        dataTypes->LinkEndChild(name);

        TiXmlElement * file = new TiXmlElement( "File" );
        dataTypes->LinkEndChild(file);

        TiXmlElement * spec = new TiXmlElement( "Specification" );
        file->LinkEndChild(spec);

        TiXmlElement * identifier = new TiXmlElement( "Identifier" );
        identifier->LinkEndChild(new TiXmlText("http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2"));
        spec->LinkEndChild(identifier);

        TiXmlElement * docu = new TiXmlElement( "Documentation" );
        docu->LinkEndChild(new TiXmlText("http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd"));
        spec->LinkEndChild(docu);

        TiXmlElement * loc = new TiXmlElement( "Location" );
        file->LinkEndChild(loc);

        TiXmlElement * fileEx = new TiXmlElement( "FileExtension" );
        fileEx->LinkEndChild(new TiXmlText("TCX"));
        loc->LinkEndChild(fileEx);

        TiXmlElement * transferDir = new TiXmlElement( "TransferDirection" );
        transferDir->LinkEndChild(new TiXmlText("OutputFromUnit"));
        file->LinkEndChild(transferDir);
    }
}


int OregonDevice::startReadFitnessData()
{
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITNESS;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int OregonDevice::finishReadFitnessData()
{
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


void OregonDevice::doWork() {
    if (this->workType == WRITEGPX) {
        this->writeGpxFile();
    } else if (this->workType == READFITNESS) {
        this->readFitnessDataFromDevice(true, "");
    } else if (this->workType == READFITNESSDIR) {
        this->readFitnessDataFromDevice(false, "");
    } else if (this->workType == READFITNESSDETAIL) {
        this->readFitnessDataFromDevice(true, this->readFitnessDetailId);
    } else {
        Log::err("Work Type not implemented!");
    }
}

// This is currently a hack - the device does not support the read of fitnessdata
// This function still needs testing!
void OregonDevice::readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId) {
    Log::dbg("Thread readFitnessData started");

    setlocale(LC_NUMERIC,"POSIX"); // Make sure snprintf prints a dot and not a comma as separator
/*
Thread-Status
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    string workFile;
    lockVariables();
    this->threadState = 1; // Working
    workFile = this->fitnessFile;
    unlockVariables();

    TiXmlDocument * output = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    output->LinkEndChild( decl );

    TiXmlElement * train = new TiXmlElement( "TrainingCenterDatabase" );
    train->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2");
    train->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
    train->SetAttribute("xsi:schemaLocation","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd");

    output->LinkEndChild( train );

    TiXmlElement * outActivities = new TiXmlElement( "Activities" );
    train->LinkEndChild( outActivities );

    TiXmlDocument doc(workFile);
    if (doc.LoadFile()) {
        TiXmlElement * gpx = doc.FirstChildElement("gpx");
        TiXmlElement * inputTrack = gpx->FirstChildElement("trk");
        while ( inputTrack != NULL) {
            TiXmlElement * name =inputTrack->FirstChildElement("name");
            TiXmlElement * trkseg =inputTrack->FirstChildElement("trkseg");
            if ((name != NULL) && (trkseg != NULL)) {
                string currentLapId="";

                TiXmlElement * outActivity = new TiXmlElement("Activity");
                outActivity->SetAttribute("Sport", "Other");
                TiXmlElement * outId = new TiXmlElement("Id");
                outActivity->LinkEndChild(outId);

                while ( trkseg != NULL) {
                    TiXmlElement * trkpoint = trkseg->FirstChildElement("trkpt");
                    if (trkpoint != NULL) {

                        double totalTrackLength = 0;
                        string lastCoords[2] = {"",""};

                        TiXmlElement * trkpointTime = trkpoint->FirstChildElement("time");
                        TiXmlElement * outLap = new TiXmlElement("Lap");
                        string startTime = "";
                        string endTime = "";
                        if (trkpointTime != NULL) {
                            startTime = trkpointTime->GetText();
                            if (currentLapId == "") {
                                currentLapId = startTime;
                            }
                            outLap->SetAttribute("StartTime", trkpointTime->GetText());
                        }
                        TiXmlElement * outTrack = new TiXmlElement("Track");

                        /*
                        Decode the following
                        <trkpt lat="xx.750916" lon="yy.182019">
                        <ele>243.83</ele>
                        <time>2008-02-22T16:01:15Z</time>
                        <extensions>
                        <gpxtpx:TrackPointExtension>
                        <gpxtpx:atemp>26.6</gpxtpx:atemp>
                        <gpxtpx:hr>109</gpxtpx:hr>
                        <gpxtpx:cad>47</gpxtpx:cad>
                        </gpxtpx:TrackPointExtension>
                        </extensions>
                        </trkpt>
                        */

                        while (trkpoint != NULL) {
                            TiXmlElement * outTrackPoint = new TiXmlElement("Trackpoint");

                            TiXmlElement * ele = trkpoint->FirstChildElement("ele");
                            if (ele != NULL) {
                                TiXmlElement * outElevation = new TiXmlElement("AltitudeMeters");
                                outElevation->LinkEndChild(new TiXmlText(ele->GetText()));
                                outTrackPoint->LinkEndChild(outElevation);
                            }
                            trkpointTime = trkpoint->FirstChildElement("time");
                            if (trkpointTime != NULL) {
                                endTime = trkpointTime->GetText();
                                TiXmlElement * outTime = new TiXmlElement("Time");
                                outTime->LinkEndChild(new TiXmlText(trkpointTime->GetText()));
                                outTrackPoint->LinkEndChild(outTime);
                            }

                            TiXmlElement * outPosition = new TiXmlElement("Position");
                            TiXmlElement * outLat = new TiXmlElement("LatitudeDegrees");
                            outLat->LinkEndChild(new TiXmlText(trkpoint->Attribute("lat")));
                            TiXmlElement * outLon = new TiXmlElement("LongitudeDegrees");
                            outLon->LinkEndChild(new TiXmlText(trkpoint->Attribute("lon")));
                            outPosition->LinkEndChild(outLat);
                            outPosition->LinkEndChild(outLon);
                            outTrackPoint->LinkEndChild(outPosition);

                            if (lastCoords[0] != "") {
                                totalTrackLength+= haversine_m_str(lastCoords[0], lastCoords[1], trkpoint->Attribute("lat"), trkpoint->Attribute("lon"));
                            }
                            lastCoords[0] = trkpoint->Attribute("lat");
                            lastCoords[1] = trkpoint->Attribute("lon");

                            // Extensions like HeartRate, Cadence  and Temperature
                            TiXmlElement * extensions = trkpoint->FirstChildElement("extensions");
                            if (extensions != NULL) {
                                TiXmlElement * trkPntExt = extensions->FirstChildElement("gpxtpx:TrackPointExtension");
                                if (trkPntExt != NULL) {
                                    TiXmlElement * heartRateData = trkPntExt->FirstChildElement("gpxtpx:hr");
                                    TiXmlElement * cadenceData = trkPntExt->FirstChildElement("gpxtpx:cad");
                                    //TiXmlElement * tempData = trkPntExt->FirstChildElement("gpxtpx:atemp"); // Ignoring Temp Data so far

                                    if (heartRateData != NULL) {
                                    /*
                                        Insert
                                            <HeartRateBpm xsi:type="HeartRateInBeatsPerMinute_t">
                                              <Value>119</Value>
                                            </HeartRateBpm>
                                    */
                                        TiXmlElement * outHeartRateBpm = new TiXmlElement("HeartRateBpm");
                                        outHeartRateBpm->SetAttribute("xsi:type","HeartRateInBeatsPerMinute_t");
                                        TiXmlElement * outValue = new TiXmlElement("Value");
                                        outValue->LinkEndChild(new TiXmlText(heartRateData->GetText()));
                                        outTrackPoint->LinkEndChild(outHeartRateBpm);
                                        outHeartRateBpm->LinkEndChild(outValue);
                                    }

                                    // Unknown how to figure out if the cadence comes from footpod or bike cadence sensor
                                    // Just asuming it is the bike cadence sensor
                                    if (cadenceData != NULL) {
                                    /*
                                        Insert
                                            <Cadence>71</Cadence>
                                            <Extensions>
                                                <TPX xmlns="http://www.garmin.com/xmlschemas/ActivityExtension/v2" CadenceSensor="Bike"/>
                                            </Extensions>
                                    */
                                        TiXmlElement * outCadence = new TiXmlElement("Cadence");
                                        outCadence->LinkEndChild(new TiXmlText(cadenceData->GetText()));
                                        outTrackPoint->LinkEndChild(outCadence);
                                        TiXmlElement * outExtensions = new TiXmlElement("Extensions");
                                        TiXmlElement * outTPX = new TiXmlElement("TPX");
                                        outTPX->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/ActivityExtension/v2");
                                        outTPX->SetAttribute("CadenceSensor","Bike");
                                        outTrackPoint->LinkEndChild(outExtensions);
                                        outExtensions->LinkEndChild(outTPX);
                                    }

                                }
                            }

                            outTrack->LinkEndChild(outTrackPoint);
                            trkpoint = trkpoint->NextSiblingElement( "trkpt" );
                        }


                        struct tm start, end;
                        double totalTimeSeconds = 0;
                        if ((strptime(startTime.c_str(), "%FT%TZ",&start) != NULL) &&
                            (strptime(endTime.c_str(), "%FT%TZ",&end) != NULL)) {
                            time_t tstart, tend;
                            tstart = mktime(&start);
                            tend = mktime(&end);
                            totalTimeSeconds = difftime (tend,tstart);
                        }
                        TiXmlElement * outTotalTimeSeconds = new TiXmlElement("TotalTimeSeconds");
                        char totalTimeBuf[50];
                        snprintf(&totalTimeBuf[0], sizeof(totalTimeBuf), "%.2f", totalTimeSeconds);
                        outTotalTimeSeconds->LinkEndChild(new TiXmlText(totalTimeBuf));

                        snprintf(&totalTimeBuf[0], sizeof(totalTimeBuf), "%.2f", totalTrackLength); // reuse the timeBuffer for total track length
                        TiXmlElement * outDistanceMeters = new TiXmlElement("DistanceMeters");
                        outDistanceMeters->LinkEndChild(new TiXmlText(totalTimeBuf));
                        TiXmlElement * outCalories = new TiXmlElement("Calories");
                        outCalories->LinkEndChild(new TiXmlText("0"));
                        TiXmlElement * outIntensity = new TiXmlElement("Intensity");
                        outIntensity->LinkEndChild(new TiXmlText("Active"));
                        TiXmlElement * outTriggerMethod = new TiXmlElement("TriggerMethod");
                        outTriggerMethod->LinkEndChild(new TiXmlText("Manual"));
                        outLap->LinkEndChild(outTotalTimeSeconds);
                        outLap->LinkEndChild(outDistanceMeters);
                        outLap->LinkEndChild(outCalories);
                        outLap->LinkEndChild(outIntensity);
                        outLap->LinkEndChild(outTriggerMethod);

                        outActivity->LinkEndChild(outLap);

                        if (readTrackData) {
                            outLap->LinkEndChild(outTrack);
                        } else {
                            delete outTrack;
                        }
                    }
                    trkseg = trkseg->NextSiblingElement( "trkseg" );
                }

                if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(currentLapId) == 0)) {
                    outId->LinkEndChild(new TiXmlText(currentLapId));
                    outActivities->LinkEndChild(outActivity);
                } else {
                    delete outActivity;
                }
                inputTrack = inputTrack->NextSiblingElement( "trk" );

            }
        }
    } else {
        Log::err("Unable to load fitness file "+workFile);
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        delete(output);
        return;
    }

    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    output->Accept( &printer );
    string fitnessXml = printer.Str();
    delete(output);

    lockVariables();
    this->fitnessDataTcdXml = fitnessXml;
    this->threadState = 3; // Finished
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessData finished"); }
    return;
}


string OregonDevice::getFitnessData() {
    return this->fitnessDataTcdXml;
}

int OregonDevice::startReadFITDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read FITDirectory from Oregon device");

    this->workType = READFITNESSDIR;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int OregonDevice::startReadFitnessDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read FITNESSDIR from Oregon device");

    this->workType = READFITNESSDIR;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int OregonDevice::finishReadFitnessDirectory() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void OregonDevice::cancelReadFitnessData() {
}


int OregonDevice::startReadFitnessDetail(string id) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read fitness detail from garmin device: "+this->displayName+ " Searching for "+id);

    this->workType = READFITNESSDETAIL;
    this->readFitnessDetailId = id;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int OregonDevice::finishReadFitnessDetail() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void OregonDevice::cancelReadFitnessDetail() {
    cancelThread();
}
