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
#ifndef TCXBASE_H_INCLUDED
#define TCXBASE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"

#include <vector>
#include "TcxActivities.h"
#include "TcxAuthor.h"

using namespace std;

class TcxBase
{
public:
    TcxBase();

    ~TcxBase();

    void addActivities(TcxActivities* activities);

    TiXmlDocument * getTcxDocument(bool readTrackData, string fitnessDetailId);
    TiXmlDocument * getGpxDocument();

    friend TcxBase& operator<<(TcxBase& base, TcxActivities* activities);
    friend TcxBase& operator<<(TcxBase& base, TcxAuthor* author);

private:
    vector<TcxActivities*> activitiesList;
    TcxAuthor * author;


};

#endif // TCXBASE_H_INCLUDED
