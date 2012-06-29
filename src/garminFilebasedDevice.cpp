/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2011 <garminplugin [AT] andreas.diesner [DOT] de>
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
#include "garminFilebasedDevice.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include "sys/statfs.h"
#include <limits.h>
#include <dirent.h>
#include <vector>
#include <stack>

#include "gpsFunctions.h"

#include <openssl/md5.h>

#include <unistd.h>

// needed for sort
#include <algorithm>

GarminFilebasedDevice::GarminFilebasedDevice()
: GpsDevice("")
, fitFileElement(0)
{
}

GarminFilebasedDevice::~GarminFilebasedDevice() {
    if (this->deviceDescription != NULL) {
        delete(deviceDescription);
        this->deviceDescription = NULL;
    }
}

void GarminFilebasedDevice::fitMsgReceived(FitMsg *msg) {
    if (this->fitFileElement == NULL) { return; }

    if (msg->GetType() == FIT_MESSAGE_FILE_ID) {
        FitMsg_File_ID *fileid = dynamic_cast<FitMsg_File_ID*> (msg);
         if (fileid != NULL) {

            if (fileid->GetTimeCreated() != FIT_FILE_ID_TIME_CREATED_INVALID) {
                TiXmlElement * timeElem = new TiXmlElement( "CreationTime" );
                timeElem->LinkEndChild(new TiXmlText(GpsFunctions::print_dtime(fileid->GetTimeCreated())));
                this->fitFileElement->LinkEndChild(timeElem);
            }

            TiXmlElement * fitId = this->fitFileElement->FirstChildElement("FitId");
                if (fitId == NULL) {
                fitId = new TiXmlElement( "FitId" );
                this->fitFileElement->LinkEndChild( fitId );
            }

            if (fileid->GetTimeCreated() != FIT_FILE_ID_TIME_CREATED_INVALID) {
                TiXmlElement * typeElem = new TiXmlElement( "Id" );
                stringstream ss;
                ss << (unsigned int)fileid->GetTimeCreated();
                typeElem->LinkEndChild(new TiXmlText(ss.str()));
                fitId->LinkEndChild(typeElem);
            }

            if (fileid->GetFileType() != FIT_FILE_ID_TYPE_INVALID) {
                TiXmlElement * typeElem = new TiXmlElement( "FileType" );
                stringstream ss;
                ss << (int)fileid->GetFileType();
                typeElem->LinkEndChild(new TiXmlText(ss.str()));
                fitId->LinkEndChild(typeElem);
            }

            if (fileid->GetManufacturer() != FIT_FILE_ID_MANUFACTURER_INVALID) {
                TiXmlElement * manElem = new TiXmlElement( "Manufacturer" );
                stringstream ss;
                ss << fileid->GetManufacturer();
                manElem->LinkEndChild(new TiXmlText(ss.str()));
                fitId->LinkEndChild(manElem);
            }

            if (fileid->GetProduct() != FIT_FILE_ID_GARMIN_PRODUCT_INVALID) {
                TiXmlElement * prodElem = new TiXmlElement( "Product" );
                stringstream ss;
                ss << fileid->GetProduct();
                prodElem->LinkEndChild(new TiXmlText(ss.str()));
                fitId->LinkEndChild(prodElem);
            }

            if (fileid->GetSerialNumber() != FIT_FILE_ID_SERIAL_NUMBER_INVALID) {
                TiXmlElement * serElem = new TiXmlElement( "SerialNumber" );
                stringstream ss;
                ss << (fileid->GetSerialNumber() & 0xFFFFFFFF);
                serElem->LinkEndChild(new TiXmlText(ss.str()));
                fitId->LinkEndChild(serElem);
            }
         } else {
             // Should not happen... internal error msgtype does not fit to class type
         }
    } else {
        // received a message we are not interested in
    }
}

string GarminFilebasedDevice::getDeviceDescription() const
{
    if (this->deviceDescription == NULL) { return ""; }

    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	this->deviceDescription->Accept( &printer );
    string str = printer.Str();

    if (Log::enabledDbg()) Log::dbg("GarminFilebasedDevice::getDeviceDescription() Done: "+this->displayName );
    return str;
}

void GarminFilebasedDevice::setDeviceDescription(TiXmlDocument * device) {
    this->deviceDescription = new TiXmlDocument(*device);
    if (this->deviceDescription != NULL) {
        setPathsFromConfiguration();
    }
}


int GarminFilebasedDevice::startWriteToGps(const string filename, const string xml)
{
    string::size_type loc = filename.find( "..", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: Filenames with .. are not allowed!");
        return 0;
    }
    loc = filename.find( "/", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: Filenames with / are not allowed!");
        return 0;
    }


    string newFilename = filename;
    // Get File extension of file to write:
    string::size_type idx;
    idx = filename.rfind('.');
    string fileToWriteExtension = "";
    if(idx != std::string::npos) {
        fileToWriteExtension = filename.substr(idx+1);
    }
    if (fileToWriteExtension.compare("") == 0) {
        // File has no file extension.
        size_t foundPos;
        foundPos=filename.find("gpxfile");
        if (foundPos!=string::npos) {
            // site http://my.garmin.com/locate/savePOI.htm uses this filename "gpxfile11152893811" for gpx files
            fileToWriteExtension = "gpx";
            newFilename.append(".gpx");
            if (Log::enabledDbg()) { Log::dbg("Using file extension gpx [file contains string gpxfile]"); }
        } else {
            // search inside xml for keywords
            foundPos=xml.find("<gpx");
            if (foundPos!=string::npos) {
                fileToWriteExtension = "gpx";
                newFilename.append(".gpx");
                if (Log::enabledDbg()) { Log::dbg("Using file extension gpx [xml contains string <gpx]"); }
            } else {
                foundPos=xml.find("<TrainingCenterDatabase");
                if (foundPos!=string::npos) {
                    fileToWriteExtension = "tcx";
                    newFilename.append(".tcx");
                    if (Log::enabledDbg()) { Log::dbg("Using file extension tcx [xml contains string <TrainingCenterDatabase]"); }
                } else {
                    Log::err("Giving up - unable to determine file type for "+filename);
                }
            }
        }
    }

    // Determine Directory to write to
    string targetDirectory = "";
    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if (currentDir.writeable) {
            // Compare file extension
            if (strncasecmp(fileToWriteExtension.c_str(), currentDir.extension.c_str(), currentDir.extension.length()) == 0) {
                targetDirectory = this->baseDirectory + "/" + currentDir.path;
                break;
            } else if (Log::enabledDbg()) {
                Log::dbg("Wrong file extension for target directory: "+currentDir.name);
            }
        }
    }
    if (targetDirectory.length() == 0) {
        Log::err("Unable to find a valid target directory to write file "+filename);
        this->transferSuccessful = false;
        return 0;
    }

    // There shouldn't be a thread running... but who knows...
    lockVariables();
    this->xmlToWrite = xml;
    this->filenameToWrite = targetDirectory + "/" + newFilename;
    this->overwriteFile = 2; // not yet asked
    this->workType = WRITEGPX;
    unlockVariables();

    if (Log::enabledDbg()) Log::dbg("Saving to file: "+this->filenameToWrite);

    if (startThread()) {
        return 1;
    }

    return 0;
}



void GarminFilebasedDevice::writeGpxFile() {

    lockVariables();
    string xml = this->xmlToWrite;
    string filename = this->filenameToWrite;
    string systemCmd = this->storageCmd;
    this->threadState = 1; // Working
    unlockVariables();

    struct stat stFileInfo;
    int intStat;
    // Attempt to get the file attributes
    intStat = stat(filename.c_str(),&stFileInfo);
    if(intStat == 0) {
        // File exists - we need to ask the user to overwrite
        lockVariables();
        this->waitingMessage = new MessageBox(Question, "File "+filename+" exists. Overwrite?", BUTTON_YES | BUTTON_NO , BUTTON_NO, this);
        this->threadState = 2;
        unlockVariables();

        waitThread(); // Sleep until thread gets signal (user answered)

        bool doOverwrite = true;

        lockVariables();
            if (this->overwriteFile != 1) {
                this->threadState = 3;
                this->transferSuccessful = false;
                doOverwrite = false;
            }
        unlockVariables();

        if (!doOverwrite) {
            Log::dbg("Thread aborted");
            return;
        }
    }

    ofstream file;
    file.open (filename.c_str());
    file << xml;
    file.close();

    // Execute extern command if wanted
    if (systemCmd.length() > 0) {
        string placeholder = "%1";
        int pos=systemCmd.find( placeholder );
        if (pos >=0) {
            systemCmd.replace(systemCmd.find(placeholder),placeholder.length(),filename);
        }

        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        Log::dbg("Thread before executing user command: "+systemCmd);
        int ret = system(systemCmd.c_str());

        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);

        if (ret != 0) {
            lockVariables();
                this->waitingMessage = new MessageBox(Question, "Error executing command: "+systemCmd, BUTTON_OK , BUTTON_OK, NULL);
                this->threadState = 2;
            unlockVariables();

            sleep(1); // give application time to fetch messagebox
            lockVariables();
            this->threadState = 3;
            unlockVariables();

            Log::err("Executing user command failed: "+systemCmd);
            return;
        }
    }

    lockVariables();
    this->threadState = 3; // Finished
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

}

void GarminFilebasedDevice::doWork() {

    if ((this->workType == WRITEGPX) ||
        (this->workType == WRITEFITNESSDATA)) {
        this->writeGpxFile();
    } else if (this->workType == READFITNESS) {
        this->readFitnessDataFromDevice(true, "");
    } else if (this->workType == READFITNESSDIR) {
        this->readFitnessDataFromDevice(false, "");
    } else if (this->workType == READFITNESSDETAIL) {
        this->readFitnessDataFromDevice(true, this->readFitnessDetailId);
    } else if (this->workType == READFITDIRECTORY) {
        this->readFITDirectoryFromDevice();
    } else if (this->workType == READABLEFILELISTING) {
        this->readFileListingFromDevice();
    } else if (this->workType == READFITNESSUSERPROFILE) {
        this->readFitnessUserProfile();
    } else if (this->workType == READFITNESSCOURSES) {
        this->readFitnessCourses(true);
    } else if (this->workType == READFITNESSCOURSESDIR) {
        this->readFitnessCourses(false);
    } else if (this->workType == READFITNESSWORKOUTS) {
        this->readFitnessWorkouts();
    } else if (this->workType == DIRECTORYLISTING) {
        this->readDirectoryListing();
    } else {
        Log::err("Work Type not implemented!");
    }
}

#define MD5READBUFFERSIZE 1024*16

void GarminFilebasedDevice::readFileListingFromDevice() {
    if (Log::enabledDbg()) { Log::dbg("Thread readFileListing started"); }

    string workDir="";
    string extensionToRead="";
    string pathOnDevice = "";
    string basename = "";

    lockVariables();
    this->threadState = 1; // Working
    bool doCalculateMd5 = this->readableFileListingComputeMD5;

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.extension.compare(this->readableFileListingFileTypeName) == 0) &&  (currentDir.name.compare(this->readableFileListingDataTypeName) == 0) && (currentDir.readable)) {
            workDir = this->baseDirectory + "/" + currentDir.path;
            extensionToRead = currentDir.extension;
            pathOnDevice = currentDir.path;
            basename = currentDir.basename;
        }
    }
    unlockVariables();


    TiXmlDocument * output = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    output->LinkEndChild( decl );

    TiXmlElement * dirList = new TiXmlElement( "DirectoryListing" );
    dirList->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/DirectoryListing/v1");
    dirList->SetAttribute("RequestedPath","");
    dirList->SetAttribute("UnitId",this->deviceId);
    dirList->SetAttribute("VolumePrefix","");
    output->LinkEndChild( dirList );

    if (workDir.length() > 0) {
        DIR *dp;
        struct dirent *dirp;

        if (Log::enabledDbg()) { Log::dbg("Found directory to read: "+workDir); }

        if((dp = opendir(workDir.c_str())) != NULL) {
            while ((dirp = readdir(dp)) != NULL) {
                string fileName = string(dirp->d_name);
                string fullFileName = workDir+'/'+fileName;
                bool isDirectory = (dirp->d_type == 4) ? true : false;

                if (Log::enabledDbg()) { Log::dbg("Found file: "+fileName); }
                if ((fileName == ".") || (fileName == "..")) { continue; }

                // Check file extension
                string lastFilePart = fileName.substr(fileName.length() - extensionToRead.length());
                if (strncasecmp(lastFilePart.c_str(), extensionToRead.c_str(), extensionToRead.length()) != 0) {
                    if (Log::enabledDbg()) { Log::dbg("Found file with incorrect extension: "+fileName);}
                    continue;
                }

                // Check basename if set
                if (basename.length()>0) {
                    string firstFilePart = fileName.substr(0, basename.length());
                    if (strncasecmp(firstFilePart.c_str(), basename.c_str(), basename.length()) != 0) {
                        if (Log::enabledDbg()) { Log::dbg("Found file with incorrect basename: "+fileName);}
                        continue;
                    }
                }

                TiXmlElement * curFile = new TiXmlElement( "File" );
                if (isDirectory) {
                    curFile->SetAttribute("IsDirectory","true");
                } else {
                    curFile->SetAttribute("IsDirectory","false");
                }
                curFile->SetAttribute("Path",pathOnDevice+'/'+fileName);

                // Get File Size
                struct stat filestatus;
                stat( fullFileName.c_str(), &filestatus );
                stringstream ss;
                ss << filestatus.st_size;
                curFile->SetAttribute("Size",ss.str());

                // Get the timestamp
                TiXmlElement * timeElem = new TiXmlElement( "CreationTime" );
                timeElem->LinkEndChild(new TiXmlText(GpsFunctions::print_dtime(filestatus.st_mtime-TIME_OFFSET)));
                curFile->LinkEndChild(timeElem);


                if ((!isDirectory) && (doCalculateMd5)) {
                    if (Log::enabledDbg()) { Log::dbg("Calculating MD5 sum of " + fullFileName);}
                    MD5_CTX c;
                    unsigned char md[MD5_DIGEST_LENGTH];
                    unsigned char buf[MD5READBUFFERSIZE];
                    FILE *f = fopen(fullFileName.c_str(),"r");
                    int fd=fileno(f);
                    MD5_Init(&c);
                    for (;;) {
                        int i=read(fd,buf,MD5READBUFFERSIZE);
                        if (i <= 0) break;
                        MD5_Update(&c,buf,(unsigned long)i);
                    }
                    MD5_Final(&(md[0]),&c);
                    string md5="";
                    for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
                        char temp[16];
                        sprintf(temp, "%02x",md[i]);
                        md5 += temp;

                    }
                    curFile->SetAttribute("MD5Sum",md5);
                }

                dirList->LinkEndChild( curFile );
            }
            closedir(dp);
        } else {
            Log::err("Error opening directory! "+ workDir);
        }
    } else {
        if (Log::enabledDbg()) { Log::dbg("No directory found to read"); }
    }


    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    output->Accept( &printer );
    string outputXml = printer.Str();
    delete(output);

    lockVariables();
    this->threadState = 3; // Finished
    this->readableFileListingXml = outputXml;
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readFileListing finished"); }
    return;
}

/**
 * Sort two activities
 */
bool activitySorter (TiXmlNode * a,TiXmlNode * b) {
    string aId="";
    string bId="";

    TiXmlElement * idNode = a->FirstChildElement("Id");
    if (idNode != NULL) { aId = idNode->GetText(); }
    idNode = b->FirstChildElement("Id");
    if (idNode != NULL) { bId = idNode->GetText(); }

    return (aId.compare(bId) > 0);
}

/**
 * Sort two fit files
 */
bool fitFileSorter (TiXmlNode * a,TiXmlNode * b) {
    string aId="";
    string bId="";

    TiXmlElement * cTimeNode = a->FirstChildElement("CreationTime");
    if (cTimeNode != NULL) { aId = cTimeNode->GetText(); }
    cTimeNode = b->FirstChildElement("CreationTime");
    if (cTimeNode != NULL) { bId = cTimeNode->GetText(); }

    return (aId.compare(bId) > 0);
}

/**
 * Parses all attributes of a TiXmlElement and adds them to another TiXmlElement if it is missing
 */
void GarminFilebasedDevice::addMissingAttributes(TiXmlElement * in, TiXmlElement * out) {
    if ((in==NULL) || (out==NULL)) { return; }
    TiXmlAttribute* pAttrib=in->FirstAttribute();
    while (pAttrib) {
        const char * a  = out->Attribute(pAttrib->Name());
        if (a==NULL) {
            // Attribute does not exist in out, need to create it
            out->SetAttribute(pAttrib->Name(),pAttrib->Value());
        }
        pAttrib=pAttrib->Next();
    }
}


/**
 * Reads TCX directories
 */
void GarminFilebasedDevice::readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId) {
    Log::dbg("Thread readFitnessData started");
/*
Thread-Status
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/
    string workDir="";
    string extension="";
    lockVariables();
    this->threadState = 1; // Working

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.dirType == TCXDIR) &&  (currentDir.name.compare("FitnessHistory") == 0)) {
            workDir = this->baseDirectory + "/" + currentDir.path;
            extension = currentDir.extension;
        }
    }
    unlockVariables();

    // Check if the device supports reading tcx files
    if (workDir.length() == 0) {
        Log::err("Device does not support reading TCX Files. Element FitnessHistory not found in xml!");
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }

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

    // We need to sort the activities, that's why they are first stored in a list
    vector <TiXmlNode *> activitiesList;

    // Loop over all files in Fitnessdirectory:
    for (unsigned int i = 0;i < files.size();i++) {
        if (files[i].find("."+extension)!=string::npos) {
            TiXmlDocument doc(workDir + "/" + files[i]);
            if (Log::enabledDbg()) { Log::dbg("Opening file: "+ files[i]); }
            if (doc.LoadFile()) {
                TiXmlElement * trainFile = doc.FirstChildElement("TrainingCenterDatabase");
                if (trainFile != NULL) {
                    addMissingAttributes(trainFile, train);
                    TiXmlElement * inputActivities = trainFile->FirstChildElement("Activities");
                    while ( inputActivities != NULL) {
                        TiXmlElement * inputActivity =inputActivities->FirstChildElement("Activity");
                        while ( inputActivity != NULL) {

                            string currentLapId="";
                            TiXmlElement * idNode = inputActivity->FirstChildElement("Id");
                            if (idNode != NULL) { currentLapId = idNode->GetText(); }

                            if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(currentLapId) == 0)) {
                                TiXmlNode * newAct = inputActivity->Clone();

                                if (!readTrackData) {
                                    // Track data must be deleted
                                    TiXmlNode * node = newAct->FirstChildElement("Lap");
                                    while (node != NULL) {
                                        TiXmlNode * trackNode = node->FirstChildElement("Track");
                                        while (trackNode != NULL) {
                                            node->RemoveChild( node->FirstChildElement("Track") );
                                            trackNode = node->FirstChildElement("Track");
                                        }

                                        //node = newAct->FirstChildElement("Lap");
                                        node = node->NextSibling();
                                    }
                                }

                                //activities->LinkEndChild( newAct );
                                activitiesList.push_back(newAct);

                                if (Log::enabledDbg()) { Log::dbg("Adding activity "+currentLapId+" from file "+files[i]); }
                            }
                            inputActivity = inputActivity->NextSiblingElement( "Activity" );
                        }
                        inputActivities = inputActivities->NextSiblingElement( "Activities" );
                    }
                }
            } else {
                Log::err("Unable to load fitness file "+files[i]);
            }
        }
    }

    // Sort list, newest activity must be on top
    sort(activitiesList.begin(), activitiesList.end(), activitySorter);
    vector<TiXmlNode *>::iterator it;
    for ( it=activitiesList.begin() ; it < activitiesList.end(); ++it )
    {
        TiXmlNode * curAct = *it;
        activities->LinkEndChild( curAct );
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

void GarminFilebasedDevice::readFITDirectoryFromDevice() {
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

    // We need to sort the fit files, that's why they are first stored in a list
    vector <TiXmlNode *> fitFileList;

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentFitDir = (*it);
        if ((currentFitDir.dirType != FITDIR)) {
            continue;
        }

        DIR *dp;
        struct dirent *dirp;
        string fullPath = this->baseDirectory + "/" + currentFitDir.path;

        if((dp = opendir(fullPath.c_str())) != NULL) {

            if (Log::enabledDbg()) { Log::dbg("Searching for files in "+fullPath);}
            while ((dirp = readdir(dp)) != NULL) {
                string fileName = string(dirp->d_name);
                bool isDirectory = (dirp->d_type == 4) ? true : false;

                if (isDirectory) { continue; } // Ignore directories

                if (fileName.length() > currentFitDir.extension.length()) {
                    string lastFilePart = fileName.substr(fileName.length() - currentFitDir.extension.length());
                    if (strncasecmp(lastFilePart.c_str(), currentFitDir.extension.c_str(), currentFitDir.extension.length()) == 0) {
                        if (Log::enabledDbg()) { Log::dbg("Found file with correct extension: "+fileName);}
                        this->fitFileElement = new TiXmlElement( "File" );
                        this->fitFileElement->SetAttribute("IsDirectory","false");
                        this->fitFileElement->SetAttribute("Path",currentFitDir.path+'/'+fileName);

                        // Opening and parsing of fit file:
                        string fullFileName = this->baseDirectory + "/" + currentFitDir.path+'/'+fileName;

                        FitReader fit(fullFileName);
                        fit.registerFitMsgFkt(this);
                        try {
                            if (Log::enabledInfo()) { Log::info("Reading fit file: "+fullFileName); }
                            if (fit.isFitFile()) {
                                while (fit.readNextRecord()) {
                                    // processing of records is done in fitMsgReceived()
                                }
                                fit.closeFitFile();
                                //dirList->LinkEndChild( this->fitFileElement );
                                fitFileList.push_back(this->fitFileElement);
                            } else {
                                Log::err("Invalid fit file: "+fullFileName);
                                delete(this->fitFileElement);
                            }
                        } catch (FitFileException &e) {
                            Log::err("Decoding error: "+e.getError());
                            delete(this->fitFileElement);
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

    // Sort list, newest fit file must be on top
    sort(fitFileList.begin(), fitFileList.end(), fitFileSorter);
    vector<TiXmlNode *>::iterator it;
    for ( it=fitFileList.begin() ; it < fitFileList.end(); ++it )
    {
        TiXmlNode * curFitFile = *it;
        dirList->LinkEndChild( curFitFile );
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

MessageBox * GarminFilebasedDevice::getMessage() {
    MessageBox * msg = this->waitingMessage;
    this->waitingMessage = NULL;
    return msg;
}

void GarminFilebasedDevice::userAnswered(const int answer) {
    if (answer == 1) {
        if (Log::enabledDbg()) Log::dbg("User wants file overwritten");
        lockVariables();
        this->overwriteFile = 1;
        unlockVariables();
    } else {
        if (Log::enabledDbg()) Log::dbg("User wants file to be untouched");
        lockVariables();
        this->overwriteFile = 0;
        unlockVariables();
    }
    lockVariables();
    this->threadState = 1; /* set back to working */
    unlockVariables();

    signalThread();
}

int GarminFilebasedDevice::finishWriteToGps()
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


void GarminFilebasedDevice::cancelWriteToGps() {
    cancelThread();
}

int GarminFilebasedDevice::getTransferSucceeded() {
    if (this->transferSuccessful) {
        return 1;
    }
    return 0;
}

void GarminFilebasedDevice::setBaseDirectory(string directory) {
    this->baseDirectory = directory;

    if (this->deviceDescription != NULL) {
        setPathsFromConfiguration();
    }
}

void GarminFilebasedDevice::setStorageCommand(string cmd) {
    this->storageCmd = cmd;
}

bool GarminFilebasedDevice::isDeviceAvailable() {
    struct stat st;
    if(stat(this->baseDirectory.c_str(),&st) == 0) {
        // directory exists
        return true;
    }
    Log::dbg("Device is not available: "+this->displayName);
    return false;
}

void GarminFilebasedDevice::setUpdatePathsFromConfiguration() {
    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("Id"); }
        if (node!=NULL) { deviceId = node->GetText(); }

        node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("MassStorageMode"); }
        if (node!=NULL) { node = node->FirstChildElement("UpdateFile"); }
        while ( node != NULL) {
            TiXmlElement *ti_path = node->FirstChildElement("Path");
            TiXmlElement *ti_filename = node->FirstChildElement("FileName");
            TiXmlElement *ti_partnum = node->FirstChildElement("PartNumber");
            MassStorageDirectoryType devDir;

            if (ti_path != NULL) {
                devDir.path = ti_path->GetText();
            }
            if (ti_filename != NULL) {
                devDir.basename = ti_filename->GetText();
            }
            if (ti_partnum != NULL) {
                devDir.name = ti_partnum->GetText();
            }
            devDir.writeable = true;
            devDir.readable = false;
            devDir.dirType = UNKNOWN;

            // Debug print
            if (Log::enabledDbg()) {
                stringstream ss;
                ss << "UpdateFile: ";
                ss << "Path: " << devDir.path;
                ss << " File: " << devDir.basename;
                ss << " Name: " << devDir.name;
                Log::dbg("Found Feature: "+ss.str());
            }

            deviceDirectories.push_back(devDir);

            node = node->NextSiblingElement("UpdateFile");
        }
    }
}

/**
 * On some devices the file GarminDevice.xml contains a wrong upper/lower writing of the directory names
 * This is corrected in this function, by searching for the proper directory name if it can not be found
 */
void GarminFilebasedDevice::checkPathsFromConfiguration() {
    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        string fullDirectoryName = this->baseDirectory + "/" + currentDir.path;
        struct stat st;

        if(stat(fullDirectoryName.c_str(),&st) != 0) {
            // directory does not exist, check upper/lower case of subdirectories
            if (Log::enabledInfo()) { Log::info("Directory "+currentDir.path+" does not exist on device, searching alternative upper/lowercase writings");}
            bool foundNewPath = true;
            stringstream ss(currentDir.path);
            string existingSubPath = "";
            string item;
            while(std::getline(ss, item, '/')) {
                string parentPath = this->baseDirectory;
                if (existingSubPath.length() > 0) {
                    parentPath += "/" + existingSubPath;
                }
                string pathToTest = parentPath + "/" + item;

                // Test name from configuration
                if(stat(pathToTest.c_str(),&st) != 0) {
                    // directory does not exist...

                    DIR *dp;
                    struct dirent *dirp;
                    if((dp = opendir(parentPath.c_str())) != NULL) {
                        bool foundEntry = false;
                        while ((dirp = readdir(dp)) != NULL) {
                            string fileName = string(dirp->d_name);

                            if ((fileName.length() == item.length()) &&
                               (strncasecmp(fileName.c_str(), item.c_str(), item.length()) == 0)) {
                                item = fileName;
                                foundEntry = true;
                                break;
                            }
                        }
                        closedir(dp);
                        if (!foundEntry) { foundNewPath = false; }
                    } else {
                        if (Log::enabledDbg()) { Log::dbg("Unable to open directory "+parentPath+" while searching for "+currentDir.path); }
                    }

                }
                if (existingSubPath.length() > 0) { existingSubPath += "/"; }
                existingSubPath += item;
            }

            if (foundNewPath) {
                if (currentDir.path.length() > 0) {
                    std::string::iterator it = currentDir.path.end() - 1;
                    if (*it == '/') {
                        existingSubPath += "/";
                    }
                }
                Log::info("Overwriting "+ currentDir.path + " from configuration with " + existingSubPath);
            } else {
                if (Log::enabledDbg()) { Log::dbg("No alternative found for "+currentDir.path);}
            }
        }
    }
}

void GarminFilebasedDevice::setPathsFromConfiguration() {
    if (!deviceDirectories.empty()) { deviceDirectories.clear(); }
    this->fitnessFile = this->baseDirectory+"/Garmin/gpx/current/Current.gpx"; // Fallback

    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("Id"); }
        if (node!=NULL) { deviceId = node->GetText(); }

        node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("MassStorageMode"); }
        if (node!=NULL) { node = node->FirstChildElement("DataType"); }
        while ( node != NULL) {
            TiXmlElement * node2 = node->FirstChildElement("Name");
            if (node2 != NULL) {
                string nameText = node2->GetText();
                node2 = node->FirstChildElement("File");
                while (node2 != NULL) {
                    TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                    string transDir = transferDirection->GetText();

                    MassStorageDirectoryType devDir;
                    devDir.dirType = UNKNOWN;
                    devDir.name = nameText;

                    if (transDir.compare("InputToUnit") == 0) {
                        devDir.writeable = true;
                        devDir.readable  = false;
                    } else if (transDir.compare("InputOutput") == 0) {
                        devDir.writeable = true;
                        devDir.readable  = true;
                    } else if (transDir.compare("OutputFromUnit") == 0) {
                        devDir.writeable = false;
                        devDir.readable  = true;
                    }

                    TiXmlElement * ti_loc = NULL;
                    TiXmlElement * ti_path = NULL;
                    TiXmlElement * ti_basename = NULL;
                    TiXmlElement * ti_ext = NULL;
                    if (node2!=NULL)  { ti_loc      = node2->FirstChildElement("Location"); }
                    if (ti_loc!=NULL) { ti_path     = ti_loc->FirstChildElement("Path"); }
                    if (ti_loc!=NULL) { ti_basename = ti_loc->FirstChildElement("BaseName"); }
                    if (ti_loc!=NULL) { ti_ext      = ti_loc->FirstChildElement("FileExtension"); }

                    if (ti_path != NULL) {
                        devDir.path = ti_path->GetText();
                    }
                    if (ti_basename != NULL) {
                        devDir.basename = ti_basename->GetText();
                    }

                    // Determine Type of directory
                    string::size_type position = nameText.find( "FIT_TYPE_", 0 );
                    if ((position != string::npos) || (nameText.compare("FITBinary")==0)) {
                        devDir.dirType = FITDIR;
                    } else if ((nameText.compare("FitnessWorkouts")==0) ||
                        (nameText.compare("FitnessHistory")==0) ||
                        (nameText.compare("FitnessCourses")==0) ||
                        (nameText.compare("FitnessUserProfile")==0)) {
                        devDir.dirType = TCXDIR;
                    } else if (nameText.compare("GPSData")==0) {
                        devDir.dirType = GPXDIR;
                    }

                    if (ti_ext != NULL) {
                        string ext  = ti_ext->GetText();
                        devDir.extension = ext;
                    }

                    // Debug print
                    if (Log::enabledDbg()) {
                        stringstream ss;
                        if (devDir.dirType == FITDIR) {
                            ss << "FIT: ";
                        } else if (devDir.dirType == TCXDIR) {
                            ss << "TCX: ";
                        } else if (devDir.dirType == GPXDIR) {
                            ss << "GPX: ";
                        } else {
                            ss << "???: ";
                        }
                        ss << "Path: " << devDir.path;
                        ss << " Ext: " << devDir.extension;
                        ss << " Name: " << devDir.name;
                        Log::dbg("Found Feature: "+ss.str());
                    }

                    deviceDirectories.push_back(devDir);

                    node2 = node2->NextSiblingElement("File");
                }
            }
            node = node->NextSiblingElement( "DataType" );
        }
    }
    setUpdatePathsFromConfiguration();

    checkPathsFromConfiguration();
}

int GarminFilebasedDevice::startReadFITDirectory() {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITDIRECTORY;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int GarminFilebasedDevice::finishReadFITDirectory() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void GarminFilebasedDevice::cancelReadFITDirectory() {
    if (Log::enabledDbg()) { Log::dbg("cancelReadFITDirectory called for "+this->displayName); }
    cancelThread();
}

string GarminFilebasedDevice::getFITData() {
    return  fitDirectoryXml;
}

int GarminFilebasedDevice::startReadFitnessDirectory(string dataTypeName) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    if (dataTypeName.compare("FitnessCourses") == 0) {
        this->workType = READFITNESSCOURSESDIR;  // FitnessCourses
    } else if (dataTypeName.compare("FitnessHistory") == 0) {
        this->workType = READFITNESSDIR;  // FitnessHistory
    } else {
        Log::err("Unknown data to read: '"+dataTypeName+"' - Defaulting back to FitnessHistory");
        this->workType = READFITNESSDIR;  // FitnessHistory
    }

    if (startThread()) {
        return 1;
    }

    return 0;
}

int GarminFilebasedDevice::finishReadFitnessDirectory() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void GarminFilebasedDevice::cancelReadFitnessData() {
    cancelThread();
}

int GarminFilebasedDevice::startReadFitnessDetail(string id) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read fitness detail from garmin device: "+this->displayName+ " Searching for "+id);

    this->workType = READFITNESSDETAIL;
    this->readFitnessDetailId = id;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int GarminFilebasedDevice::finishReadFitnessDetail() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void GarminFilebasedDevice::cancelReadFitnessDetail() {
    cancelThread();
}

int GarminFilebasedDevice::startReadFromGps() {
    struct stat stFileInfo;
    int intStat;

    // Search for GPSData in Configuration
    this->fitnessFile = "";
    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.dirType == GPXDIR) &&  (currentDir.name.compare("GPSData") == 0) && (currentDir.readable)) {
            this->fitnessFile = this->baseDirectory + "/" + currentDir.path;
            if (currentDir.basename.length() > 0) {
                this->fitnessFile += '/' + currentDir.basename + '.' + currentDir.extension;
            }
        }
    }

    if (this->fitnessFile.length() == 0) {
        Log::err("Unable to determine fitness file, does the device support GPSData?");
        return 0;
    }

    // Attempt to get the file attributes
    intStat = stat(this->fitnessFile.c_str(),&stFileInfo);
    if(intStat != 0) {
        Log::err("The file "+this->fitnessFile+" could not be found. Unable to read Gpx data.");
        this->transferSuccessful = 0;
        return 0;
    }

    this->transferSuccessful = 1;
    if (Log::enabledDbg()) Log::dbg("No thread necessary to read from device, gpx file exists");

    return 1;
}

int GarminFilebasedDevice::finishReadFromGps() {
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    return 3; // No processing of data necessary, therefore finish instantly
}


string GarminFilebasedDevice::getGpxData() {
    stringstream filecontent;
    std::ifstream file;
    file.open (this->fitnessFile.c_str());
    if (file) {
        string line;
        while (getline(file, line)) {
            filecontent << line << "\n";
        }
        file.close();
    } else {
        Log::err("GetGpxData(): Unable to open file "+this->fitnessFile);
    }

    return filecontent.str();
}

void GarminFilebasedDevice::cancelReadFromGps() {
    this->transferSuccessful = 0;
    Log::dbg("Canceling ReadFromGps...");
    // Nothing much to do here as no thread was started
}

string GarminFilebasedDevice::getBinaryFile(string relativeFilePath) {
    //TODO: Check for .. in file path. No website should be able to access files outside the garmin device!
    // Check if site is allowed to read from that directory

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
    in.close();
    return buffer.str();
}

int GarminFilebasedDevice::startDownloadData(string gpsDataString) {
    Log::err("startDownloadData was called for "+this->displayName);

    if (!deviceDownloadList.empty()) {
        Log::info("There are still files to download in the queue. Erasing these files...");
    }
    deviceDownloadList.clear();

    TiXmlDocument doc;
    doc.Parse(gpsDataString.c_str());

    // It is possible that more than one file can be downloaded
    TiXmlElement * devDown = doc.FirstChildElement("DeviceDownload");
    if (devDown != NULL) {
        TiXmlElement * file = devDown->FirstChildElement("File");
        while (file != NULL) {
            const char * url = file->Attribute("Source");
            const char * dest = file->Attribute("Destination");
            const char * region = file->Attribute("RegionId");
            if ((url != NULL) && (dest != NULL)) {
                string strRegion = "";
                if (region != NULL) {
                    strRegion = region;
                }
                string strUrl = url;
                string strDest = dest;

                if (Log::enabledDbg()) { Log::dbg("Download destination: "+strDest + "  URL: "+strUrl); }

                if ((strUrl.length() > 0) && (strDest.length() > 0)) {
                    // Replace \ with /
                    string::size_type pos = strDest.find("\\", 0);
                    while(string::npos != pos ) {
                            strDest.replace(pos, 1, "/");
                            pos = strDest.find("\\", 0);
                    }

                    //If anyone knows a better way to detect directory traversal, please notify me
                    pos = strDest.find("../", 0);
                    if (string::npos == pos ) {

                        //Test if target directory is a valid write directory
                        bool directoryIsValid = false;
                        string fileNameOnly = basename(strDest.c_str());
                        string directoryOnly = "";
                        if (fileNameOnly.length() < strDest.length()) {
                            directoryOnly = strDest.substr(0,strDest.length()-fileNameOnly.length()-1);
                        }
                        Log::dbg("Comparing with "+directoryOnly);
                        for (list<MassStorageDirectoryType>::iterator it=deviceDirectories.begin(); it!=deviceDirectories.end(); ++it) {
                            MassStorageDirectoryType dt = (*it);
                            if ((directoryOnly.compare( dt.path ) == 0) && (dt.writeable)) { directoryIsValid = true; }
                        }

                        if (directoryIsValid) {
                            DeviceDownloadData fileElement;
                            fileElement.url = strUrl;
                            fileElement.destination = strDest;
                            fileElement.regionId = strRegion;
                            deviceDownloadList.push_back(fileElement);
                        } else {
                            Log::err("Device does not allow to write to this path: "+strDest);
                        }
                    } else {
                        Log::err("Invalid filename! Found '..' Directory traversal not allowed!");
                    }
                }
            } else {
                if (Log::enabledDbg()) { Log::dbg("Received an element with no Source/Destination Attribute"); }
            }
            file = file->NextSiblingElement("File");
        }
    } else {
        if (Log::enabledDbg()) { Log::dbg("Unable to find xml element DeviceDownload in data"); }
    }

    if (Log::enabledDbg()) {
        stringstream ss;
        ss << "Received a list of " << deviceDownloadList.size() << " files to download!";
        Log::dbg(ss.str());
    }

    if (!deviceDownloadList.empty()) {
        downloadDataErrorCount = 0;
    }

    return deviceDownloadList.size();
}

string GarminFilebasedDevice::getNextDownloadDataUrl() {
    if (!deviceDownloadList.empty()) {
        DeviceDownloadData downloadData = deviceDownloadList.front();
        return downloadData.url;
    }
    return "";
}

int GarminFilebasedDevice::writeDownloadData(char * buf, int length) {
    if (!deviceDownloadList.empty()) {
        DeviceDownloadData downloadData = deviceDownloadList.front();
        string filename = baseDirectory + "/" + downloadData.destination;

        if (Log::enabledDbg()) {
            stringstream ss;
            ss << "Writing " << length << " bytes to file " << filename;
            Log::dbg(ss.str());
        }
        if (!downloadDataOutputStream.is_open()) {
            downloadDataOutputStream.open(filename.c_str(), ios::out | ios::binary);
        }
        if (downloadDataOutputStream.is_open()) {
            downloadDataOutputStream.write (buf, length);
        } else {
            downloadDataErrorCount++;
            Log::err("Unable to open file "+filename);
            return -1;
        }
    }
    return length;
}

void GarminFilebasedDevice::saveDownloadData() {
    Log::dbg("saveDownloadData was called for "+this->displayName);
    if (downloadDataOutputStream.is_open()) {
        downloadDataOutputStream.close();
        if (!deviceDownloadList.empty()) {
            deviceDownloadList.pop_front();
            Log::dbg("Removing file to download from list");
        }
    } else {
        Log::dbg("Not closing anything, since nothing was open...");
    }
}

void GarminFilebasedDevice::cancelDownloadData() {
    Log::dbg("cancelDownloadData was called for "+this->displayName);
    if (downloadDataOutputStream.is_open()) {
        downloadDataOutputStream.close();
    }
    if (!deviceDownloadList.empty()) {
        deviceDownloadList.pop_front();
    }
    downloadDataErrorCount++;
    this->transferSuccessful = false;
}


/**
 * This is used to indicate the status of the write download data process.
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GarminFilebasedDevice::finishDownloadData() {
    if (downloadDataErrorCount > 0) {
        this->transferSuccessful = false;
        return 3; // Finished
    }
    if (!deviceDownloadList.empty()) {
        return 1;
    } else {
        this->transferSuccessful = true;
        return 3;
    }
}

/**
* Starts a thread that writes the passed xml string to the given filename
* @param filename - filename on disk
* @param data - the filename to write to on the device.
* @param dataTypeName - a Fitness DataType from the GarminDevice.xml retrieved with DeviceDescription
* @return int returns 1 if successful otherwise 0
*/
int GarminFilebasedDevice::startWriteFitnessData(string filename, string data, string dataTypeName) {
    string::size_type loc = filename.find( "../", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: Filenames with ../ are not allowed! "+filename);
        return 0;
    }

    string pathToWrite = "";
    for (list<MassStorageDirectoryType>::iterator it=deviceDirectories.begin(); it!=deviceDirectories.end(); ++it) {
        MassStorageDirectoryType dt = (*it);
        if ((dataTypeName.compare(dt.name) == 0) && (dt.writeable)) {
            pathToWrite = dt.path;
        }
    }

    if (pathToWrite.length() == 0) {
        Log::err("Path for " + dataTypeName + " not found. Not writing to device!");
        return 0;
    }

    // There shouldn't be a thread running... but who knows...
    lockVariables();
    this->xmlToWrite = data;
    this->filenameToWrite = this->baseDirectory + "/" + pathToWrite + "/" + filename;
    this->overwriteFile = 2; // not yet asked
    this->workType = WRITEFITNESSDATA;
    unlockVariables();

    if (Log::enabledDbg()) Log::dbg("Saving to file: "+this->filenameToWrite);

    if (startThread()) {
        return 1;
    }

    return 0;


    return 0;
}

/**
 * This is used to indicate the status of the write fitness data process.
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GarminFilebasedDevice::finishWriteFitnessData() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

/**
 * Cancels the current write of fitness data
 */
void GarminFilebasedDevice::cancelWriteFitnessData() {
    cancelThread();
}

/**
 * Returns the bytes available in the given path on the device
 * @return bytes available (-1 for non-mass storage mode devices.)
 */
int GarminFilebasedDevice::bytesAvailable(string path) {
    if (Log::enabledDbg()) { Log::dbg("bytesAvailable called for path "+path); }
    string fullPath = baseDirectory + "/" + path;

    struct statfs st;
    if (statfs(fullPath.c_str(), &st) == 0) {
        long freeBytes = st.f_bfree * st.f_bsize;
        if (Log::enabledDbg()) {
            stringstream ss;
            ss << "Bytes available for path " << fullPath << ": " << freeBytes;
            Log::dbg(ss.str());
        }
        if (freeBytes > INT_MAX) {
            return INT_MAX;
        } else {
            return (int)freeBytes;
        }
    } else {
        Log::err("Error getting bytes available for path: "+fullPath);
        return -1;
    }
}

int GarminFilebasedDevice::startReadFitnessData(string dataTypeName) {
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device ("+dataTypeName+")");

    if (dataTypeName.compare("FitnessUserProfile")==0) {
        this->workType = READFITNESSUSERPROFILE;
    } else if (dataTypeName.compare("FitnessWorkouts")==0) {
        this->workType = READFITNESSWORKOUTS;
    } else if (dataTypeName.compare("FitnessCourses")==0) {
        this->workType = READFITNESSCOURSES;
    } else if (dataTypeName.compare("FitnessHistory")==0) {
        this->workType = READFITNESS;
    } else {
        Log::err("Unknown data to read: '"+dataTypeName+"' - Defaulting back to FitnessHistory");
        this->workType = READFITNESS;
    }

    if (startThread()) {
        return 1;
    }

    return 0;
}

int GarminFilebasedDevice::finishReadFitnessData() {
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

string GarminFilebasedDevice::getFitnessData() {
    return this->fitnessDataTcdXml;
}



/**
* Starts an asynchronous file listing operation for a Mass Storage mode device.
* Only files that are output from the device are listed. </br>
* The result can be retrieved with getDirectoryXml().
* Minimum plugin version 2.8.1.0 <br/>
*
* @param {String} dataTypeName a DataType from GarminDevice.xml retrieved with DeviceDescription
* @param {String} fileTypeName a Specification Identifier for a File in dataTypeName from GarminDevice.xml
* @param {Boolean} computeMD5 If true, the plug-in will generate an MD5 checksum for each readable file.
*/
int GarminFilebasedDevice::startReadableFileListing(string dataTypeName, string fileTypeName, bool computeMd5) {

    lockVariables();
    this->threadState = 1;
    this->readableFileListingDataTypeName = dataTypeName;
    this->readableFileListingFileTypeName = fileTypeName;
    this->readableFileListingComputeMD5   = computeMd5;
    this->readableFileListingXml = "";
    unlockVariables();

    if (Log::enabledDbg()) Log::dbg("Starting thread to read file listing from garmin device "+this->displayName);

    this->workType = READABLEFILELISTING;

    if (startThread()) {
        return 1;
    }

    return 0;
}

/**
 * Returns the status of the asynchronous file listing operation for the mass storage mode device
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GarminFilebasedDevice::finishReadableFileListing() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

/**
 * Cancels the asynchronous file listing operation for the mass storage mode device
 */
void GarminFilebasedDevice::cancelReadableFileListing() {
    if (Log::enabledDbg()) { Log::dbg("cancelReadableFileListing for device "+this->displayName); }
    cancelThread();
}

/**
 * Returns the status of the asynchronous file listing operation
 * @return string with directory listing
 */
string GarminFilebasedDevice::getDirectoryListingXml() {
    return this->readableFileListingXml;
}


void GarminFilebasedDevice::readFitnessUserProfile() {
    Log::dbg("Thread readFitnessUserProfile started");
    string workFile="";
    lockVariables();
    this->threadState = 1; // Working

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.readable) &&  (currentDir.name.compare("FitnessUserProfile") == 0)) {
            workFile = this->baseDirectory + "/" + currentDir.path +"/" + currentDir.basename + "." + currentDir.extension;
        }
    }
    unlockVariables();

    // Check if the device supports reading tcx files
    if (workFile.length() == 0) {
        Log::err("Device does not support reading FitnessUserProfile. Element FitnessUserProfile not found in xml!");
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }

    if (Log::enabledDbg()) { Log::dbg("Opening file "+workFile); }
    std::ifstream in(workFile.c_str());
    if(!in) {
        Log::err("readFitnessUserProfile unable to open file: "+workFile);
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    lockVariables();
    this->fitnessDataTcdXml = buffer.str();
    this->threadState = 3; // Finished
    this->transferSuccessful = true; // Success;
    unlockVariables();
    return;
}

void GarminFilebasedDevice::readFitnessCourses(bool readTrackData) {

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessCourses started"); }

    string workDir="";
    string extension="";
    lockVariables();
    this->threadState = 1; // Working

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.readable) &&  (currentDir.name.compare("FitnessCourses") == 0)) {
            workDir = this->baseDirectory + "/" + currentDir.path;
            extension = currentDir.extension;
            break;
        }
    }
    unlockVariables();

    // Check if the device supports reading crs files
    if (workDir.length() == 0) {
        Log::err("Device does not support reading CRS Files. Element FitnessCourses not found in xml!");
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }

    DIR *dp;
    struct dirent *dirp;
    vector<string> files = vector<string>();

    if((dp = opendir(workDir.c_str())) == NULL) {
        Log::err("Error opening course directory! "+ workDir);

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

    TiXmlElement * folders = new TiXmlElement( "Folders" );
    train->LinkEndChild( folders );

    TiXmlElement * courses = new TiXmlElement( "Courses" );
    train->LinkEndChild( courses );

    // Loop over all files in Courses directory:
    for (unsigned int i = 0;i < files.size();i++) {
        if (files[i].find("."+extension)!=string::npos) {
            if (Log::enabledDbg()) { Log::dbg("Opening file: "+ files[i]); }
            TiXmlDocument doc(workDir + "/" + files[i]);
            if (doc.LoadFile()) {
                TiXmlElement * train = doc.FirstChildElement("TrainingCenterDatabase");
                if (train != NULL) {
                    TiXmlElement * inputCourses = train->FirstChildElement("Courses");
                    while ( inputCourses != NULL) {
                        TiXmlElement * inputCourse =inputCourses->FirstChildElement("Course");
                        while ( inputCourse != NULL) {

                            TiXmlNode * newCourse = inputCourse->Clone();
                            if (!readTrackData) {
                                // Track data must be deleted
                                TiXmlNode * trackNode = newCourse->FirstChildElement("Track");
                                while (trackNode != NULL) {
                                    newCourse->RemoveChild( trackNode );
                                    trackNode = newCourse->FirstChildElement("Track");
                                }

                                TiXmlNode * coursePointNode = newCourse->FirstChildElement("CoursePoint");
                                while (coursePointNode != NULL) {
                                    newCourse->RemoveChild( coursePointNode );
                                    coursePointNode = newCourse->FirstChildElement("CoursePoint");
                                }

                                TiXmlNode * creatorNode = newCourse->FirstChildElement("Creator");
                                while (creatorNode != NULL) {
                                    newCourse->RemoveChild( creatorNode );
                                    creatorNode = newCourse->FirstChildElement("Creator");
                                }
                            }
                            courses->LinkEndChild( newCourse );

                            inputCourse = inputCourse->NextSiblingElement( "Course" );
                        }
                        inputCourses = inputCourses->NextSiblingElement( "Courses" );
                    }
                }
            } else {
                Log::err("Unable to load course file "+files[i]);
            }
        }
    }

    addAuthorXmlElement(train);

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

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessCourses finished"); }
    return;
}

void GarminFilebasedDevice::addAuthorXmlElement(TiXmlElement * parentNode) {
    if (parentNode == NULL) { return; }

    TiXmlElement * authorNode = new TiXmlElement( "Author" );
    authorNode->SetAttribute("xsi:type","Application_t");
    parentNode->LinkEndChild( authorNode );

    TiXmlElement * nameNode = new TiXmlElement( "Name" );
    nameNode->LinkEndChild(new TiXmlText("Garmin Communicator Plug-In"));
    authorNode->LinkEndChild( nameNode );

    TiXmlElement * buildNode = new TiXmlElement( "Build" );
    authorNode->LinkEndChild( buildNode );

    TiXmlElement * versionNode = new TiXmlElement( "Version" );
    buildNode->LinkEndChild( versionNode );

    TiXmlElement * versionMajNode = new TiXmlElement( "VersionMajor" );
    versionMajNode->LinkEndChild(new TiXmlText("2"));
    versionNode->LinkEndChild( versionMajNode );

    TiXmlElement * versionMinNode = new TiXmlElement( "VersionMinor" );
    versionMinNode->LinkEndChild(new TiXmlText("9"));
    versionNode->LinkEndChild( versionMinNode );

    TiXmlElement * versionBuildMajNode = new TiXmlElement( "BuildMajor" );
    versionBuildMajNode->LinkEndChild(new TiXmlText("3"));
    versionNode->LinkEndChild( versionBuildMajNode );

    TiXmlElement * versionBuildMinNode = new TiXmlElement( "BuildMinor" );
    versionBuildMinNode->LinkEndChild(new TiXmlText("0"));
    versionNode->LinkEndChild( versionBuildMinNode );

    TiXmlElement * buildTypeNode = new TiXmlElement( "Type" );
    buildTypeNode->LinkEndChild(new TiXmlText("Release"));
    buildNode->LinkEndChild( buildTypeNode );

    TiXmlElement * buildTimeNode = new TiXmlElement( "Time" );
    buildTimeNode->LinkEndChild(new TiXmlText("Oct 28 2010, 10:21:55"));
    buildNode->LinkEndChild( buildTimeNode );

    TiXmlElement * buildBuilderNode = new TiXmlElement( "Builder" );
    buildBuilderNode->LinkEndChild(new TiXmlText("sqa"));
    buildNode->LinkEndChild( buildBuilderNode );

    TiXmlElement * buildLangNode = new TiXmlElement( "LangID" );
    buildLangNode->LinkEndChild(new TiXmlText("EN"));
    authorNode->LinkEndChild( buildLangNode );

    TiXmlElement * buildPartNode = new TiXmlElement( "PartNumber" );
    buildPartNode->LinkEndChild(new TiXmlText("006-A0160-00"));
    authorNode->LinkEndChild( buildPartNode );

}


void GarminFilebasedDevice::readFitnessWorkouts() {

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessWorkouts started"); }

    string workDir="";
    string extension="";
    lockVariables();
    this->threadState = 1; // Working

    for (list<MassStorageDirectoryType>::iterator it = deviceDirectories.begin(); it != deviceDirectories.end(); ++it) {
        MassStorageDirectoryType currentDir = (*it);
        if ((currentDir.readable) &&  (currentDir.name.compare("FitnessWorkouts") == 0)) {
            workDir = this->baseDirectory + "/" + currentDir.path;
            extension = currentDir.extension;
            break;
        }
    }
    unlockVariables();

    // Check if the device supports reading crs files
    if (workDir.length() == 0) {
        Log::err("Device does not support reading Workout Files. Element FitnessWorkouts not found in xml!");
        lockVariables();
        this->fitnessDataTcdXml = "";
        this->threadState = 3; // Finished
        this->transferSuccessful = false; // Failed;
        unlockVariables();
        return;
    }

    DIR *dp;
    struct dirent *dirp;
    vector<string> files = vector<string>();

    if((dp = opendir(workDir.c_str())) == NULL) {
        Log::err("Error opening workout directory! "+ workDir);

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

    TiXmlElement * folders = new TiXmlElement( "Folders" );
    train->LinkEndChild( folders );

    TiXmlElement * folderWorkouts = new TiXmlElement( "Workouts" );
    folders->LinkEndChild( folderWorkouts );
    TiXmlElement * workoutsRunning = new TiXmlElement( "Running" );
    workoutsRunning->SetAttribute("Name","Running");
    folderWorkouts->LinkEndChild( workoutsRunning );
    TiXmlElement * workoutsBiking = new TiXmlElement( "Biking" );
    workoutsBiking->SetAttribute("Name","Biking");
    folderWorkouts->LinkEndChild( workoutsBiking );
    TiXmlElement * workoutsOther = new TiXmlElement( "Other" );
    workoutsOther->SetAttribute("Name","Other");
    folderWorkouts->LinkEndChild( workoutsOther );

    TiXmlElement * workouts = new TiXmlElement( "Workouts" );
    train->LinkEndChild( workouts );


    // Loop over all files in workout directory:
    for (unsigned int i = 0;i < files.size();i++) {
        if (files[i].find("."+extension)!=string::npos) {
            if (Log::enabledDbg()) { Log::dbg("Opening file: "+ files[i]); }
            TiXmlDocument doc(workDir + "/" + files[i]);
            if (doc.LoadFile()) {
                TiXmlElement * inputTrain = doc.FirstChildElement("TrainingCenterDatabase");
                if (inputTrain != NULL) {
                    // Do folder part
                    TiXmlElement * inputFolders = NULL;
                    TiXmlElement * inputFoldersWorkouts = NULL;
                    inputFolders = inputTrain->FirstChildElement("Folders");
                    if (inputFolders != NULL) { inputFoldersWorkouts = inputFolders->FirstChildElement("Workouts"); }
                    if (inputFoldersWorkouts != NULL) {
                        // Running
                        TiXmlElement * inputFoldersWorkoutsType = inputFoldersWorkouts->FirstChildElement("Running");
                        TiXmlElement * inputWorkoutNameRef = NULL;
                        if (inputFoldersWorkoutsType != NULL) { inputWorkoutNameRef = inputFoldersWorkoutsType->FirstChildElement("WorkoutNameRef");}
                        while (inputWorkoutNameRef != NULL) {
                            TiXmlNode * newNode = inputWorkoutNameRef->Clone();
                            workoutsRunning->LinkEndChild(newNode);
                            inputWorkoutNameRef = inputWorkoutNameRef->NextSiblingElement("WorkoutNameRef");
                        }
                        // Biking
                        inputFoldersWorkoutsType = inputFoldersWorkouts->FirstChildElement("Biking");
                        if (inputFoldersWorkoutsType != NULL) { inputWorkoutNameRef = inputFoldersWorkoutsType->FirstChildElement("WorkoutNameRef");}
                        while (inputWorkoutNameRef != NULL) {
                            workoutsBiking->LinkEndChild(inputWorkoutNameRef->Clone());
                            inputWorkoutNameRef = inputWorkoutNameRef->NextSiblingElement("WorkoutNameRef");
                        }
                        // Other
                        inputFoldersWorkoutsType = inputFoldersWorkouts->FirstChildElement("Other");
                        if (inputFoldersWorkoutsType != NULL) { inputWorkoutNameRef = inputFoldersWorkoutsType->FirstChildElement("WorkoutNameRef");}
                        while (inputWorkoutNameRef != NULL) {
                            workoutsOther->LinkEndChild(inputWorkoutNameRef->Clone());
                            inputWorkoutNameRef = inputWorkoutNameRef->NextSiblingElement("WorkoutNameRef");
                        }
                    }

                    // Do workout part
                    TiXmlElement * inputWorkouts = inputTrain->FirstChildElement("Workouts");
                    if (inputWorkouts != NULL) {
                        TiXmlElement * inputWorkout = inputWorkouts->FirstChildElement("Workout");
                        while (inputWorkout != NULL) {
                            workouts->LinkEndChild(inputWorkout->Clone());
                            inputWorkout = inputWorkout->NextSiblingElement("Workout");
                        }
                    }
                }
            } else {
                Log::err("Unable to load course file "+files[i]);
            }
        }
    }

    addAuthorXmlElement(train);

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

    if (Log::enabledDbg()) { Log::dbg("Thread readFitnessWorkouts finished"); }
    return;
}


int GarminFilebasedDevice::startDirectoryListing(string relativePath, bool computeMd5) {
    lockVariables();
    this->threadState = 1;
    this->readableFileListingFileTypeName = relativePath;
    this->readableFileListingComputeMD5   = computeMd5;
    this->readableFileListingXml = "";
    unlockVariables();

    if (Log::enabledDbg()) Log::dbg("Starting thread to read directory listing from garmin device "+this->displayName);

    this->workType = DIRECTORYLISTING;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int GarminFilebasedDevice::finishDirectoryListing() {
    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}

void GarminFilebasedDevice::cancelDirectoryListing() {
    if (Log::enabledDbg()) { Log::dbg("cancelDirectoryListing for device "+this->displayName); }
    cancelThread();
}


void GarminFilebasedDevice::readDirectoryListing() {
    if (Log::enabledDbg()) { Log::dbg("Thread readDirectoryListing started"); }


    lockVariables();
    string workDir=this->readableFileListingFileTypeName; // contains relative file path
    this->threadState = 1; // Working
    bool doCalculateMd5 = this->readableFileListingComputeMD5;
    unlockVariables();

    TiXmlDocument * output = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    output->LinkEndChild( decl );

    TiXmlElement * dirList = new TiXmlElement( "DirectoryListing" );
    dirList->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/DirectoryListing/v1");
    dirList->SetAttribute("RequestedPath",workDir);
    dirList->SetAttribute("UnitId",this->deviceId);
    dirList->SetAttribute("VolumePrefix",this->baseDirectory);
    output->LinkEndChild( dirList );

    stack<string> directoryList;

    string::size_type loc = workDir.find( "..", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: work directories with .. are not allowed!");
    } else {
        directoryList.push(workDir);
    }

    while (!directoryList.empty()) {
        string relativeWorkDir = directoryList.top();
        string currentWorkDir = this->baseDirectory + "/" + relativeWorkDir;
        directoryList.pop(); // get rid of the element

        DIR *dp;
        struct dirent *dirp;
        if((dp = opendir(currentWorkDir.c_str())) != NULL) {
            while ((dirp = readdir(dp)) != NULL) {
                string fileName = string(dirp->d_name);
                string fullFileName = currentWorkDir+'/'+fileName;
                string relativeFileName = relativeWorkDir+'/'+fileName;
                if (relativeWorkDir.length() == 0) {
                    relativeFileName = fileName;
                }
                bool isDirectory = (dirp->d_type == 4) ? true : false;
                if (Log::enabledDbg()) { Log::dbg("Found file: "+fileName); }
                if ((fileName == ".") || (fileName == "..")) { continue; }

                TiXmlElement * curFile = new TiXmlElement( "File" );
                if (isDirectory) {
                    curFile->SetAttribute("IsDirectory","true");
                    directoryList.push(relativeFileName);
                } else {
                    curFile->SetAttribute("IsDirectory","false");
                }
                curFile->SetAttribute("Path",relativeFileName);

                // Get File Size
                struct stat filestatus;
                stat( fullFileName.c_str(), &filestatus );
                if (!isDirectory) {
                    stringstream ss;
                    ss << filestatus.st_size;
                    curFile->SetAttribute("Size",ss.str());
                }

                // Get the timestamp
                TiXmlElement * timeElem = new TiXmlElement( "CreationTime" );
                timeElem->LinkEndChild(new TiXmlText(GpsFunctions::print_dtime(filestatus.st_mtime-TIME_OFFSET)));
                curFile->LinkEndChild(timeElem);


                if ((!isDirectory) && (doCalculateMd5)) {
                    if (Log::enabledDbg()) { Log::dbg("Calculating MD5 sum of " + fullFileName);}
                    MD5_CTX c;
                    unsigned char md[MD5_DIGEST_LENGTH];
                    unsigned char buf[MD5READBUFFERSIZE];
                    FILE *f = fopen(fullFileName.c_str(),"r");
                    int fd=fileno(f);
                    MD5_Init(&c);
                    for (;;) {
                        int i=read(fd,buf,MD5READBUFFERSIZE);
                        if (i <= 0) break;
                        MD5_Update(&c,buf,(unsigned long)i);
                    }
                    MD5_Final(&(md[0]),&c);
                    string md5="";
                    for (int i=0; i<MD5_DIGEST_LENGTH; i++) {
                        char temp[16];
                        sprintf(temp, "%02x",md[i]);
                        md5 += temp;

                    }
                    curFile->SetAttribute("MD5Sum",md5);
                }
                dirList->LinkEndChild( curFile );
            }
            closedir(dp);
        } else {
            Log::err("Error opening directory! "+ currentWorkDir);
        }
    }

    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    output->Accept( &printer );
    string outputXml = printer.Str();
    delete(output);

    lockVariables();
    this->threadState = 3; // Finished
    this->readableFileListingXml = outputXml;
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

    if (Log::enabledDbg()) { Log::dbg("Thread readDirectoryListing finished"); }
    return;
}
