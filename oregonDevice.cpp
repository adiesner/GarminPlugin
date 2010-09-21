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

#include "TcxBuilder/TcxBase.h"

OregonDevice::OregonDevice()
{
    this->displayName = "Oregon";
    this->partNumber = "006-B0625-00"; // is actually an edge705
    this->fitnessData = NULL;
}

OregonDevice::~OregonDevice() {
    Log::dbg("OregonDevice destructor");
    if (this->fitnessData != NULL) {
        delete(fitnessData);
        fitnessData = NULL;
    }
}

void OregonDevice::setPathesFromConfiguration() {
    GarminFilebasedDevice::setPathesFromConfiguration();

    TiXmlElement * massStorageNode = NULL; // needed later to add ReadFitnessData Functionality

    this->fitnessFile = this->baseDirectory+"/Garmin/gpx/current/Current.gpx"; // Fallback
    // Set fitness directory from configuration
    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");

        // read partNumber
        TiXmlElement * partNbr = NULL;
        if (node!=NULL) { partNbr = node->FirstChildElement("Model"); }
        if (partNbr!=NULL) { partNbr = partNbr->FirstChildElement("PartNumber"); }
        if (partNbr!=NULL) { this->partNumber = partNbr->GetText(); }

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
    Log::dbg("Thread readFitnessData started: "+this->displayName);

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

    if (this->fitnessData == NULL) {

        TiXmlDocument doc(workFile);
        if (doc.LoadFile()) {
            this->fitnessData = new TcxBase();

            // Add author information
            TcxAuthor * author = new TcxAuthor();
            author->setPartNumber(this->partNumber);
            author->setLangId("EN");
            author->setBuild("0.0");
            author->setType("Release");
            author->setVersion("2.80");
            author->setName(this->displayName);
            *(this->fitnessData)<<author;

            TcxActivities * activities = new TcxActivities();
            *(this->fitnessData)<<activities;

            TiXmlElement * gpx = doc.FirstChildElement("gpx");
            TiXmlElement * inputTrack = gpx->FirstChildElement("trk");
            while ( inputTrack != NULL) {
                TiXmlElement * name =inputTrack->FirstChildElement("name");
                TiXmlElement * trkseg =inputTrack->FirstChildElement("trkseg");
                if ((name != NULL) && (trkseg != NULL)) {
                    string currentLapId="";
                    TcxActivity * singleActivity = new TcxActivity("");
                    singleActivity->setSportType(TrainingCenterDatabase::Other);

                    TcxCreator * creator = new TcxCreator();
                    creator->setName(this->displayName);
                    creator->setVersion("1.12");
                    *singleActivity <<creator;

                    while ( trkseg != NULL) {
                        TiXmlElement * trkpoint = trkseg->FirstChildElement("trkpt");
                        if (trkpoint != NULL) {

                            TiXmlElement * trkpointTime = trkpoint->FirstChildElement("time");

                            TcxLap * singleLap = new TcxLap();
                            *singleActivity<<singleLap;
                            singleLap->setTriggerMethod(TrainingCenterDatabase::Manual);
                            singleLap->setIntensity(TrainingCenterDatabase::Active);

                            TcxTrack *singleTrack = new TcxTrack();
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
                                string trackpointtime = "";
                                trkpointTime = trkpoint->FirstChildElement("time");
                                if (trkpointTime != NULL) {
                                    trackpointtime = trkpointTime->GetText();
                                    if (currentLapId == "") {
                                        currentLapId = trackpointtime;
                                    }
                                }
                                TcxTrackpoint *singlePoint = new TcxTrackpoint(trackpointtime, trkpoint->Attribute("lat"), trkpoint->Attribute("lon"));
                                *singleTrack << singlePoint;

                                TiXmlElement * ele = trkpoint->FirstChildElement("ele");
                                if (ele != NULL) {
                                    singlePoint->setAltitudeMeters( ele->GetText() );
                                }


                                // Extensions like HeartRate, Cadence  and Temperature
                                TiXmlElement * extensions = trkpoint->FirstChildElement("extensions");
                                if (extensions != NULL) {
                                    TiXmlElement * trkPntExt = extensions->FirstChildElement("gpxtpx:TrackPointExtension");
                                    if (trkPntExt != NULL) {
                                        TiXmlElement * heartRateData = trkPntExt->FirstChildElement("gpxtpx:hr");
                                        TiXmlElement * cadenceData = trkPntExt->FirstChildElement("gpxtpx:cad");
                                        //TiXmlElement * tempData = trkPntExt->FirstChildElement("gpxtpx:atemp"); // Ignoring Temp Data so far

                                        if (heartRateData != NULL) {
                                            singlePoint->setHeartRateBpm(heartRateData->GetText());
                                        }

                                        // Unknown how to figure out if the cadence comes from footpod or bike cadence sensor
                                        // Just asuming it is the bike cadence sensor
                                        if (cadenceData != NULL) {
                                            singlePoint-> setCadence(cadenceData->GetText());
                                            singlePoint-> setCadenceSensorType(TrainingCenterDatabase::Bike);
                                        }

                                    }
                                }
                                trkpoint = trkpoint->NextSiblingElement( "trkpt" );
                            }

                            *singleLap << singleTrack;
                        }
                        trkseg = trkseg->NextSiblingElement( "trkseg" );
                    }

                    singleActivity->setId(currentLapId);
                    *activities<<singleActivity;

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
            return;
        }
    }


    TiXmlDocument * output = this->fitnessData->getTcxDocument(readTrackData, fitnessDetailId);
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
