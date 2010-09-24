
#include "TcxTrack.h"

TcxTrack::TcxTrack() {

}

TcxTrack::~TcxTrack() {
    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); it++ )
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
    for ( it=trackpointList.begin() ; it < trackpointList.end(); it++ )
    {
        TcxTrackpoint* trackpoint = *it;
        xmlTrack->LinkEndChild(trackpoint->getTiXml());
    }
    return xmlTrack;
}

vector<TiXmlElement *> TcxTrack::getGpxTiXml() {
    vector<TiXmlElement *> pointList;

    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); it++ )
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
    for ( it=trackpointList.begin() ; it < trackpointList.end(); it++ )
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
    for ( it=trackpointList.begin() ; it < trackpointList.end(); it++ )
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
