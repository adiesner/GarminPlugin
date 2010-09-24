#ifndef TCXTRACK_H_INCLUDED
#define TCXTRACK_H_INCLUDED
#include "TcxTrackpoint.h"
#include <vector>

#define TIXML_USE_TICPP
#include "ticpp.h"

using namespace std;

class TcxTrack
{
public:
    TcxTrack();

    ~TcxTrack();

    void addTrackpoint(TcxTrackpoint* track);

    TiXmlElement * getTiXml();
    vector<TiXmlElement *> getGpxTiXml();

    friend TcxTrack& operator<<(TcxTrack& base, TcxTrackpoint* track);

    string getStartTime();

    double calculateDistanceMeters();
    double calculateTotalTime();

private:
    vector<TcxTrackpoint*> trackpointList;



};

#endif // TCXTRACK_H_INCLUDED
