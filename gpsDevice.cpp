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


pthread_mutex_t shareVariables_mtx= PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t      waitThreadCond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t     waitThreadMutex = PTHREAD_MUTEX_INITIALIZER;

GpsDevice::GpsDevice() : threadId (0), progressState(0) {
}

GpsDevice::~GpsDevice() {
    Log::dbg("Destructor of GpsDevice "+ this->displayName + " called");
    cancelThread();
}

bool GpsDevice::startThread() {
    progressState = 0;
    int code = pthread_create(&(this->threadId), NULL, GpsDevice::workerThread, (void*)this);

    if (code != 0) {
        Log::err("Creation of thread failed!");
        return false;  // Error happened
    }
    return true;
}

void GpsDevice::lockVariables() {
    pthread_mutex_lock( &shareVariables_mtx ); // LOCK Shared variables
}

void GpsDevice::unlockVariables() {
    pthread_mutex_unlock( &shareVariables_mtx); // UNLOCK Shared variables
}

void GpsDevice::waitThread() {
    Log::dbg("Thread is going to sleep!");
    pthread_mutex_lock(&waitThreadMutex);
    while (2 == this->threadState) {
        pthread_cond_wait(&waitThreadCond, &waitThreadMutex);
    }
    pthread_mutex_unlock(&waitThreadMutex);
    Log::dbg("Thread was woken up!");
}

void GpsDevice::signalThread() {
    Log::dbg("Thread wake up signal sending...");
    // wake up thread
    pthread_mutex_lock(&waitThreadMutex);
    pthread_cond_signal(&waitThreadCond);
    pthread_mutex_unlock(&waitThreadMutex);
    Log::dbg("Thread wake up signal was sent!");
}

void GpsDevice::cancelThread() {
    Log::dbg("Cancel Thread in GPSDevice für "+this->displayName);
    if (this->threadId > 0) {
        pthread_cancel(this->threadId);
    }
}


/*static*/
void * GpsDevice::workerThread(void * pthis) {
    Log::dbg("Thread started");
    GpsDevice * obj = (GpsDevice*)pthis;

    obj->doWork();

    Log::dbg("Thread finished");
    obj->threadId = 0;
    return NULL;
}

/**
 * Default implementation simply increments the counter
 * with each call to simulate a progress bar
 */
int GpsDevice::getProgress() {
    if (progressState < 100) {
        progressState++;
    } else {
        progressState=0;
    }
    return progressState;
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
int GpsDevice::startReadableFileListing(string dataTypeName, string fileTypeName, bool computeMd5) {
    Log::err("startReadableFileListing is not implemented for device "+this->displayName);
    return 0;
}

/**
 * Returns the status of the asynchronous file listing operation for the mass storage mode device
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GpsDevice::finishReadableFileListing() {
    Log::err("finishReadableFileListing is not implemented for device "+this->displayName);
    return 3;
}

/**
 * Cancels the asynchronous file listing operation for the mass storage mode device
 */
void GpsDevice::cancelReadableFileListing() {
    Log::err("cancelReadableFileListing is not implemented for device "+this->displayName);
}

/**
 * Returns the status of the asynchronous file listing operation
 * @return string with directory listing
 */
string GpsDevice::getDirectoryListingXml() {
    Log::err("getDirectoryListingXml is not implemented for device "+this->displayName);
    // Since this is the default implementation, return empty result
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n\
           <DirectoryListing xmlns=\"http://www.garmin.com/xmlschemas/DirectoryListing/v1\" RequestedPath=\"\" UnitId=\"1234567890\" VolumePrefix=\"\"/>";
}
