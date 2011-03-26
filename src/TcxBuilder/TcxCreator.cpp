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

#include "TcxCreator.h"

TcxCreator::TcxCreator() {
    this->name = "Unknown";
    this->unitId = "3549600474"; // A random unit id found on the web for the Edge 705
    this->productId = "625";     // A random product id found on the web for the Edge 705
    this->versionMajor = "0";
    this->versionMinor = "0";
    this->buildMajor = "";
    this->buildMinor = "";
}

TcxCreator::~TcxCreator() {
}

TiXmlElement * TcxCreator::getTiXml() {
    TiXmlElement * xmlCreator = new TiXmlElement("Creator");
    xmlCreator->SetAttribute("xsi:type","Device_t");

    TiXmlElement * xmlName = new TiXmlElement("Name");
    xmlName->LinkEndChild(new TiXmlText(this->name));
    xmlCreator->LinkEndChild(xmlName);

    TiXmlElement * xmlUnitId = new TiXmlElement("UnitId");
    xmlUnitId->LinkEndChild(new TiXmlText(this->unitId));
    xmlCreator->LinkEndChild(xmlUnitId);

    TiXmlElement * xmlProductId = new TiXmlElement("ProductID");
    xmlProductId->LinkEndChild(new TiXmlText(this->productId));
    xmlCreator->LinkEndChild(xmlProductId);

    TiXmlElement * xmlVersion = new TiXmlElement("Version");
    TiXmlElement * xmlVerMajor = new TiXmlElement("VersionMajor");
    xmlVerMajor->LinkEndChild(new TiXmlText(this->versionMajor));
    TiXmlElement * xmlVerMinor = new TiXmlElement("VersionMinor");
    xmlVerMinor->LinkEndChild(new TiXmlText(this->versionMinor));
    xmlVersion->LinkEndChild(xmlVerMajor);
    xmlVersion->LinkEndChild(xmlVerMinor);
    xmlCreator->LinkEndChild(xmlVersion);

    if (this->buildMajor.length() > 0) {
        TiXmlElement * xmlBuildMajor = new TiXmlElement("BuildMajor");
        xmlBuildMajor->LinkEndChild(new TiXmlText(this->buildMajor));
        TiXmlElement * xmlBuildMinor = new TiXmlElement("BuildMinor");
        xmlBuildMinor->LinkEndChild(new TiXmlText(this->buildMinor));
        xmlVersion->LinkEndChild(xmlBuildMajor);
        xmlVersion->LinkEndChild(xmlBuildMinor);
    }

    return xmlCreator;
}

void TcxCreator::setName(string name) {
    this->name = name;
}

void TcxCreator::setUnitId(string id) {
    this->unitId = id;
}

void TcxCreator::setProductId(string id) {
    this->productId = id;
}

void TcxCreator::setVersion(string version) {
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

void TcxCreator::setVersion(string major, string minor) {
    this->versionMajor = major;
    this->versionMinor = minor;
}

void TcxCreator::setBuild(string build) {
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

void TcxCreator::setBuild(string major, string minor) {
    this->buildMajor = major;
    this->buildMinor = minor;
}
