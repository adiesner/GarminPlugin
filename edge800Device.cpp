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
#include "edge800Device.h"
#include <dirent.h>

#include "fit/fit_decode.hpp"
#include "fit/fit_mesg_broadcaster.hpp"

#include "gpsFunctions.h"

class Listener : public fit::FileIdMesgListener
{
   public :
      void setXmlElement(TiXmlElement * elem) {
          this->fileElement = elem;
      }

      void OnMesg(fit::FileIdMesg& mesg)
      {
         if (fileElement == NULL) { return; }

         if (mesg.GetTimeCreated() != FIT_UINT32Z_INVALID) {
            TiXmlElement * timeElem = new TiXmlElement( "CreationTime" );
            timeElem->LinkEndChild(new TiXmlText(GpsFunctions::print_dtime(mesg.GetTimeCreated())));
            fileElement->LinkEndChild(timeElem);
         }

         TiXmlElement * fitId = fileElement->FirstChildElement("FitId");
         if (fitId == NULL) {
            fitId = new TiXmlElement( "FitId" );
            fileElement->LinkEndChild( fitId );
         }

         if (mesg.GetType() != FIT_FILE_INVALID) {
            TiXmlElement * typeElem = new TiXmlElement( "Id" );
            stringstream ss;
            ss << (unsigned int)mesg.GetTimeCreated();
            typeElem->LinkEndChild(new TiXmlText(ss.str()));
            fitId->LinkEndChild(typeElem);
         }

         if (mesg.GetType() != FIT_FILE_INVALID) {
            TiXmlElement * typeElem = new TiXmlElement( "FileType" );
            stringstream ss;
            ss << (int)mesg.GetType();
            typeElem->LinkEndChild(new TiXmlText(ss.str()));
            fitId->LinkEndChild(typeElem);
         }
         if (mesg.GetManufacturer() != FIT_MANUFACTURER_INVALID) {
            TiXmlElement * manElem = new TiXmlElement( "Manufacturer" );
            stringstream ss;
            ss << mesg.GetManufacturer();
            manElem->LinkEndChild(new TiXmlText(ss.str()));
            fitId->LinkEndChild(manElem);
         }
         if (mesg.GetProduct() != FIT_UINT16_INVALID) {
            TiXmlElement * prodElem = new TiXmlElement( "Product" );
            stringstream ss;
            ss << mesg.GetProduct();
            prodElem->LinkEndChild(new TiXmlText(ss.str()));
            fitId->LinkEndChild(prodElem);
         }
         if (mesg.GetSerialNumber() != FIT_UINT32Z_INVALID) {
            TiXmlElement * serElem = new TiXmlElement( "SerialNumber" );
            stringstream ss;
            ss << mesg.GetSerialNumber();
            serElem->LinkEndChild(new TiXmlText(ss.str()));
            fitId->LinkEndChild(serElem);
         }
      }
   private :

   TiXmlElement * fileElement;

};


Edge800Device::Edge800Device()
{
    this->displayName = "Edge 800";
    this->fitnessFileExtension = "fit";
}

Edge800Device::~Edge800Device() {
    Log::dbg("Edge800Device destructor");
}

void Edge800Device::setPathesFromConfiguration() {
    GarminFilebasedDevice::setPathesFromConfiguration();

    this->fitnessDirectory = this->baseDirectory; // Fallback
    // Set fitness directory from configuration
    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("MassStorageMode"); }
        if (node!=NULL) { node = node->FirstChildElement("DataType"); }
        while ( node != NULL) {
            TiXmlElement * node2 = node->FirstChildElement("Name");
            if (node2 != NULL) {
                string nameText = node2->GetText();
                string::size_type position = nameText.find( "FIT_TYPE_", 0 );
                if (position != string::npos) {
                    node2 = node->FirstChildElement("File");
                    while (node2 != NULL) {
                        TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                        string transDir = transferDirection->GetText();
                        if ((transDir.compare("OutputFromUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
                            FitDirectory fitDir;
                            fitDir.type = nameText.substr(strlen("FIT_TYPE_"));
                            TiXmlElement * loc = NULL;
                            if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("Path"); }
                            if (node2!=NULL) {
                                fitDir.path = node2->GetText();
                            }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("FileExtension"); }
                            if (node2!=NULL) {
                                fitDir.extension = node2->GetText();
                            }
                            fitDirectoryList.push_back(fitDir);
                        }
                        node2 = node2->NextSiblingElement("File");
                    }
                }
            }
            node = node->NextSiblingElement( "DataType" );
        }
    }
}

int Edge800Device::startReadFitnessData()
{
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITNESS;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge800Device::finishReadFitnessData()
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


void Edge800Device::doWork() {
    if (this->workType == WRITEGPX) {
        this->writeGpxFile();
    } else if (this->workType == READFITNESS) {
        this->readFitnessDataFromDevice(true, "");
    } else if (this->workType == READFITNESSDIR) {
        this->readFitnessDataFromDevice(false, "");
    } else if (this->workType == READFITNESSDETAIL) {
        this->readFitnessDataFromDevice(false, this->readFitnessDetailId);
    } else if (this->workType == READFITDIRECTORY) {
        this->readFITDirectoryFromDevice();
    } else {
        Log::err("Work Type not implemented!");
    }
}

void Edge800Device::readFITDirectoryFromDevice() {
    if (Log::enabledDbg()) { Log::dbg("Thread readFITDirectory started");}

    lockVariables();
    this->threadState = 1; // Working
    unlockVariables();


    TiXmlDocument * output = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    output->LinkEndChild( decl );

    TiXmlElement * dirList = new TiXmlElement( "DirectoryListing" );
    dirList->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/DirectoryListing/v1");
    dirList->SetAttribute("RequestedPath","");
    dirList->SetAttribute("UnitId",deviceId);
    dirList->SetAttribute("VolumePrefix","");
    output->LinkEndChild( dirList );

    // For decoding of fit files
    Listener fitListener;
    fstream file;

    for (list<FitDirectory>::iterator it = fitDirectoryList.begin(); it != fitDirectoryList.end(); it++) {
        FitDirectory currentFitDir = (*it);

        DIR *dp;
        struct dirent *dirp;
        string fullPath = this->baseDirectory + "/" + currentFitDir.path;

        if((dp = opendir(fullPath.c_str())) != NULL) {

            if (Log::enabledDbg()) { Log::dbg("Searching for files in "+fullPath);}
            while ((dirp = readdir(dp)) != NULL) {
                string fileName = string(dirp->d_name);

                if (fileName.length() > currentFitDir.extension.length()) {
                    string lastFilePart = fileName.substr(fileName.length() - currentFitDir.extension.length());
                    if (strncasecmp(lastFilePart.c_str(), currentFitDir.extension.c_str(), currentFitDir.extension.length()) == 0) {
                        if (Log::enabledDbg()) { Log::dbg("Found file with correct extension: "+fileName);}
                        TiXmlElement * fileElem = new TiXmlElement( "File" );
                        fileElem->SetAttribute("IsDirectory","false");
                        fileElem->SetAttribute("Path",currentFitDir.path+'/'+fileName);


                        // Opening and parsing of fit file:
                        string fullFileName = this->baseDirectory + "/" + currentFitDir.path+'/'+fileName;
                        file.open(fullFileName.c_str(), ios::in|ios::binary);
                        if (file.is_open())
                        {
                            fitListener.setXmlElement(fileElem);
                            fit::Decode decode;
                            fit::MesgBroadcaster mesgBroadcaster;
                            mesgBroadcaster.AddListener((fit::FileIdMesgListener &)fitListener);

                            if (decode.CheckIntegrity(file))
                            {
                                try {
                                    mesgBroadcaster.Run(file);
                                    dirList->LinkEndChild( fileElem );
                                } catch (const fit::RuntimeException& e) {
                                    Log::err("Exception decoding file: "+fullFileName+" : "+e.what());
                                    delete(fileElem);
                                }
                            } else {
                                Log::err("FIT file integrity failed: "+fullFileName);
                                delete(fileElem);
                            }
                            file.close();
                        } else {
                            Log::err("Unable to open file "+fullFileName);
                            delete(fileElem);
                        }
                    } else {
                        if (Log::enabledDbg()) { Log::dbg("Wrong file extension of "+fileName);}
                    }
                }
            }
            closedir(dp);

        } else {
            Log::err("Failed to open FitnessDirectory: "+currentFitDir.path);
        }
    }

    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    output->Accept( &printer );
    string outputXml = printer.Str();
    delete(output);

    lockVariables();
    this->fitDirectoryXml = outputXml;
    this->threadState = 3; // Finished
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readFITDirectory finished"); }
    return;

}

//TODO: This function is not really used on the Edge800 (no tcx files there)
//Research if this function can be removed or if it is possible to read the binary
//fit files and convert them to tcx files
void Edge800Device::readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId) {
    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessData started"); }
/*
Thread-Status
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    string workDir;
    lockVariables();
    this->threadState = 1; // Working
    workDir = this->fitnessDirectory;
    unlockVariables();


    DIR *dp;
    struct dirent *dirp;
    vector<string> files = vector<string>();

    if((dp = opendir(workDir.c_str())) == NULL) {
        Log::err("Error opening fitness directory! "+ workDir);

        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);

    TiXmlDocument * output = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    output->LinkEndChild( decl );

    TiXmlElement * train = new TiXmlElement( "TrainingCenterDatabase" );
    train->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2");
    train->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
    train->SetAttribute("xsi:schemaLocation","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd");

    output->LinkEndChild( train );

    TiXmlElement * activities = new TiXmlElement( "Activities" );
    train->LinkEndChild( activities );

    // Loop over all files in Fitnessdirectory:
    for (unsigned int i = 0;i < files.size();i++) {
        if (files[i].find("."+this->fitnessFileExtension)!=string::npos) {
            TiXmlDocument doc(workDir + "/" + files[i]);
            if (doc.LoadFile()) {
                TiXmlElement * train = doc.FirstChildElement("TrainingCenterDatabase");
                TiXmlElement * inputActivities = train->FirstChildElement("Activities");
                while ( inputActivities != NULL) {
                    TiXmlElement * inputActivity =inputActivities->FirstChildElement("Activity");
                    while ( inputActivity != NULL) {

                        string currentLapId="";
                        TiXmlElement * idNode = inputActivity->FirstChildElement("Id");
                        if (idNode != NULL) { currentLapId = idNode->GetText(); }

                        if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(currentLapId) == 0)) {
                            TiXmlNode * newAct = inputActivity->Clone();

                            if (readTrackData) {
                                // Track data must be deleted
                                TiXmlNode * node = newAct->FirstChildElement("Lap");
                                if ((node != NULL) && (node->FirstChildElement("Track") != NULL)) { node->RemoveChild( node->FirstChildElement("Track") );}
                            }

                            activities->LinkEndChild( newAct );

                            if (Log::enabledDbg()) { Log::dbg("Adding activity "+currentLapId+" from file "+files[i]); }
                        }
                        inputActivity = inputActivity->NextSiblingElement( "Activity" );
                    }
                    inputActivities = inputActivities->NextSiblingElement( "Activities" );
                }
            } else {
                Log::err("Unable to load fitness file "+files[i]);
            }
        } else {
                if (Log::enabledDbg()) { Log::dbg("Wrong file extension of file "+files[i]); }
        }
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


string Edge800Device::getFitnessData() {
    return this->fitnessDataTcdXml;
}

int Edge800Device::startReadFitnessDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITNESSDIR;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge800Device::finishReadFitnessDirectory() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void Edge800Device::cancelReadFitnessData() {
}


int Edge800Device::startReadFitnessDetail(string id) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read fitness detail from garmin device: "+this->displayName+ " Searching for "+id);

    this->workType = READFITNESSDETAIL;
    this->readFitnessDetailId = id;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge800Device::finishReadFitnessDetail() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void Edge800Device::cancelReadFitnessDetail() {
    cancelThread();
}

int Edge800Device::startReadFITDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITDIRECTORY;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int Edge800Device::finishReadFITDirectory() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void Edge800Device::cancelReadFITDirectory() {
    if (Log::enabledDbg()) { Log::dbg("cancelReadFITDirectory called for "+this->displayName); }
    cancelThread();
}

string Edge800Device::getFITData() {
    return  fitDirectoryXml;
}

string Edge800Device::getBinaryFile(string relativeFilePath) {
    //TODO: Check for .. in file path. No website should be able to access files outside the garmin device!

    if (Log::enabledDbg()) { Log::dbg("getBinaryFile called for "+this->displayName); }
    if (Log::enabledDbg()) { Log::dbg("Opening file "+relativeFilePath); }
    string fullFilePath = this->baseDirectory + '/' + relativeFilePath;
    std::ifstream in(fullFilePath.c_str());
    if(!in) {
        Log::dbg("getBinaryFile unable to open file: "+fullFilePath);
        return "";
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

