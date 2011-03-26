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

#include "TcxAuthor.h"

TcxAuthor::TcxAuthor() {
    this->name = "Garmin Communicator Plug-In";
    this->versionMajor = "2";
    this->versionMinor = "9";
    this->buildMajor = "1";
    this->buildMinor = "0";
    this->partNumber = "006-A0160-00";
    this->type = "Release";
    this->langId = "EN";
}

TcxAuthor::~TcxAuthor() {
}

TiXmlElement * TcxAuthor::getTiXml() {
    TiXmlElement * xmlAuthor = new TiXmlElement("Author");
    xmlAuthor->SetAttribute("xsi:type","Application_t");

    TiXmlElement * xmlName = new TiXmlElement("Name");
    xmlName->LinkEndChild(new TiXmlText(this->name));
    xmlAuthor->LinkEndChild(xmlName);

    TiXmlElement * xmlBuild = new TiXmlElement("Build");
    xmlAuthor->LinkEndChild(xmlBuild);

    TiXmlElement * xmlLangId = new TiXmlElement("LangID");
    xmlLangId->LinkEndChild(new TiXmlText(this->langId));
    xmlAuthor->LinkEndChild(xmlLangId);

    TiXmlElement * xmlPartNumber = new TiXmlElement("PartNumber");
    xmlPartNumber->LinkEndChild(new TiXmlText(this->partNumber));
    xmlAuthor->LinkEndChild(xmlPartNumber);

    TiXmlElement * xmlVersion = new TiXmlElement("Version");
    TiXmlElement * xmlVerMajor = new TiXmlElement("VersionMajor");
    xmlVerMajor->LinkEndChild(new TiXmlText(this->versionMajor));
    TiXmlElement * xmlVerMinor = new TiXmlElement("VersionMinor");
    xmlVerMinor->LinkEndChild(new TiXmlText(this->versionMinor));
    xmlVersion->LinkEndChild(xmlVerMajor);
    xmlVersion->LinkEndChild(xmlVerMinor);
    xmlBuild->LinkEndChild(xmlVersion);

    if (this->type.length() > 0) {
        TiXmlElement * xmlType = new TiXmlElement("Type");
        xmlType->LinkEndChild(new TiXmlText(this->type));
        xmlBuild->LinkEndChild(xmlType);
    }

    if (this->buildMajor.length() > 0) {
        TiXmlElement * xmlBuildMajor = new TiXmlElement("BuildMajor");
        xmlBuildMajor->LinkEndChild(new TiXmlText(this->buildMajor));
        TiXmlElement * xmlBuildMinor = new TiXmlElement("BuildMinor");
        xmlBuildMinor->LinkEndChild(new TiXmlText(this->buildMinor));
        xmlVersion->LinkEndChild(xmlBuildMajor);
        xmlVersion->LinkEndChild(xmlBuildMinor);
    }

    return xmlAuthor;
}

void TcxAuthor::setName(string name) {
    this->name = name;
}

void TcxAuthor::setVersion(string version) {
    unsigned int cutAt = version.find_first_of(".");
    if ((cutAt != version.npos ) && (cutAt > 0))
    {
        this->versionMajor = version.substr(0,cutAt);
        this->versionMinor = version.substr(cutAt+1);
    } else {
        this->versionMajor = version;
        this->versionMinor = "0";
    }
}

void TcxAuthor::setVersion(string major, string minor) {
    this->versionMajor = major;
    this->versionMinor = minor;
}

void TcxAuthor::setBuild(string build) {
    unsigned int cutAt = build.find_first_of(".");
    if ((cutAt != build.npos ) && (cutAt > 0))
    {
        this->buildMajor = build.substr(0,cutAt);
        this->buildMinor = build.substr(cutAt+1);
    } else {
        this->buildMajor = build;
        this->buildMinor = "0";
    }
}

void TcxAuthor::setBuild(string major, string minor) {
    this->buildMajor = major;
    this->buildMinor = minor;
}

void TcxAuthor::setType(string type) {
    this->type = type;
}

void TcxAuthor::setPartNumber(string number) {
    this->partNumber = number;
}

void TcxAuthor::setLangId(string id) {
    this->langId = id;
}
