
#include "configManager.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "log.h"

ConfigManager::ConfigManager() : createdNew(false) {
}

ConfigManager::~ConfigManager() {
    delete configuration;
}

void ConfigManager::readConfiguration() {
    string homeDir = getenv ("HOME");
    this->configurationFile = homeDir + "/.config/garminplugin/garminplugin.xml";

    try
    {
        this->configuration = new TiXmlDocument(this->configurationFile );
        if (this->configuration->LoadFile())
        {
            return;
        }
    }
    catch(ticpp::Exception& ex)
    {
        if (Log::enabledInfo()) Log::info("Failed reading configuration from "+this->configurationFile);
    }

    this->configurationFile = homeDir + "/.garminplugin.xml";
    try
    {
        this->configuration = new TiXmlDocument(this->configurationFile );
        if (this->configuration->LoadFile())
        {
            return;
        }
    }
    catch(ticpp::Exception& ex)
    {
        if (Log::enabledInfo()) Log::info("Failed reading configuration from "+this->configurationFile);
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
        <Device>
          <Name>My Oregon 300</Name>
          <StoragePath>/tmp</StoragePath>
          <StorageCommand></StorageCommand>
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

    try {
        doc->SaveFile(configFile);
        configurationFile = configFile;
    }
    catch(ticpp::Exception& ex)
    {
        if (Log::enabledErr()) Log::err("Failed storing initial configuration to "+configFile);
    }

    return doc;
}

MessageBox * ConfigManager::getMessage() {
    if (!this->createdNew) return NULL;

    return new MessageBox(Question, "A new configuration was created at "+this->configurationFile, BUTTON_OK, BUTTON_OK, NULL);
}
