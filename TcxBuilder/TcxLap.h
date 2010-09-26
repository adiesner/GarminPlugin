#ifndef TCXLAP_H_INCLUDED
#define TCXLAP_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"

#include <string>
#include <vector>
#include "TcxTrack.h"
#include "TcxTypes.h"

using namespace std;

class TcxLap
{
public:

    TcxLap();

    ~TcxLap();

    void addTrack(TcxTrack* track);

    TiXmlElement * getTiXml(bool readTrackData);
    TiXmlElement * getGpxTiXml();

    void setTotalTimeSeconds(string time);
    void setDistanceMeters(string distance);
    void setMaximumSpeed(string speed);
    void setCalories(string calories);
    void setAverageHeartRateBpm(string averageBpm);
    void setMaximumHeartRateBpm(string maximumBpm);
    void setIntensity(TrainingCenterDatabase::Intensity_t intensitiy);
    void setCadence(string cadence);
    void setTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method);
    void setNotes(string note);
    void setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type);

    bool isEmpty();

    friend TcxLap& operator<<(TcxLap& base, TcxTrack* track);

private:
    vector<TcxTrack*> trackList;

    void calculateTotalTimeSeconds();
    void calculateDistanceMeters();
    void calculateCalories();
    string getTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method);
    string getIntensity(TrainingCenterDatabase::Intensity_t intensity);
    string getStartTime();

    string totalTimeSeconds;
    string distanceMeters;
    string maximumSpeed;
    string calories;
    string averageHeartRateBpm;
    string maximumHeartRateBpm;
    TrainingCenterDatabase::Intensity_t intensity;
    string cadence;
    TrainingCenterDatabase::TriggerMethod_t triggerMethod;
    string notes;
    TrainingCenterDatabase::CadenceSensorType_t cadenceSensorType;

};

#endif // TCXLAP_H_INCLUDED
