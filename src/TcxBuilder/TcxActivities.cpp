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
#include "TcxActivities.h"

// needed for sort
#include <algorithm>

TcxActivities::TcxActivities() {
}

TcxActivities::~TcxActivities() {
    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); ++it )
    {
        TcxActivity* activity = *it;
        delete(activity);
    }
    activityList.clear();
}

void TcxActivities::addActivity(TcxActivity* activity) {
    this->activityList.push_back(activity);
}

/**
 * Sort two TcxActivity
 */
bool activitySorter (TcxActivity * a,TcxActivity * b) {
    string aId=a->getId();
    string bId=b->getId();

    return (aId.compare(bId) > 0);
}

TiXmlElement * TcxActivities::getTiXml(bool readTrackData, string fitnessDetailId) {
    TiXmlElement * xmlActivities = new TiXmlElement("Activities");

    // sort activities, newest on top
    sort (activityList.begin(), activityList.end(), activitySorter);

    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); ++it )
    {
        TcxActivity* activity = *it;
        if (!activity->isEmpty()) {
            if ((fitnessDetailId.length() == 0) || (fitnessDetailId.compare(activity->getId())==0)) {
                xmlActivities->LinkEndChild( activity->getTiXml(readTrackData) );
            }
        }
    }
    return xmlActivities;
}

vector<TiXmlElement*> TcxActivities::getGpxTiXml() {
    vector<TiXmlElement*> trkElements;

    vector<TcxActivity*>::iterator it;
    for ( it=activityList.begin() ; it < activityList.end(); ++it )
    {
        TcxActivity* activity = *it;
        if (!activity->isEmpty()) {
            trkElements.push_back(activity->getGpxTiXml());
        }
    }
    return trkElements;
}

TcxActivities& operator<<(TcxActivities& activities, TcxActivity* activity)
{
    activities.addActivity(activity);
    return activities;
}
