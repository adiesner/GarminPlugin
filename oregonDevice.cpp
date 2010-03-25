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

OregonDevice::OregonDevice()
{
    this->displayName = "Oregon";
}

OregonDevice::~OregonDevice() {
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

    // Oregon officially does not support read fitness data. Therefore add a node to support it as I implemented it
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
        this->readFitnessDataFromDevice();
    } else {
        Log::err("Work Type not implemented!");
    }
}


void OregonDevice::readFitnessDataFromDevice() {
    Log::dbg("Thread readFitnessData started");
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
                TiXmlElement * outActivity = new TiXmlElement("Activity");
                outActivity->SetAttribute("Sport", "Other");
                TiXmlElement * outId = new TiXmlElement("Id");
                outId->LinkEndChild(new TiXmlText(name->GetText()));
                outActivity->LinkEndChild(outId);

                while ( trkseg != NULL) {
                    TiXmlElement * trkpoint = trkseg->FirstChildElement("trkpt");
                    if (trkpoint != NULL) {

                        TiXmlElement * trkpointTime = trkpoint->FirstChildElement("time");
                        TiXmlElement * outLap = new TiXmlElement("Lap");
                        if (trkpointTime != NULL) {
                            outLap->SetAttribute("StartTime", trkpointTime->GetText());
                        }
                        TiXmlElement * outTrack = new TiXmlElement("Track");
                        outLap->LinkEndChild(outTrack);

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

                            outTrack->LinkEndChild(outTrackPoint);
                            trkpoint = trkpoint->NextSiblingElement( "trkpt" );
                        }
                        outActivity->LinkEndChild(outLap);
                    }
                    trkseg = trkseg->NextSiblingElement( "trkseg" );
                }
                inputTrack = inputTrack->NextSiblingElement( "trk" );
                outActivities->LinkEndChild(outActivity);
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

