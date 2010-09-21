#ifndef TCXACTIVITIES_H_INCLUDED
#define TCXACTIVITIES_H_INCLUDED
#include <string>

#define TIXML_USE_TICPP
#include "ticpp.h"

#include "TcxActivity.h"

using namespace std;

class TcxActivities
{
public:
    TcxActivities();
    ~TcxActivities();
    void addActivity(TcxActivity* activity);

    TiXmlElement * getTiXml(bool readTrackData, string fitnessDetailId);

    friend TcxActivities& operator<<(TcxActivities& base, TcxActivity* activity);

private:
    vector<TcxActivity*> activityList;
};

#endif // TCXACTIVITIES_H_INCLUDED
