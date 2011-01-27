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
#ifndef SDCARDDEVICE_H_INCLUDED
#define SDCARDDEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"
#include "garminFilebasedDevice.h"

using namespace std;

class MessageBox;

class SDCardDevice : public GarminFilebasedDevice
{
public:
    SDCardDevice();

    virtual ~SDCardDevice();


  /**
   * Starts a thread that tries to read the fitness data from a garmin device
   * @return int returns 1 if successful otherwise 0
   */
    int startReadFitnessData();

  /**
   * Returns the status of reading fitness data from the device
   * @return int     0 = idle    1 = working    2 = waiting    3 = finished
   */
    int finishReadFitnessData();

  /**
   * Gets the fitness data xml
   * @return xml containing fitness data read from garmin device
   */
    string getFitnessData();

    static TiXmlDocument * getDefaultConfiguration(string devicename, string gpxpath, string tcxpath);

    virtual bool isDeviceAvailable();

protected:
    virtual void doWork();
    void * readFitnessDataFromDevice();

  /**
   * Directory where this device stores its fitness data
   */
    string fitnessDirectory;

    string fitnessFileExtension;

    virtual void setPathesFromConfiguration();

private:


  /**
   * Stores the fitnessData which was read from the device
   */
    string fitnessDataTcdXml;

};



#endif // SDCARDDEVICE_H_INCLUDED
