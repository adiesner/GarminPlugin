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

#include "TcxActivity.h"

TcxActivity::TcxActivity(string id) {
    this->id = id;
    this->creator = NULL;
    this->sportType = TrainingCenterDatabase::Other;
}

TcxActivity::~TcxActivity() {
    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); ++it )
    {
        TcxLap* lap = *it;
        delete(lap);
    }
    lapList.clear();

    if (this->creator != NULL) {
        delete(this->creator);
    }
}

void TcxActivity::addLap(TcxLap* lap) {
    this->lapList.push_back(lap);
}

TiXmlElement * TcxActivity::getTiXml(bool readTrackData) {
    TiXmlElement * xmlActivity = new TiXmlElement("Activity");

    switch (this->sportType) {
        case TrainingCenterDatabase::Running:
            xmlActivity->SetAttribute("Sport","Running");
            break;
        case TrainingCenterDatabase::Biking:
            xmlActivity->SetAttribute("Sport","Biking");
            break;
        default:
            xmlActivity->SetAttribute("Sport","Other");
    }

    TiXmlElement * xmlId = new TiXmlElement("Id");
    xmlActivity->LinkEndChild(xmlId);
    xmlId->LinkEndChild(new TiXmlText(this->id));

    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); ++it )
    {
        TcxLap* lap = *it;
        xmlActivity->LinkEndChild( lap->getTiXml(readTrackData) );
    }

    if (this->creator != NULL) {
        xmlActivity->LinkEndChild(this->creator->getTiXml());
    }
    return xmlActivity;
}

TiXmlElement * TcxActivity::getGpxTiXml() {
    TiXmlElement* trk = new TiXmlElement("trk");

    TiXmlElement * gpxname = new TiXmlElement("name");
    trk->LinkEndChild(gpxname);
    gpxname->LinkEndChild(new TiXmlText(this->id));

    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); ++it )
    {
        TcxLap* lap = *it;
        trk->LinkEndChild( lap->getGpxTiXml() );
    }
    return trk;
}

string TcxActivity::getId() {
    return this->id;
}

TcxActivity& operator<<(TcxActivity& activity, TcxLap* lap)
{
    activity.addLap(lap);
    return activity;
}

TcxActivity& operator<<(TcxActivity& activity, TcxCreator* creator)
{
    if (activity.creator != NULL) {
        delete activity.creator;
    }
    activity.creator = creator;
    return activity;
}

void TcxActivity::setSportType(TrainingCenterDatabase::Sport_t type) {
    this->sportType = type;
}

void TcxActivity::setId(string id) {
    this->id = id;
}

bool TcxActivity::isEmpty() {
    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); ++it )
    {
        TcxLap* lap = *it;
        if (!lap->isEmpty()) {
            return false;
        }
    }
    return true;
}

string TcxActivity::getOverview() {
    stringstream ss;
    ss << this->id;

    ss << " Laps: " << lapList.size() << "(";

    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); ++it )
    {
        TcxLap* lap = *it;
        ss << lap->getDistance();
        if ((it+1) < lapList.end()) {
            ss << ",";
        }
    }
    ss << ")";
    return ss.str();
}
