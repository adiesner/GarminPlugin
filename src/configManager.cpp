/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2010 <andreas.diesner [AT] gmx [DOT] de>
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


#include "configManager.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "log.h"

ConfigManager::ConfigManager() : configuration(NULL), createdNew(false) {
}

ConfigManager::~ConfigManager() {
    Log::dbg("ConfigManager destructor");
    if (this->configuration != NULL) {
        delete this->configuration;
    }
}

void ConfigManager::readConfiguration() {
    string homeDir = getenv ("HOME");
    this->configurationFile = homeDir + "/.config/garminplugin/garminplugin.xml";

    if (this->configuration != NULL) {
        delete (this->configuration);
        this->configuration = NULL;
    }

    this->configuration = new TiXmlDocument(this->configurationFile );
    if (this->configuration->LoadFile())
    {
        return;
    }

    this->configurationFile = homeDir + "/.garminplugin.xml";
    this->configuration = new TiXmlDocument(this->configurationFile );
    if (this->configuration->LoadFile())
    {
        return;
    }

    configuration = createNewConfiguration();
}

TiXmlDocument * ConfigManager::getConfiguration() {
    return this->configuration;
}

TiXmlDocument * ConfigManager::createNewConfiguration() {
    if (Log::enabledDbg()) Log::dbg("Creating new initial configuration");
    createdNew = true;
    string homeDir = getenv ("HOME");
    string storagePath = homeDir + "/.config";
    struct stat st;
    if(stat(storagePath.c_str(),&st) == 0) {
        // directory exists
        storagePath += "/garminplugin";
        if(stat(storagePath.c_str(),&st) == 0) {
            // directory already exists
            storagePath += "/";
        } else {
            if(mkdir(storagePath.c_str(), 0755) == -1)
            {
                if (Log::enabledErr()) Log::err("Failed to create directory "+storagePath);
                storagePath = homeDir+"/.";
            } else {
                storagePath += "/";
            }
        }
    } else {
        storagePath = homeDir+"/.";
    }

    string configFile = storagePath + "garminplugin.xml";

    TiXmlDocument * doc = new TiXmlDocument();

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
    doc->LinkEndChild( decl );

/*  <GarminPlugin logfile="" level="ERROR">
      <Devices>
        <Device enabled="false">
          <Name>My Oregon 300</Name>
          <StoragePath>/tmp</StoragePath>
          <StorageCommand></StorageCommand>
          <FitnessDataPath></FitnessDataPath>
          <GpxDataPath></GpxDataPath>
        </Device>
      </Devices>
    </GarminPlugin> */
	TiXmlElement * plugin = new TiXmlElement( "GarminPlugin" );
	plugin->SetAttribute("logfile", "");
	plugin->SetAttribute("level", "ERROR");
	doc->LinkEndChild( plugin );

	TiXmlElement * devices = new TiXmlElement( "Devices" );
	plugin->LinkEndChild( devices );

	TiXmlElement * device = new TiXmlElement( "Device" );
	device->SetAttribute("enabled", "false");
	devices->LinkEndChild( device );

	TiXmlElement * name = new TiXmlElement( "Name" );
	name->LinkEndChild(new TiXmlText("Home Directory "+homeDir));
	device->LinkEndChild( name );

	TiXmlElement * storePath = new TiXmlElement( "StoragePath" );
	storePath->LinkEndChild(new TiXmlText(homeDir));
	device->LinkEndChild( storePath );

	TiXmlElement * storageCmd = new TiXmlElement( "StorageCommand" );
	storageCmd->LinkEndChild(new TiXmlText(""));
	device->LinkEndChild( storageCmd );

	TiXmlElement * fitnessPath = new TiXmlElement( "FitnessDataPath" );
	fitnessPath->LinkEndChild(new TiXmlText(""));
	device->LinkEndChild( fitnessPath );

	TiXmlElement * gpxPath = new TiXmlElement( "GpxDataPath" );
	gpxPath->LinkEndChild(new TiXmlText(""));
	device->LinkEndChild( gpxPath );

/*
    <Settings>
        <ForerunnerTools enabled ="true"/>
    </Settings>
*/
    TiXmlElement * settings = new TiXmlElement( "Settings" );
	plugin->LinkEndChild( settings );

    TiXmlElement * forerunnertools = new TiXmlElement( "ForerunnerTools" );
	settings->LinkEndChild( forerunnertools );

	forerunnertools->SetAttribute("enabled", "true");

    doc->SaveFile(configFile);
    configurationFile = configFile;

    return doc;
}

MessageBox * ConfigManager::getMessage() {
    if (!this->createdNew) return NULL;

    return new MessageBox(Question, "A new configuration was created at "+this->configurationFile, BUTTON_OK, BUTTON_OK, NULL);
}
