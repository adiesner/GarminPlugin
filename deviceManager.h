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


#ifndef DEVICEMANAGER_H_INCLUDED
#define DEVICEMANAGER_H_INCLUDED

#include <iostream>
#include <string>

#define TIXML_USE_TICPP

#include "ticpp.h"

#include "gpsDevice.h"

using namespace std;

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();

  /**
   * Returns an xml string that describes all attached gps devices
   * Devices that return false on isDeviceAvailable() will not be listed
   * @return string with xml description
   */
    const std::string getDevicesXML();

  /**
   * Triggers a search for new devices. Currently it has no implementation
   */
    void startFindDevices();

  /**
   * Cancels a search for new devices. Currently it has no implementation
   */
    void cancelFindDevices();

  /**
   * Checks if the search for devices has finished
   * @return returns 1 if search was finished
   */
    int finishedFindDevices();

  /**
   * Returns an instance of an attached GPS device
   * @param number that identifies the device (listed in getDevicesXML()
   * @return returns the device or null
   */
    GpsDevice * getGpsDevice(int number);

  /**
   * Sets the configuration that has been loaded from disk and initializes the attached devices
   * @param XML document with configuration
   */
    void setConfiguration(TiXmlDocument * );

private:
  /**
   * Reads the configuration settings xml and creates the device list
   * @param XML document with configuration
   */
    void createDeviceList(TiXmlDocument * );

  /**
   * Stores all configured devices
   */
    vector<GpsDevice *> gpsDeviceList;

  /**
   * Stores all active devices
   */
    vector<GpsDevice *> gpsActiveDeviceList;
};

#endif // DEVICEMANAGER_H_INCLUDED
