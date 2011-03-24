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
#ifndef TCXACTIVITY_H_INCLUDED
#define TCXACTIVITY_H_INCLUDED

#include "TcxLap.h"
#include "TcxCreator.h"
#include "TcxTypes.h"

using namespace std;

class TcxActivity
{
public:
    TcxActivity(string id);
    ~TcxActivity();

    void addLap(TcxLap* lap);
    string getId();

    TiXmlElement * getTiXml(bool readTrackData);
    TiXmlElement * getGpxTiXml();

    void setSportType(TrainingCenterDatabase::Sport_t type);
    void setId(string id);

    bool isEmpty();

    /**
     * Returns a string with information about this activity (debug purpose)
     */
    string getOverview();

    friend TcxActivity& operator<<(TcxActivity& activity, TcxLap* lap);
    friend TcxActivity& operator<<(TcxActivity& activity, TcxCreator* creator);

private:
    string id;
    TrainingCenterDatabase::Sport_t sportType;
    vector<TcxLap*> lapList;
    TcxCreator* creator;
};

#endif // TCXACTIVITY_H_INCLUDED
