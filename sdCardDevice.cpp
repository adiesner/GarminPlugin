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
#include "sdCardDevice.h"
#include <dirent.h>

SDCardDevice::SDCardDevice()
{
    this->displayName = "SD Card";
    this->fitnessFileExtension = "tcx";
}

SDCardDevice::~SDCardDevice() {
    Log::dbg("SDCardDevice destructor");
}

bool SDCardDevice::isDeviceAvailable() {
    struct stat st;

    if ((this->gpxDirectory.length() > 0) && (stat(this->gpxDirectory.c_str(),&st) == 0)) {
        // Directory exists
        return true;
    }

    if ((this->fitnessDirectory.length() > 0) && (stat(this->fitnessDirectory.c_str(),&st) == 0)) {
        // Directory exists
        return true;
    }

    Log::dbg("Device is not available: "+this->displayName);
    return false;
}

void SDCardDevice::setPathesFromConfiguration() {
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
                if (nameText.compare("FitnessHistory") == 0) {
                    node2 = node->FirstChildElement("File");
                    while (node2 != NULL) {

                        TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                        string transDir = transferDirection->GetText();
                        if ((transDir.compare("OutputFromUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
                            TiXmlElement * loc = NULL;
                            if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("Path"); }
                            if (node2!=NULL) {
                                this->fitnessDirectory = this->baseDirectory + "/" + node2->GetText();
                                Log::dbg("Fitness directory is: "+this->fitnessDirectory);
                            }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("FileExtension"); }
                            if (node2!=NULL) {
                                this->fitnessFileExtension = node2->GetText();
                            }
                        }
                        node2 = node2->NextSiblingElement("File");
                    }
                }
            }
            node = node->NextSiblingElement( "DataType" );
        }
    }
}

int SDCardDevice::startReadFitnessData()
{
    if (Log::enabledDbg()) Log::dbg("Starting thread to read from garmin device");

    this->workType = READFITNESS;

    if (startThread()) {
        return 1;
    }

    return 0;
}

int SDCardDevice::finishReadFitnessData()
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


void SDCardDevice::doWork() {
    if (this->workType == WRITEGPX) {
        this->writeGpxFile();
    } else if (this->workType == READFITNESS) {
        this->readFitnessDataFromDevice();
    } else {
        Log::err("Work Type not implemented!");
    }
}


void * SDCardDevice::readFitnessDataFromDevice() {
    Log::dbg("Thread readFitnessData started");
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
        return NULL;
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
                        TiXmlNode * newAct = inputActivity->Clone();
                        activities->LinkEndChild( newAct );

                        inputActivity = inputActivity->NextSiblingElement( "Activity" );
                        if (Log::enabledDbg()) { Log::dbg("Adding activity from file "+files[i]); }
                    }
                    inputActivities = inputActivities->NextSiblingElement( "Activities" );
                }
            } else {
                Log::err("Unable to load fitness file "+files[i]);
            }
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
    return NULL;
}


string SDCardDevice::getFitnessData() {
    return this->fitnessDataTcdXml;
}


/*static*/
TiXmlDocument * SDCardDevice::getDefaultConfiguration(string devicename, string gpxpath, string tcxpath) {

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
    doc->LinkEndChild( decl );

    /*<Device xmlns="http://www.garmin.com/xmlschemas/GarminDevice/v2"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd">*/

	TiXmlElement * device = new TiXmlElement( "Device" );
    device->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/GarminDevice/v2");
    device->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    device->SetAttribute("xsi:schemaLocation", "http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd");
    doc->LinkEndChild( device );

/*  <Model>
    <PartNumber>006-B0000-00</PartNumber>
    <SoftwareVersion>0</SoftwareVersion>
    <Description>An SD Card</Description>
  </Model> */
	TiXmlElement * model = new TiXmlElement( "Model" );
	TiXmlElement * partnumber = new TiXmlElement( "PartNumber" );
	partnumber->LinkEndChild(new TiXmlText("006-B0000-00"));
	TiXmlElement * version = new TiXmlElement( "SoftwareVersion" );
	version->LinkEndChild(new TiXmlText("0"));
	TiXmlElement * descr = new TiXmlElement( "Description" );
	descr->LinkEndChild(new TiXmlText(devicename));
	model->LinkEndChild(partnumber);
	model->LinkEndChild(version);
	model->LinkEndChild(descr);
    device->LinkEndChild( model );

/*  <Id>4294967295</Id> */
	TiXmlElement * id = new TiXmlElement( "Id" );
	id->LinkEndChild(new TiXmlText("4294967295"));
	device->LinkEndChild(id);
/*  <DisplayName>Removable Disk (F:\\)</DisplayName>*/
	TiXmlElement * dispName = new TiXmlElement( "DisplayName" );
	dispName->LinkEndChild(new TiXmlText(devicename));
	device->LinkEndChild(dispName);

    TiXmlElement * massStorage = new TiXmlElement( "MassStorageMode" );
    device->LinkEndChild(massStorage);

/*
    <DataType>
      <Name>GPSData</Name>
      <File>
        <Specification>
          <Identifier>
          http://www.topografix.com/GPX/1/1</Identifier>
          <Documentation>
          http://www.topografix.com/GPX/1/1/gpx.xsd</Documentation>
        </Specification>
        <Location>
          <Path>Garmin</Path>
          <FileExtension>pgx</FileExtension>
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

    TiXmlElement * filePath = new TiXmlElement( "Path" );
   	filePath->LinkEndChild(new TiXmlText(gpxpath));
    loc->LinkEndChild(filePath);

    TiXmlElement * fileEx = new TiXmlElement( "FileExtension" );
   	fileEx->LinkEndChild(new TiXmlText("gpx"));
    loc->LinkEndChild(fileEx);

    TiXmlElement * transferDir = new TiXmlElement( "TransferDirection" );
    transferDir->LinkEndChild(new TiXmlText("InputOutput"));
    file->LinkEndChild(transferDir);


    /*
    <DataType>
      <Name>FitnessHistory</Name>
      <File>
        <Location>
          <Path>Garmin</Path>
          <FileExtension>tcx</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    */

    // Only offer FitnessHistory as device capability if a path was set
    if (tcxpath.length() > 0) {
        dataTypes = new TiXmlElement( "DataType" );
        massStorage->LinkEndChild(dataTypes);
        name = new TiXmlElement( "Name" );
        name->LinkEndChild(new TiXmlText("FitnessHistory"));
        dataTypes->LinkEndChild(name);

        file = new TiXmlElement( "File" );
        dataTypes->LinkEndChild(file);

        loc = new TiXmlElement( "Location" );
        file->LinkEndChild(loc);

        filePath = new TiXmlElement( "Path" );
        filePath->LinkEndChild(new TiXmlText(tcxpath));
        loc->LinkEndChild(filePath);

        fileEx = new TiXmlElement( "FileExtension" );
        fileEx->LinkEndChild(new TiXmlText("tcx"));
        loc->LinkEndChild(fileEx);

        transferDir = new TiXmlElement( "TransferDirection" );
        transferDir->LinkEndChild(new TiXmlText("InputOutput"));
        file->LinkEndChild(transferDir);
    }

    return doc;
}
