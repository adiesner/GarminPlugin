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

#include "TcxTrack.h"

TcxTrack::TcxTrack() {

}

TcxTrack::~TcxTrack() {
    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        delete(trackpoint);
    }
    trackpointList.clear();
}

void TcxTrack::addTrackpoint(TcxTrackpoint* point) {
    this->trackpointList.push_back(point);
}

TiXmlElement * TcxTrack::getTiXml() {
    TiXmlElement * xmlTrack = new TiXmlElement("Track");
    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        xmlTrack->LinkEndChild(trackpoint->getTiXml());
    }
    return xmlTrack;
}

vector<TiXmlElement *> TcxTrack::getGpxTiXml() {
    vector<TiXmlElement *> pointList;

    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        if (trackpoint->hasCoordinates()) {
            pointList.push_back(trackpoint->getGpxTiXml());
        }
    }
    return pointList;
}

TcxTrack& operator<<(TcxTrack& track, TcxTrackpoint* point)
{
    track.addTrackpoint(point);
    return track;
}

string TcxTrack::getStartTime() {
    vector<TcxTrackpoint*>::iterator it;
    string startTime = "";
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        startTime = trackpoint->getTime();
        if (startTime.length() > 0) {
            break;
        }
    }
    return startTime;
}

double TcxTrack::calculateDistanceMeters() {
    double totalDistance = 0;

    vector<TcxTrackpoint*>::iterator it;

    TcxTrackpoint* lastTrackpoint = NULL;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        if (NULL != lastTrackpoint) {
            totalDistance += lastTrackpoint->calculateDistanceTo(totalDistance, trackpoint);
        }
        lastTrackpoint = trackpoint;
    }

    // Set total distance to last point
    if (NULL != lastTrackpoint) {
        lastTrackpoint->calculateDistanceTo(totalDistance, lastTrackpoint);
    }
    return totalDistance;
}

double TcxTrack::calculateTotalTime() {
    double totalTimeSeconds = 0;

    if ((trackpointList.front() != NULL) && (trackpointList.back() != NULL)) {
        struct tm start={0,0,0,0,0,0,0,0,0};
        struct tm end={0,0,0,0,0,0,0,0,0};
        if ((strptime(trackpointList.front()->getTime().c_str(), "%FT%TZ",&start) != NULL) &&
            (strptime(trackpointList.back()->getTime().c_str(),  "%FT%TZ",&end) != NULL)) {
            time_t tstart, tend;
            tstart = mktime(&start);
            tend = mktime(&end);
            totalTimeSeconds = difftime (tend,tstart);
        }
    }
    return totalTimeSeconds;
}

int TcxTrack::getMaxHeartRate() {
    int maxHeartRate = 0;

    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        string heartRate = trackpoint->getHeartRateBpm();
        if (heartRate.length()>0) {
        	stringstream ss(heartRate);
        	int currentRate;
        	ss >> currentRate;
        	maxHeartRate = (currentRate > maxHeartRate) ? currentRate : maxHeartRate;
        }
    }

    return maxHeartRate;
}


bool TcxTrack::isEmpty() {
    return trackpointList.empty();
}
