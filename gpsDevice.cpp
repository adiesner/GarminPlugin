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

pthread_mutex_t shareVariables_mtx= PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t      waitThread      = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     waitThreadMutex = PTHREAD_MUTEX_INITIALIZER;



GpsDevice::GpsDevice() : threadStatus(0)
{
    this->displayName = "unknown";
    this->storageCmd = "";
    this->storageDirectory = "/tmp";
}

string GpsDevice::getDeviceDescription()
{

    if (Log::enabledDbg()) Log::dbg("GpsDevice::getDeviceDescription() "+this->displayName);
/*

<?xml version="1.0" encoding="utf-8" standalone="no"?>
<Device xmlns="http://www.garmin.com/xmlschemas/GarminDevice/v2"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd">

  <Model>
    <PartNumber>006-B0000-00</PartNumber>
    <SoftwareVersion>0</SoftwareVersion>
    <Description>An SD Card</Description>
  </Model>
  <Id>4294967295</Id>
  <DisplayName>Removable Disk (F:\\)</DisplayName>
  <MassStorageMode>
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
          <FileExtension>GPX</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>CustomPOI</Name>
      <File>
        <Specification>
          <Identifier>GPI0.0</Identifier>
        </Specification>
        <Location>
          <FileExtension>gpi</FileExtension>
        </Location>
        <TransferDirection>InputToUnit</TransferDirection>
      </File>
    </DataType>
    <DataType>
      <Name>SupplementalMaps</Name>
      <File>
        <Specification>
          <Identifier>SupplementalMaps</Identifier>
        </Specification>
        <Location>
          <Path>Garmin</Path>
          <BaseName>gmapsupp</BaseName>
          <FileExtension>IMG</FileExtension>
        </Location>
        <TransferDirection>InputToUnit</TransferDirection>
      </File>
    </DataType>
    <UpdateFile>
      <PartNumber>006-B0000-00</PartNumber>
      <Version>
        <Major>0</Major>
        <Minor>0</Minor>
      </Version>
    </UpdateFile>
  </MassStorageMode>
</Device>


*/
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
	descr->LinkEndChild(new TiXmlText("An SD Card"));
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
	dispName->LinkEndChild(new TiXmlText("Removable Disk (F:\\)"));
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

    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	doc.Accept( &printer );
    string str = printer.Str();

    if (Log::enabledDbg()) Log::dbg("GpsDevice::getDeviceDescription() Done: "+this->displayName );
    return str;
}

int GpsDevice::startWriteToGps(const string filename, const string xml)
{
    // There shouldn't be a thread running... but who knows...
    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
        this->xmlToWrite = xml;
        this->filenameToWrite = storageDirectory + "/" + filename;
        this->overwriteFile = 2; // not yet asked
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

    if (Log::enabledDbg()) Log::dbg("Saving to file: "+this->filenameToWrite);

    int code = pthread_create(&(this->threadId), NULL, GpsDevice::writeToDevice, (void*)this);

    if (code != 0) {
        Log::err("Creation of thread to store document failed!");
        return 0;  // Error happened
    }

    return 1;
}

int GpsDevice::finishWriteToGps()
{
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
        int status = this->threadStatus;
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

    return status;
}

string GpsDevice::getDisplayName()
{
    return this->displayName;
}

void GpsDevice::setDisplayName(string name)
{
    this->displayName = name;
}

GpsDevice::~GpsDevice()
{
    if (Log::enabledDbg()) Log::dbg("Desctructor of GpsDevice called");
}

void GpsDevice::setStorageDirectory(string directory) {
    this->storageDirectory = directory;
}

void GpsDevice::setStorageCommand(string cmd) {
    cerr << "setStorageCommand" << endl;
    this->storageCmd = cmd;
}

void GpsDevice::userAnswered(const int answer) {
    if (answer == 1) {
        if (Log::enabledDbg()) Log::dbg("User wants file overwritten");
        pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
            this->overwriteFile = 1;
        pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables
    } else {
        if (Log::enabledDbg()) Log::dbg("User wants file to be untouched");
        pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
            this->overwriteFile = 0;
        pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables
    }
    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
        this->threadStatus = 1; /* set back to working */
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

    Log::dbg("Thread wake up signal sending...");
    // wake up thread
    pthread_mutex_lock(&waitThreadMutex);
    pthread_cond_signal(&waitThread);
    pthread_mutex_unlock(&waitThreadMutex);
    Log::dbg("Thread wake up signal was sent!");
}


MessageBox * GpsDevice::getMessage() {
    MessageBox * msg = this->waitingMessage;
    this->waitingMessage = NULL;
    return msg;
}

/*static*/
void * GpsDevice::writeToDevice(void * pthis) {


    Log::dbg("Thread started");
/*
Thread-Status
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    GpsDevice * data = (GpsDevice*)pthis;

    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
        string xml = data->xmlToWrite;
        string filename = data->filenameToWrite;
        string systemCmd = data->storageCmd;
        data->threadStatus = 1; // Working
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

    struct stat stFileInfo;
    int intStat;
    // Attempt to get the file attributes
    intStat = stat(filename.c_str(),&stFileInfo);
    if(intStat == 0) {
        // File exists - we need to ask the user to overwrite
        pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
            data->waitingMessage = new MessageBox(Question, "File "+filename+" exists. Overwrite?", BUTTON_YES | BUTTON_NO , BUTTON_NO, data);
            data->threadStatus = 2;
        pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

        Log::dbg("Thread is going to sleep!");
        pthread_mutex_lock(&waitThreadMutex);
        while (2 == data->threadStatus) {
            pthread_cond_wait(&waitThread, &waitThreadMutex);
        }
        pthread_mutex_unlock(&waitThreadMutex);
        Log::dbg("Thread was woken up!");

        bool doOverwrite = true;
        pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
            if (data->overwriteFile != 1) {
                data->threadStatus = 3;
                data->transferSuccessful = false;
                doOverwrite = false;
            }
        pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

        if (!doOverwrite) {
            Log::dbg("Thread aborted");
            return NULL;
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
            pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
                data->waitingMessage = new MessageBox(Question, "Error executing command: "+systemCmd, BUTTON_OK , BUTTON_OK, NULL);
                data->threadStatus = 2;
            pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

            sleep(1); // give application time to fetch messagebox
            pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
                data->threadStatus = 3;
            pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

            Log::err("Executing user command failed: "+systemCmd);
            return NULL;
        }
    }

    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
        data->threadStatus = 3; // Finished
        data->transferSuccessful = true; // Successfull;
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables

    Log::dbg("Thread ending");
    return NULL;
}

int GpsDevice::getTransferSucceeded() {
    if (this->transferSuccessful) {
        return 1;
    }
    return 0;
}

bool GpsDevice::isDeviceAvailable() {
    struct stat st;
    if(stat(storageDirectory.c_str(),&st) == 0) {
        // directory exists
        return true;
    }
    return false;
}

void GpsDevice::cancelWriteToGps() {
    pthread_cancel(this->threadId);
}
