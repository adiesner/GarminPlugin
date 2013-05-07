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


#include "deviceManager.h"
#include "log.h"
#include <mntent.h>
#include <dirent.h>
#include "garminFilebasedDevice.h"

#include "edge305Device.h"
#include "gpsFunctions.h"

#include <algorithm>
#include <string>

DeviceManager::DeviceManager()
:configuration(0)
{
}

DeviceManager::~DeviceManager() {
    if (Log::enabledDbg()) Log::dbg("DeviceManager destructor");
    while (gpsDeviceList.size() > 0)
    {
        GpsDevice *dev = gpsDeviceList.back();
        gpsDeviceList.pop_back();
        delete(dev);
    }
}


const std::string DeviceManager::getDevicesXML()
{
    // <?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<Devices xmlns=\"http://www.garmin.com/xmlschemas/PluginAPI/v1\">\n<Device DisplayName=\"Oregon (/mnt/Oregon/)\" Number=\"0\"/>\n</Devices>\n
    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
	TiXmlElement * devices = new TiXmlElement( "Devices" );
    devices->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/PluginAPI/v1");

    int deviceCount = 0;

    vector<GpsDevice*>::iterator it=gpsDeviceList.begin();
    while(it != gpsDeviceList.end()){
        // Delete devices that are no longer available
        if( !(*it)->isDeviceAvailable() ){
            delete *it;
            it = gpsDeviceList.erase(it);
            continue;
        } else {
            TiXmlElement *device = new TiXmlElement ( "Device" );
            device->SetAttribute("DisplayName", (*it)->getDisplayName());
            device->SetAttribute("Number", deviceCount);
            devices->LinkEndChild( device );
            deviceCount++;
        }
        ++it;
    }

    if (Log::enabledDbg()) {
        std::ostringstream dbgOut;
        dbgOut << "getDeviceXML returns " << deviceCount << " devices";
        Log::dbg(dbgOut.str());
    }

    doc.LinkEndChild( decl );
    doc.LinkEndChild( devices );

    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	doc.Accept( &printer );
    string str = printer.Str();

    return str;
}

void DeviceManager::startFindDevices() {
    // Think about putting this routine into a thread when devices will be supported that take more time to search for

    // Remove active devices
    while (!gpsDeviceList.empty())
    {
        GpsDevice *dev = gpsDeviceList.back();
        gpsDeviceList.pop_back();
        delete(dev);
    }

    FILE *mounts = NULL;
    struct mntent *ent = NULL;
    mounts = setmntent("/etc/mtab", "r");

    bool searchGarmin = true;
    string backupPath = "";
    if (this->configuration != NULL) {
        TiXmlElement * pRoot = this->configuration->FirstChildElement( "GarminPlugin" );
        TiXmlElement * settings = NULL;
        TiXmlElement * ftools = NULL;
        TiXmlElement * backup = NULL;

        if (pRoot != NULL) { settings = pRoot->FirstChildElement("Settings"); }
        if (settings != NULL) {
        	ftools = settings->FirstChildElement("ForerunnerTools");
        	backup = settings->FirstChildElement("BackupWorkouts");
        } else {
        	Log::dbg("settings is null!");
        }
        if (ftools != NULL) {
            searchGarmin = getXmlBoolAttribute(ftools, "enabled", true);
        } else {
			Log::dbg("Xml Element ForerunnerTools is null!");
		}
        if (backup != NULL) {
            bool doBackup = getXmlBoolAttribute(backup, "enabled", false);
            if (doBackup) {
            	backupPath = backup->Attribute("path");
            } else {
            	backupPath = "";
            }
        } else {
			Log::dbg("Xml Element BackupWorkouts is null!");
		}
    }

    Log::dbg("Searching for Edge705/Oregon300/...");
    while ( (ent = getmntent(mounts)) != NULL ) {
        string filesystype = ent->mnt_type;
        string mountPath = ent->mnt_dir;
        if (filesystype.compare("vfat") == 0) {
            Log::dbg("Searching on ["+mountPath+"] ["+filesystype+"]");
            GpsDevice *dev = createGarminDeviceFromPath(mountPath, NULL);
            if (dev != NULL) {
            	dev->setBackupPath(backupPath);
                gpsDeviceList.push_back(dev);
            }
        } else {
            Log::dbg("Not searching on ["+mountPath+"] ["+filesystype+"] - wrong fstype.");
        }
    }

    string deviceName;
    if (searchGarmin) {
        // Search for garmin 305
        deviceName = Edge305Device::getAttachedDeviceName();
        if (deviceName.length() > 0) {  // Found a device
            Log::dbg("Found device via garmintools: "+deviceName);
            Edge305Device * device = new Edge305Device(deviceName);
            device->setBackupPath(backupPath);
            gpsDeviceList.push_back(device);
        }
    } else {
        Log::dbg("Search via garmintools is disabled!");
    }

    // Now create virtual devices devices from configuration

    if (this->configuration != NULL) {
        TiXmlElement * pRoot = this->configuration->FirstChildElement( "GarminPlugin" );
        if (pRoot != NULL) {
            TiXmlElement * devices = pRoot->FirstChildElement("Devices");
            TiXmlElement * device = devices->FirstChildElement("Device");
            while ( device != NULL )
            {
                bool deviceEnabled = getXmlBoolAttribute(device, "enabled", true);;
                string storagePath = "";
                string storageCmd = "";
                string fitnessPath = "";
                string gpxPath = "";
                TiXmlElement * dir = device->FirstChildElement("StoragePath");
                if ((dir) && (dir->GetText() != NULL)) {
                    storagePath = dir->GetText();
                }
                TiXmlElement * cmd = device->FirstChildElement("StorageCommand");
                if ((cmd) && (cmd->GetText() != NULL)) {
                    storageCmd = cmd->GetText();
                }
                TiXmlElement * fitness = device->FirstChildElement("FitnessDataPath");
                if ((fitness) && (fitness->GetText() != NULL)) {
                    fitnessPath = fitness->GetText();
                }
                TiXmlElement * gpxData = device->FirstChildElement("GpxDataPath");
                if ((gpxData) && (gpxData->GetText() != NULL)) {
                    gpxPath = gpxData->GetText();
                }

                GpsDevice * currentDevice = NULL;
                TiXmlElement * name = device->FirstChildElement("Name");

                if ((!deviceEnabled) && (Log::enabledDbg())) {
                    if ((name!=NULL) && (name->GetText() != NULL)) {
                        string outputName = name->GetText();
                        Log::dbg("Found disabled device "+outputName+" in configuration.");
                    } else {
                        Log::dbg("Found disabled device with no name in configuration.");
                    }
                }

                if ((deviceEnabled) && (name!=NULL)) {
                    if (name->GetText() != NULL) {
                        string devName = name->GetText();
                        for(unsigned int i=0; i < gpsDeviceList.size(); i++)
                        {
                            // Device exists, and is configured in configuration
                            if (gpsDeviceList[i]->getDisplayName().compare(name->GetText()) == 0) {
                                currentDevice = gpsDeviceList[i];
                            }
                        }
                        if (currentDevice == NULL) { // no device found
                            Log::dbg("Creating device "+devName+" from configuration.");

                            currentDevice = createGarminDeviceFromPath(storagePath, NULL);
                            if (currentDevice == NULL) {
                                if (Log::enabledDbg()) { Log::dbg("Device from configuration - no XML found for "+devName); }

                                //TODO: Create a pseudo configuration file for this device
                                TiXmlDocument *doc = createMinimalGarminConfig(devName);
                                if (fitnessPath.length() > 0) {
                                    doc = addTcxProfile(doc, fitnessPath);
                                }
                                if (gpxPath.length() > 0) {
                                    doc = addGpxProfile(doc, gpxPath);
                                } else if ((gpxPath.length() == 0) && (fitnessPath.length() == 0) && (storageCmd.length() > 0)) {
                                    // Probably an old configuration. Required to add GpxProfile with empty path
                                    doc = addGpxProfile(doc, ".");
                                }
                                currentDevice = createGarminDeviceFromPath(storagePath, doc);
                                delete(doc);
                            } else {
                                Log::dbg("Created device "+devName+" from existing GarminDevice.xml configuration.");
                            }

                            if (currentDevice != NULL) {
                            	currentDevice->setBackupPath(backupPath);
                                gpsDeviceList.push_back(currentDevice);
                            }
                        } else {
                            Log::dbg("Ignoring device "+devName+" from configuration - existing device with same name exists.");
                        }
                    }
                }

                if ((storageCmd.length() > 0) && (currentDevice!=NULL)) {
                    Log::dbg("Setting Storage Command for "+currentDevice->getDisplayName()+": "+storageCmd);
                    currentDevice->setStorageCommand(storageCmd);
                }

                device = device->NextSiblingElement( "Device" );
            }
        }
    }

    std::ostringstream infoOut;
    infoOut << "Number of devices found: " << gpsDeviceList.size();
    Log::info(infoOut.str());
}

void DeviceManager::setConfiguration(TiXmlDocument * config) {
    // Memory will be freed from configManager
    this->configuration = config;
}


void DeviceManager::cancelFindDevices() {

}

int DeviceManager::finishedFindDevices() {
    return 1;
}

GpsDevice * DeviceManager::getGpsDevice(int number)
{
    if (number < (int)gpsDeviceList.size()) {
        return gpsDeviceList[number];
    }
    return NULL;
}


bool DeviceManager::getXmlBoolAttribute(TiXmlElement *xmlElement, string attrName, bool defaultValue) {
    if (xmlElement == NULL) {
        return defaultValue;
    }
    const char * boolStringValue = xmlElement->Attribute(attrName.c_str());

    if (boolStringValue != NULL) {
        string trueFalseStr = boolStringValue;
        transform(trueFalseStr.begin(), trueFalseStr.end(), trueFalseStr.begin(), ::tolower );
        if ((trueFalseStr == "yes") || (trueFalseStr == "true") || (trueFalseStr == "1")) {
            return true;
        } else if ((trueFalseStr == "no") || (trueFalseStr == "false") || (trueFalseStr == "0")) {
            return false;
        } else {
            return defaultValue;
        }
    } else {
        return defaultValue;
    }
}


GpsDevice * DeviceManager::createGarminDeviceFromPath(string devicepath, TiXmlDocument *doc) {

    bool deleteXmlDoc = false;
    GpsDevice * device = NULL;

    if (doc == NULL) {
        DIR *dp;
        struct dirent *dirp;
        if((dp = opendir(devicepath.c_str())) == NULL) {
            Log::err("Error opening directory: "+devicepath);
            return NULL;
        }

        bool garminDirFound = false;
        string dirname = "";
        while ((dirp = readdir(dp)) != NULL) {
            dirname = string(dirp->d_name);
            if (GpsFunctions::iequals(dirname, "Garmin")) {
                garminDirFound = true;
                break;
            }
        }
        closedir(dp);

        if (garminDirFound) {
            string basePath = devicepath + "/" + dirname;
            string fullPath = basePath +"/GarminDevice.xml";

            // Ignore case search for file GarminDevice.xml
            if((dp = opendir(basePath.c_str())) != NULL) {
            	while ((dirp = readdir(dp)) != NULL) {
					string entry = string(dirp->d_name);
					if (GpsFunctions::iequals(entry, "GarminDevice.xml")) {
						fullPath = basePath + "/" + entry;
						break;
					}
				}
                closedir(dp);
            }

            doc = new TiXmlDocument(fullPath);
            deleteXmlDoc = true;
            if (!doc->LoadFile()) {
                deleteXmlDoc = false;
                delete(doc);
                doc = NULL;
                Log::info("Unable to load xml file "+fullPath);
            }
        } else {
            Log::dbg("Garmin directory not found at "+devicepath);
        }
    }

    if (doc != NULL) {
        TiXmlElement * node = doc->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("Model"); }
        if (node!=NULL) { node = node->FirstChildElement("Description"); }
        if (node!=NULL) {
            // Perfect, seems to be a Garmin Device
            string deviceName = node->GetText();

            GarminFilebasedDevice *fileDev = new GarminFilebasedDevice();
            fileDev->setBaseDirectory(devicepath);
            fileDev->setDeviceDescription(doc);
            fileDev->setDisplayName(deviceName);
            device = fileDev;

            Log::dbg("Found "+deviceName+" at "+devicepath);
        } else {
            Log::err("GarminDevice.xml has unexpected format!");
        }
    }

    if (deleteXmlDoc) {
        delete(doc);
        doc = NULL;
    }
    return device;
}


TiXmlDocument * DeviceManager::createMinimalGarminConfig(string name) {

    TiXmlDocument *doc = new TiXmlDocument();
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
    doc->LinkEndChild( decl );

    /*<Device xmlns="http://www.garmin.com/xmlschemas/GarminDevice/v2"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd">*/

	TiXmlElement * device = new TiXmlElement( "Device" );
    device->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/GarminDevice/v2");
    device->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    device->SetAttribute("xsi:schemaLocation", "http://www.garmin.com/xmlschemas/GarminDevice/v2 http://www.garmin.com/xmlschemas/GarminDevicev2.xsd");
    doc->LinkEndChild( device );

/*  <Model>
    <PartNumber>006-B0000-00</PartNumber>
    <SoftwareVersion>0</SoftwareVersion>
    <Description>An SD Card</Description>
  </Model> */
	TiXmlElement * model = new TiXmlElement( "Model" );
	TiXmlElement * partnumber = new TiXmlElement( "PartNumber" );
	partnumber->LinkEndChild(new TiXmlText("006-B0000-00"));
	TiXmlElement * version = new TiXmlElement( "SoftwareVersion" );
	version->LinkEndChild(new TiXmlText("0"));
	TiXmlElement * descr = new TiXmlElement( "Description" );
	descr->LinkEndChild(new TiXmlText(name));
	model->LinkEndChild(partnumber);
	model->LinkEndChild(version);
	model->LinkEndChild(descr);
    device->LinkEndChild( model );

/*  <Id>4294967295</Id> */
	TiXmlElement * id = new TiXmlElement( "Id" );
	id->LinkEndChild(new TiXmlText("4294967295"));
	device->LinkEndChild(id);
/*  <DisplayName>Removable Disk (F:\\)</DisplayName>*/
	TiXmlElement * dispName = new TiXmlElement( "DisplayName" );
	dispName->LinkEndChild(new TiXmlText(name));
	device->LinkEndChild(dispName);

    TiXmlElement * massStorage = new TiXmlElement( "MassStorageMode" );
    device->LinkEndChild(massStorage);

    return doc;
}

TiXmlDocument * DeviceManager::addTcxProfile(TiXmlDocument * doc, string tcxpath) {
    /*
    <DataType>
      <Name>FitnessHistory</Name>
      <File>
        <Location>
          <Path>Garmin</Path>
          <FileExtension>tcx</FileExtension>
        </Location>
        <TransferDirection>InputOutput</TransferDirection>
      </File>
    </DataType>
    */

    if (doc == NULL) { return NULL; }

    TiXmlElement * massStorage=NULL;
    TiXmlElement * node = doc->FirstChildElement("Device");
    if (node!=NULL) { massStorage = node->FirstChildElement("MassStorageMode"); }
    if (massStorage == NULL) { return doc; }

    TiXmlElement * dataTypes = new TiXmlElement( "DataType" );
    massStorage->LinkEndChild(dataTypes);
    TiXmlElement * name = new TiXmlElement( "Name" );
    name->LinkEndChild(new TiXmlText("FitnessHistory"));
    dataTypes->LinkEndChild(name);

    TiXmlElement * file = new TiXmlElement( "File" );
    dataTypes->LinkEndChild(file);

    TiXmlElement * loc = new TiXmlElement( "Location" );
    file->LinkEndChild(loc);

    TiXmlElement * filePath = new TiXmlElement( "Path" );
    filePath->LinkEndChild(new TiXmlText(tcxpath));
    loc->LinkEndChild(filePath);

    TiXmlElement * fileEx = new TiXmlElement( "FileExtension" );
    fileEx->LinkEndChild(new TiXmlText("tcx"));
    loc->LinkEndChild(fileEx);

    TiXmlElement * transferDir = new TiXmlElement( "TransferDirection" );
    transferDir->LinkEndChild(new TiXmlText("InputOutput"));
    file->LinkEndChild(transferDir);

    return doc;
}

TiXmlDocument * DeviceManager::addGpxProfile(TiXmlDocument * doc, string gpxpath) {
/*
    <DataType>
      <Name>GPSData</Name>
      <File>
        <Specification>
          <Identifier>
          http://www.topografix.com/GPX/1/1</Identifier>
          <Documentation>
          http://www.topografix.com/GPX/1/1/gpx.xsd</Documentation>
        </Specification>
        <Location>
          <Path>Garmin</Path>
          <FileExtension>gpx</FileExtension>
        </Location>
        <TransferDirection>InputToUnit</TransferDirection>
      </File>
    </DataType>
*/
    if (doc == NULL) { return NULL; }

    TiXmlElement * massStorage=NULL;
    TiXmlElement * node = doc->FirstChildElement("Device");
    if (node!=NULL) { massStorage = node->FirstChildElement("MassStorageMode"); }
    if (massStorage == NULL) { return doc; }

    TiXmlElement * dataTypes = new TiXmlElement( "DataType" );
    massStorage->LinkEndChild(dataTypes);
    TiXmlElement * name = new TiXmlElement( "Name" );
   	name->LinkEndChild(new TiXmlText("GPSData"));
    dataTypes->LinkEndChild(name);

    TiXmlElement * file = new TiXmlElement( "File" );
    dataTypes->LinkEndChild(file);
    TiXmlElement * spec = new TiXmlElement( "Specification" );
    file->LinkEndChild(spec);

    TiXmlElement * identifier = new TiXmlElement( "Identifier" );
    identifier->LinkEndChild(new TiXmlText("http://www.topografix.com/GPX/1/1"));
    spec->LinkEndChild(identifier);

    TiXmlElement * docu = new TiXmlElement( "Documentation" );
   	docu->LinkEndChild(new TiXmlText("http://www.topografix.com/GPX/1/1/gpx.xsd"));
    spec->LinkEndChild(docu);

    TiXmlElement * loc = new TiXmlElement( "Location" );
    file->LinkEndChild(loc);

    TiXmlElement * filePath = new TiXmlElement( "Path" );
   	filePath->LinkEndChild(new TiXmlText(gpxpath));
    loc->LinkEndChild(filePath);

    TiXmlElement * fileEx = new TiXmlElement( "FileExtension" );
   	fileEx->LinkEndChild(new TiXmlText("gpx"));
    loc->LinkEndChild(fileEx);

    TiXmlElement * transferDir = new TiXmlElement( "TransferDirection" );
    transferDir->LinkEndChild(new TiXmlText("InputToUnit"));
    file->LinkEndChild(transferDir);

    return doc;
}
