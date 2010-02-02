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


#ifndef GPSDEVICE_H_INCLUDED
#define GPSDEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"

#include <pthread.h>


using namespace std;

class MessageBox;

class GpsDevice
{
public:
  /**
   * Creates a new GpsDevice - which simulates an SD Card
   */
    GpsDevice();

  /**
   * Destructor - currently useless
   */
    ~GpsDevice();

  /**
   * Returns the device description in XML format to be passed to the Garmin Javascript Libs
   * @return xml string with device description
   */
    string getDeviceDescription();

  /**
   * Starts a thread that writes the passed xml string to the given filename
   * @param filename - filename on disk
   * @param xml - content for the file on disk
   * @return int returns 1 if successful otherwise 0
   */
    int startWriteToGps(string filename, string xml);

  /**
   * Returns the status if writing to device is finished
   * @return int     0 = idle    1 = working    2 = waiting    3 = finished
   */
    int finishWriteToGps();

  /**
   * Cancels the write thread
   */
    void cancelWriteToGps();

  /**
   * Returns if the transfer to the device was successful
   * @return int     0 = failed    1 = success
   */
    int getTransferSucceeded();

  /**
   * Returns the name of the device
   * @return string with name of device
   */
    string getDisplayName();

  /**
   * Set the name of the device
   * @param name set the name of the devices
   */
    void setDisplayName(string name);

  /**
   * Sets the path on disk where to store the files
   * @param directory full path to a directory on disk
   */
    void setStorageDirectory(string directory);

  /**
   * Sets a command that gets executed after the file was stored on disk
   * @param cmd command to execute after the file was stored on disk
   */
    void setStorageCommand(string cmd);

  /**
   * Returns a message for the user. Should be called if finishWriteToGps returns 2 (waiting)
   * The function that fetches the message must delete the message!
   * @return MessageBox for the user to display
   */
    MessageBox * getMessage();

  /**
   * A message can be answered by the user. This function must be called if the message was answered
   * @param answer contains the button the user pressed
   */
    void userAnswered(const int answer);

  /**
   * Returns true if the device is available - current implementation checks if directory exists
   * @return true if devices should be shown to user
   */
    bool isDeviceAvailable();

private:

  /**
   * Thread that gets called when a file should be written to disk
   * @param instance of the GpsDevice that should be written to disk
   */
    static void * writeToDevice(void * pthis);

  /**
   * Name of the GpsDevice which gets displayed to the user
   */
    string displayName;

  /**
   * Directory where this device stores its data to
   */
    string storageDirectory;

  /**
   * Command that should be executed after file was written. Leave empty if no command should be executed
   */
    string storageCmd;

  /**
   * File content to write to disk
   */
    string xmlToWrite;

  /**
   * File name for file on disk
   */
    string filenameToWrite;

  /**
   * Stores the status of the thread
   * 0 = idle    1 = working    2 = waiting    3 = finished
   * This value is used by finishWriteToGps to signal the current status
   * @see finishWriteToGps
   */
    int threadStatus;

  /**
   * Stores the thread id
   */
    pthread_t threadId;

  /**
   * The thread can create a message for the user - this message is stored here
   */
    MessageBox *  waitingMessage;

  /**
   * If an overwrite yes/no message is posted to the user, this variable is set to 1 if the user wishes to overwrite the file. Otherwise set to 0
   */
    int overwriteFile;

  /**
   * The thread stores the success state in this variable
   */
    bool transferSuccessful;
};

#endif // GPSDEVICE_H_INCLUDED
