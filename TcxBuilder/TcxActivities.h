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
    vector<TiXmlElement*> getGpxTiXml();

    friend TcxActivities& operator<<(TcxActivities& base, TcxActivity* activity);

private:
    vector<TcxActivity*> activityList;
};

#endif // TCXACTIVITIES_H_INCLUDED
