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
};

#endif // DEVICEMANAGER_H_INCLUDED
