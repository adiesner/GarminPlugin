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
#include <vector>

#include "gpsDevice.h"

#include "tinyxml.h"

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
   * Triggers a search for new devices.
   */
    void startFindDevices();

  /**
   * Cancels a search for new devices.
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
   * Stores all configured devices
   */
    vector<GpsDevice *> gpsDeviceList;

    TiXmlDocument * configuration;

    /**
     * Searches for an attribute with the name attrName in xmlElement
     * If attribute is found, it is checked for true/false values
     * If not exists or unknown value, defaultValue will be returned
     */
    bool getXmlBoolAttribute(TiXmlElement * xmlElement, string attrName, bool defaultValue);

    /**
     * Creates a device if configuration is given or Garmin/GarminDevice.xml exists
     * @param path - Path on disk
     * @param doc - if null a Garmin/GarminDevice.xml must exist on the given path.
     * @return Returns the created device
     */
    GpsDevice * createGarminDeviceFromPath(string path, TiXmlDocument *doc);

    /**
     * Creates a minimalistic garmin device configuration
     * @param name - device name
     * @return Xml document (in the format of GarminDevice.xml)
     */
    TiXmlDocument * createMinimalGarminConfig(string name);

    /**
     * Adds a tcx profile for read and write to a configuration
     * @param doc - must contain an already valid GarminDevice.xml format
     * @param tcxpath - relative path to tcx data
     * @return Xml document (in the format of GarminDevice.xml)
     */
    TiXmlDocument * addTcxProfile(TiXmlDocument * doc, string tcxpath);

    /**
     * Adds a gpx profile for read and write to a configuration
     * @param doc - must contain an already valid GarminDevice.xml format
     * @param gpxpath - relative path to gpx data
     * @return Xml document (in the format of GarminDevice.xml)
     */
    TiXmlDocument * addGpxProfile(TiXmlDocument * doc, string gpxpath);

    /**
     * Stores the thread id
     */
      pthread_t threadId;

      /**
       * Stores the status of the thread
       * 0 = idle    1 = working    2 = waiting    3 = finished
       */
     int findDeviceState;

     /**
      * Thread gets called to search for devices
      */
     static void * findDeviceThread(void * pthis);

     /**
      * Thread that actually searches for a device
      */
     void findDevices();

};

#endif // DEVICEMANAGER_H_INCLUDED
