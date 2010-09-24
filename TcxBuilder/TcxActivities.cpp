#include "TcxActivities.h"

TcxActivities::TcxActivities() {
}

TcxActivities::~TcxActivities() {
    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); it++ )
    {
        TcxActivity* activity = *it;
        delete(activity);
    }
    activityList.clear();
}

void TcxActivities::addActivity(TcxActivity* activity) {
    this->activityList.push_back(activity);
}

TiXmlElement * TcxActivities::getTiXml(bool readTrackData, string fitnessDetailId) {
    TiXmlElement * xmlActivities = new TiXmlElement("Activities");

    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); it++ )
    {
        TcxActivity* activity = *it;
        if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(activity->getId())==0)) {
            xmlActivities->LinkEndChild( activity->getTiXml(readTrackData) );
        }
    }
    return xmlActivities;
}

vector<TiXmlElement*> TcxActivities::getGpxTiXml() {
    vector<TiXmlElement*> trkElements;

    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); it++ )
    {
        TcxActivity* activity = *it;
        trkElements.push_back(activity->getGpxTiXml());
    }
    return trkElements;
}

TcxActivities& operator<<(TcxActivities& activities, TcxActivity* activity)
{
    activities.addActivity(activity);
    return activities;
}
