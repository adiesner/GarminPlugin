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
#ifndef TCXTRACK_H_INCLUDED
#define TCXTRACK_H_INCLUDED
#include "TcxTrackpoint.h"
#include <vector>

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
    string getEndTime();

    double calculateDistanceMeters();
    double calculateTotalTime();
    int getMaxHeartRate();

    bool isEmpty();

private:
    vector<TcxTrackpoint*> trackpointList;



};

#endif // TCXTRACK_H_INCLUDED
