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
#include "edge705Device.h"
#include "oregonDevice.h"
#include "edge305Device.h"
#include "sdCardDevice.h"


DeviceManager::DeviceManager()
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
    while (gpsDeviceList.size() > 0)
    {
        GpsDevice *dev = gpsDeviceList.back();
        gpsDeviceList.pop_back();
        delete(dev);
    }

    FILE *mounts = NULL;
    struct mntent *ent = NULL;
    mounts = setmntent("/etc/mtab", "r");

    Log::dbg("Searching for Edge705/Oregon300/...");
    while ( (ent = getmntent(mounts)) != NULL ) {
        string filesystype = ent->mnt_type;
        if (filesystype.compare("vfat") == 0) {
            string mountPath = ent->mnt_dir;
            DIR *dp;
            struct dirent *dirp;
            if((dp = opendir(mountPath.c_str())) == NULL) {
                Log::err("Error opening directory: "+mountPath);
                break;
            }

            bool garminDirFound = false;
            while ((dirp = readdir(dp)) != NULL) {
                string dir = string(dirp->d_name);
                if (dir.compare("Garmin") == 0) {
                    garminDirFound = true;
                    break;
                }
            }
            closedir(dp);

            if (garminDirFound) {
                string fullPath = mountPath + "/Garmin/GarminDevice.xml";
                TiXmlDocument doc(fullPath);
                if (doc.LoadFile()) {
                    // Perfect, seems to be a Garmin Device
                    TiXmlElement * node = doc.FirstChildElement("Device");
                    if (node!=NULL) { node = node->FirstChildElement("Model"); }
                    if (node!=NULL) { node = node->FirstChildElement("Description"); }
                    if (node!=NULL) {
                        string deviceName = node->GetText();

                        GpsDevice * device = NULL;
                        string::size_type position = deviceName.find( "Oregon", 0 );
                        if ((device == NULL) && (position != string::npos)) { // Found Oregon in deviceName
                            OregonDevice * oregon = new OregonDevice();
                            oregon->setBaseDirectory(mountPath);
                            oregon->setDeviceDescription(&doc);
                            oregon->setDisplayName(deviceName);
                            device = oregon;
                        }

                        position = deviceName.find( "EDGE", 0 );
                        if ((device == NULL) && (position != string::npos)) {
                            Edge705Device * edge = new Edge705Device();
                            edge->setBaseDirectory(mountPath);
                            edge->setDeviceDescription(&doc);
                            edge->setDisplayName(deviceName);
                            device = edge;
                        }

                        if (device != NULL) {
                            Log::dbg("Found "+deviceName+" at "+mountPath);
                            gpsDeviceList.push_back(device);
                        } else {
                            Log::err("Unknown device "+deviceName+" at "+mountPath);
                        }

                    } else {
                        Log::err("GarminDevice.xml has unexpected format!");
                    }
                } else {
                    Log::err("Not yet implemented new SD-Card"); //@TODO
                }
            } else {
                Log::dbg("Garmin directory not found at "+mountPath);
            }
        }
    }

    bool searchGarmin = true;
    if (this->configuration != NULL) {
        TiXmlElement * pRoot = this->configuration->FirstChildElement( "GarminPlugin" );
        TiXmlElement * settings = NULL;
        TiXmlElement * ftools = NULL;

        if (pRoot != NULL) { settings = pRoot->FirstChildElement("Settings"); }
        if (settings != NULL) { ftools = settings->FirstChildElement("ForerunnerTools"); } else { Log::dbg("settings is null!"); }
        if (ftools != NULL) {
            const char * ftoolsEnabled = ftools->Attribute("enabled");

            if (ftoolsEnabled != NULL) {
                string enabledStr = ftoolsEnabled;
                if ((enabledStr == "yes") || (enabledStr == "YES") || (enabledStr == "true") || (enabledStr == "TRUE") || (enabledStr == "1")) {
                    searchGarmin = true;
                } else {
                    searchGarmin = false;
                }
            } else {
				Log::dbg("ftoolsEnabled is null!");
			}
        } else {
			Log::dbg("ftools is null!");
		}
    }

    string deviceName;
    if (searchGarmin) {
        // Search for garmin 305
        deviceName = Edge305Device::getAttachedDeviceName();
        if (deviceName.length() > 0) {  // Found a device
            Log::dbg("Found device via garmintools: "+deviceName);
            Edge305Device * device = new Edge305Device(deviceName);
            gpsDeviceList.push_back(device);
        }
    } else {
        Log::dbg("Search via garmintools is disabled!");
    }

    // Now create virtual SD Card devices from configuration

    if (this->configuration != NULL) {
        TiXmlElement * pRoot = this->configuration->FirstChildElement( "GarminPlugin" );
        if (pRoot != NULL) {
            TiXmlElement * devices = pRoot->FirstChildElement("Devices");
            TiXmlElement * device = devices->FirstChildElement("Device");
            while ( device != NULL )
            {
                string storagePath = "";
                string storageCmd = "";
                string fitnessPath = "";
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

                GpsDevice * currentDevice = NULL;
                TiXmlElement * name = device->FirstChildElement("Name");
                if (name!=NULL) {
                    if (name->GetText() != NULL) {
                        for(unsigned int i=0; i < gpsDeviceList.size(); i++)
                        {
                            // Device exists, and is configured in configuration
                            if (gpsDeviceList[i]->getDisplayName().compare(name->GetText()) == 0) {
                                currentDevice = gpsDeviceList[i];
                            }
                        }
                        if (currentDevice == NULL) { // no device found
                            deviceName = name->GetText();
                            Log::info("Creating new SD Card Device from configuration: "+deviceName);
                            SDCardDevice * sdcard = new SDCardDevice();
                            sdcard->setDisplayName(name->GetText());
                            sdcard->setBaseDirectory("");
                            sdcard->setDeviceDescription(SDCardDevice::getDefaultConfiguration(deviceName, storagePath, fitnessPath));
                            if (sdcard->isDeviceAvailable()) {
                                currentDevice = sdcard;
                                gpsDeviceList.push_back(currentDevice);
                            } else {
                                delete(sdcard);
                                currentDevice = NULL;
                                Log::dbg("Device "+deviceName+" is not available.");
                            }
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
