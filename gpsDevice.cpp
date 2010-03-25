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


bool GpsDevice::startThread() {
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
    pthread_cancel(this->threadId);
}


/*static*/
void * GpsDevice::workerThread(void * pthis) {
    Log::dbg("Thread started");
    GpsDevice * obj = (GpsDevice*)pthis;

    obj->doWork();

    Log::dbg("Thread finished");
    return NULL;
}
