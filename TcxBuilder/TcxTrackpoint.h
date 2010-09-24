#ifndef TCXTRACKPOINT_H_INCLUDED
#define TCXTRACKPOINT_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "TcxTypes.h"

using namespace std;

class TcxTrackpoint
{
public:

    TcxTrackpoint(string time, string latitude, string longitude);
    TcxTrackpoint(string time);

    ~TcxTrackpoint();

    void setPosition(string latitude, string longitude);
    void setAltitudeMeters(string altitude);
    void setDistanceMeters(string distance);
    void setHeartRateBpm(string heartrate);
    void setCadence(string cadence);
    void setSensorState(TrainingCenterDatabase::SensorState_t state);
    void setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type);
    string getTime();
    double calculateDistanceTo(double totalTrackDistance, TcxTrackpoint * nextPoint);

    bool hasCoordinates();

    TiXmlElement * getTiXml();
    TiXmlElement * getGpxTiXml();

private:
    void initializeVariables();

    string time;
    string longitude;
    string latitude;
    string altitudeMeters;
    string distanceMeters;
    string heartRateBpm;
    string cadence;
    TrainingCenterDatabase::SensorState_t sensorState;
    TrainingCenterDatabase::CadenceSensorType_t cadenceSensorType;

};

#endif // TCXTRACKPOINT_H_INCLUDED
