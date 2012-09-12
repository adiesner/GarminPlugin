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
#ifndef GARMINFILEBASEDDEVICE_H_INCLUDED
#define GARMINFILEBASEDDEVICE_H_INCLUDED

#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include "messageBox.h"
#include "gpsDevice.h"
#include "log.h"

#include "fit/fitReader.hpp"
#include "fit/fitMsg.hpp"
#include "fit/fitMsg_File_ID.hpp"
#include "fit/fitFileException.hpp"

using namespace std;

class MessageBox;



class GarminFilebasedDevice : public GpsDevice, FitMsg_Listener
{
public:
    GarminFilebasedDevice();

    virtual ~GarminFilebasedDevice();

    void setDeviceDescription(TiXmlDocument * device);

  /**
   * Returns the device description in XML format to be passed to the Garmin Javascript Libs
   * @return xml string with device description
   */
    string getDeviceDescription() const;

  /**
   * Starts a thread that writes the passed xml string to the given filename
   * @param filename - filename on disk
   * @param xml - content for the file on disk
   * @return int returns 1 if successful otherwise 0
   */
    int startWriteToGps(string filename, string xml);

  /**
   * Sets the path on disk where the device is mounted
   * @param directory full path to a directory on disk
   */
    virtual void setBaseDirectory(string directory);

  /**
   * Sets a command that gets executed after the file was stored on disk
   * @param cmd command to execute after the file was stored on disk
   */
    virtual void setStorageCommand(string cmd);

  /**
   * Returns true if the device is available - current implementation checks if directory exists
   * @return true if devices should be shown to user
   */
    virtual bool isDeviceAvailable();


  /**
   * Starts reading the FIT data directory
   */
    virtual int startReadFITDirectory();

  /**
   * Checks if the read of the FIT data directory finished
   * @return 0 = idle 1 = working 2 = waiting 3 = finished
   */
    virtual int finishReadFITDirectory();

  /**
   * Cancels reading the FIT data directory
   */
    virtual void cancelReadFITDirectory();

  /**
   * Starts reading the fitness data without points
   * @param dataTypeName - which type of data should be read from the device
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startReadFitnessDirectory(string dataTypeName);

  /**
   * Checks if the read of the fitness directory finished
   * @return 0 = idle 1 = working 2 = waiting 3 = finished
   */
    virtual int finishReadFitnessDirectory();

    /**
     * Cancels the read of the fitness data
     */
    virtual void cancelReadFitnessData();

    virtual int startReadFitnessDetail(string id);

    virtual int finishReadFitnessDetail();

    virtual void cancelReadFitnessDetail();

    virtual string getGpxData();

    /**
     * Start the reading of a file in the GPX format (like current.gpx on Oregon)
     */
    virtual int startReadFromGps();
    /**
     * This is used to indicate the status of the read process.
     * @return 0 = idle 1 = working 2 = waiting 3 = finished
     */
    virtual int finishReadFromGps();
    /**
     * Cancels the current read from the device.
     */
    virtual void cancelReadFromGps();

    /**
     * Returns a file from the device
     */
    virtual string getBinaryFile(string relativeFilePath);

    /**
     * Starts a binary file write to the device
     * @return number of downloads found in data
     */
    virtual int startDownloadData(string gpsDataString);

    /**
     * Retrieves the next download url
     * @return url to download
     */
    virtual string getNextDownloadDataUrl();

    /**
     * Writes some bytes into the file opened by startDownloadData
     * @return Bytes written to file
     */
    virtual int writeDownloadData(char *, int length);

    /**
     * This is used to indicate the status of the write download data process.
     * @return 0 = idle 1 = working 2 = waiting 3 = finished
     */
    virtual int finishDownloadData();

    /**
     * Saves/Closes the current file
     */
    virtual void saveDownloadData();

    /**
     * Cancels the current file download
     */
    virtual void cancelDownloadData();

  /**
   * Starts a thread that writes the passed xml string to the given filename
   * @param filename - filename on disk
   * @param data - the filename to write to on the device.
   * @param dataTypeName - a Fitness DataType from the GarminDevice.xml retrieved with DeviceDescription
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startWriteFitnessData(string filename, string data, string dataTypeName);

    /**
     * This is used to indicate the status of the write fitness data process.
     * @return 0 = idle 1 = working 2 = waiting 3 = finished
     */
    virtual int finishWriteFitnessData();

    /**
     * Cancels the current write of fitness data
     */
    virtual void cancelWriteFitnessData();

    /**
     * Returns the bytes available in the given path on the device
     * @return bytes available (-1 for non-mass storage mode devices.)
     */
    virtual int bytesAvailable(string path);

  /**
   * Starts a thread that tries to read the fitness data from a garmin device
   * @param dataTypeName - which type of data should be read from the device
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startReadFitnessData(string dataTypeName);

  /**
   * Returns the status of reading fitness data from the device
   * @return int     0 = idle    1 = working    2 = waiting    3 = finished
   */
    virtual int finishReadFitnessData();

  /**
   * Gets the fitness data xml
   * @return xml containing fitness data read from garmin device
   */
    virtual string getFitnessData();

    /**
     * Receives all decoded messages stored in the fit file
     */
    virtual void fitMsgReceived(FitMsg *msg);


   /**
    * Starts an asynchronous file listing operation for a Mass Storage mode device.
    * Only files that are output from the device are listed. </br>
    * The result can be retrieved with getDirectoryXml().
    * Minimum plugin version 2.8.1.0 <br/>
    *
    * @param {String} dataTypeName a DataType from GarminDevice.xml retrieved with DeviceDescription
    * @param {String} fileTypeName a Specification Identifier for a File in dataTypeName from GarminDevice.xml
    * @param {Boolean} computeMD5 If true, the plug-in will generate an MD5 checksum for each readable file.
    * @return int returns 1 if successful otherwise 0
    */
    virtual int startReadableFileListing(string dataTypeName, string fileTypeName, bool computeMd5);

    /**
     * Returns the status of the asynchronous file listing operation for the mass storage mode device
     * @return 0 = idle 1 = working 2 = waiting 3 = finished
     */
    virtual int finishReadableFileListing();

    /**
     * Cancels the asynchronous file listing operation for the mass storage mode device
     */
    virtual void cancelReadableFileListing();

    /**
     * Returns the status of the asynchronous file listing operation
     * @return string with directory listing
     */
    virtual string getDirectoryListingXml();


   /**
    * Starts an asynchronous file listing operation for a Mass Storage mode device.
    * This function lists all files that are available on the device. </br>
    * The result can be retrieved with getDirectoryListingXml().
    * Minimum plugin version 2.8.1.0 <br/>
    *
    * @param {String} relativePath specifies the relative path on the device
    * @param {Boolean} computeMD5 If true, the plug-in will generate an MD5 checksum for each readable file.
    * @return int returns 1 if successful otherwise 0
    */
    virtual int startDirectoryListing(string relativePath, bool computeMd5);

    /**
     * Returns the status of the asynchronous file listing operation for the mass storage mode device
     * @return 0 = idle 1 = working 2 = waiting 3 = finished
     */
    virtual int finishDirectoryListing();

    /**
     * Cancels the asynchronous file listing operation for the mass storage mode device
     */
    virtual void cancelDirectoryListing();

protected:

    /**
     * Reads tcx files in the fit directory. (called by thread)
     */
    void readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId);

    /**
     * Reads the fitness profile from the device (called by thread)
     */
    void readFitnessUserProfile();

    /**
     * Reads the fitness courses from the device (called by thread)
     */
    void readFitnessCourses(bool readTrackData);

    /**
     * Reads the fitness workout from the device (called by thread)
     */
    void readFitnessWorkouts();

    /**
     * Reads the a directory listing from the device (called by thread)
     */
    void readDirectoryListing();


    /**
     * Reads file structure of fit directories for ReadFITDirectory
     */
    void readFITDirectoryFromDevice();

    /**
     * Reads the file listening from the device
     */
    void readFileListingFromDevice();

  /**
   * Returns a message for the user. Should be called if finishWriteToGps returns 2 (waiting)
   * The function that fetches the message must delete the message!
   * @return MessageBox for the user to display
   */
    virtual MessageBox * getMessage();

  /**
   * A message can be answered by the user. This function must be called if the message was answered
   * @param answer contains the button the user pressed
   */
    virtual void userAnswered(const int answer);

  /**
   * Cancels the write thread
   */
    virtual void cancelWriteToGps();

  /**
   * Returns if the transfer to the device was successful
   * @return int     0 = failed    1 = success
   */
    virtual int getTransferSucceeded();


    TiXmlDocument * deviceDescription;


  /**
   * Returns the status if writing to device is finished
   * @return int     0 = idle    1 = working    2 = waiting    3 = finished
   */
    virtual int finishWriteToGps();


    /**
     * Adds the author element to a given node
     */
    void addAuthorXmlElement(TiXmlElement * parentNode);

    virtual void doWork();

    void writeGpxFile();

  /**
   * Directory where this device is mounted in the file system
   */
    string baseDirectory;

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

  /**
   * Searches the configuration for the update directory and adds it.
   */
  void setUpdatePathsFromConfiguration();

 /**
  * Searches for FIT, TCX and GPX directories in the configuration
  */
  virtual void setPathsFromConfiguration();

 /**
  * Check upper/lower case of paths, if they match the file system and correct if needed
  */
  void checkPathsFromConfiguration();

/**
 * Parses all attributes of a TiXmlElement and adds them to another TiXmlElement if it is missing
 */
  void addMissingAttributes(TiXmlElement * in, TiXmlElement * out);

  /**
   * File where this device stores its fitness/gpx data
   */
  string fitnessFile;

  /**
   * Contains the device id
   */
   string deviceId;

    /**
     * Is used to store the data given by startDownloadData()
     */
   typedef struct _DeviceDownloadData {
        string url;          /**< URL to download */
        string destination;  /**< Local path to store url */
        string destinationtmp;/**< Local path to store url while downloading */
        string regionId;     /**< RegionId - unknown purpose */
   } DeviceDownloadData;

   /**
    * Renames temporary file to real file name
    * TimeZone data file needs post processing after download.
    */
   void postProcessDownloadData(DeviceDownloadData fileElement);

    /**
     * stores a list of files to download and store on the device (provided by startDownloadData())
     */
   list <DeviceDownloadData> deviceDownloadList;

    /**
     * File hande for downloadData
     */
   ofstream downloadDataOutputStream;

    /**
     * Counts errors during file download to the device
     */
   int downloadDataErrorCount;

    enum DirDataType
    {
      FITDIR,
      TCXDIR,
      GPXDIR,
      CRSDIR,
      UNKNOWN
    };

    typedef struct _MassStorageDirectoryType {
        DirDataType dirType;
        string path;
        string name;
        string extension;
        string basename;
        bool writeable;
        bool readable;
    } MassStorageDirectoryType;

    /**
     * Contains a list of directories available on device (read from garmindevice.xml)
     */
   list <MassStorageDirectoryType> deviceDirectories;


  /**
   * Stores the fitnessData which was read from the device
   */
    string fitnessDataTcdXml;


    /**
     * Stores the id of the track that should be read
     */
    string readFitnessDetailId;

    /**
     * Stores the current xml element where the Fit Message Listener (fitMsgReceived())
     * should write the file information data to
     * Needed for the Fit-File Reader
     */
    TiXmlElement * fitFileElement;

   /**
    * Variable containing the dataTypeName to be read by the asynchronous task READABLEFILELISTING
    */
    string readableFileListingDataTypeName;

   /**
    * Variable containing the FileTypeName to be read by the asynchronous task READABLEFILELISTING
    */
    string readableFileListingFileTypeName;

   /**
    * Variable containing boolean computeMD5 to be read by the asynchronous task READABLEFILELISTING
    */
    bool readableFileListingComputeMD5;

  /**
   * Stores the file listing Data which was read from the device
   */
    string directoryListingXml;

    /**
     * Read a file and calculate the md5 checksum
     */
    string getMd5FromFile(string filename);
};

#endif // GARMINFILEBASEDDEVICE_H_INCLUDED
