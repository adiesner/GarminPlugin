#include "garminFilebasedDevice.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>


GarminFilebasedDevice::GarminFilebasedDevice() {
    this->deviceDescription = NULL;
}

GarminFilebasedDevice::~GarminFilebasedDevice() {
    if (this->deviceDescription != NULL) {
        delete(deviceDescription);
        this->deviceDescription = NULL;
    }
}


string GarminFilebasedDevice::getDeviceDescription() const
{
    if (this->deviceDescription == NULL) { return ""; }

    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	this->deviceDescription->Accept( &printer );
    string str = printer.Str();

    if (Log::enabledDbg()) Log::dbg("GarminFilebasedDevice::getDeviceDescription() Done: "+this->displayName );
    return str;
}

void GarminFilebasedDevice::setDeviceDescription(TiXmlDocument * device) {
    this->deviceDescription = new TiXmlDocument(*device);
    if (this->deviceDescription != NULL) {
        setPathesFromConfiguration();
    }
}


int GarminFilebasedDevice::startWriteToGps(const string filename, const string xml)
{
    string::size_type loc = filename.find( "..", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: Filenames with .. are not allowed!");
        return 0;
    }
    loc = filename.find( "/", 0 );
    if( loc != string::npos ) {
        Log::err("SECURITY WARNING: Filenames with / are not allowed!");
        return 0;
    }

    // There shouldn't be a thread running... but who knows...
    lockVariables();
    this->xmlToWrite = xml;
    this->filenameToWrite = gpxDirectory + "/" + filename;
    this->overwriteFile = 2; // not yet asked
    this->workType = WRITEGPX;
    unlockVariables();

    if (Log::enabledDbg()) Log::dbg("Saving to file: "+this->filenameToWrite);

    if (startThread()) {
        return 1;
    }

    return 0;
}



void GarminFilebasedDevice::writeGpxFile() {

    lockVariables();
    string xml = this->xmlToWrite;
    string filename = this->filenameToWrite;
    string systemCmd = this->storageCmd;
    this->threadState = 1; // Working
    unlockVariables();

    struct stat stFileInfo;
    int intStat;
    // Attempt to get the file attributes
    intStat = stat(filename.c_str(),&stFileInfo);
    if(intStat == 0) {
        // File exists - we need to ask the user to overwrite
        lockVariables();
        this->waitingMessage = new MessageBox(Question, "File "+filename+" exists. Overwrite?", BUTTON_YES | BUTTON_NO , BUTTON_NO, this);
        this->threadState = 2;
        unlockVariables();

        waitThread(); // Sleep until thread gets signal (user answered)

        bool doOverwrite = true;

        lockVariables();
            if (this->overwriteFile != 1) {
                this->threadState = 3;
                this->transferSuccessful = false;
                doOverwrite = false;
            }
        unlockVariables();

        if (!doOverwrite) {
            Log::dbg("Thread aborted");
            return;
        }
    }

    ofstream file;
    file.open (filename.c_str());
    file << xml;
    file.close();

    // Execute extern command if wanted
    if (systemCmd.length() > 0) {
        string placeholder = "%1";
        int pos=systemCmd.find( placeholder );
        if (pos >=0) {
            systemCmd.replace(systemCmd.find(placeholder),placeholder.length(),filename);
        }

        pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        Log::dbg("Thread before executing user command: "+systemCmd);
        int ret = system(systemCmd.c_str());

        pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, NULL);

        if (ret != 0) {
            lockVariables();
                this->waitingMessage = new MessageBox(Question, "Error executing command: "+systemCmd, BUTTON_OK , BUTTON_OK, NULL);
                this->threadState = 2;
            unlockVariables();

            sleep(1); // give application time to fetch messagebox
            lockVariables();
            this->threadState = 3;
            unlockVariables();

            Log::err("Executing user command failed: "+systemCmd);
            return;
        }
    }

    lockVariables();
    this->threadState = 3; // Finished
    this->transferSuccessful = true; // Successfull;
    unlockVariables();

}

void GarminFilebasedDevice::doWork() {
    if (this->workType == WRITEGPX) {
        this->writeGpxFile();
    } else {
        Log::err("Work Type not implemented!");
    }
}

MessageBox * GarminFilebasedDevice::getMessage() {
    MessageBox * msg = this->waitingMessage;
    this->waitingMessage = NULL;
    return msg;
}

void GarminFilebasedDevice::userAnswered(const int answer) {
    if (answer == 1) {
        if (Log::enabledDbg()) Log::dbg("User wants file overwritten");
        lockVariables();
        this->overwriteFile = 1;
        unlockVariables();
    } else {
        if (Log::enabledDbg()) Log::dbg("User wants file to be untouched");
        lockVariables();
        this->overwriteFile = 0;
        unlockVariables();
    }
    lockVariables();
    this->threadState = 1; /* set back to working */
    unlockVariables();

    signalThread();
}

int GarminFilebasedDevice::finishWriteToGps()
{
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    lockVariables();
    int status = this->threadState;
    unlockVariables();

    return status;
}


void GarminFilebasedDevice::cancelWriteToGps() {
    cancelThread();
}

int GarminFilebasedDevice::getTransferSucceeded() {
    if (this->transferSuccessful) {
        return 1;
    }
    return 0;
}

void GarminFilebasedDevice::setBaseDirectory(string directory) {
    this->baseDirectory = directory;

    if (this->deviceDescription != NULL) {
        setPathesFromConfiguration();
    }
}

void GarminFilebasedDevice::setStorageCommand(string cmd) {
    this->storageCmd = cmd;
}

bool GarminFilebasedDevice::isDeviceAvailable() {
    struct stat st;
    if(stat(this->baseDirectory.c_str(),&st) == 0) {
        // directory exists
        return true;
    }
    Log::dbg("Device is not available: "+this->displayName);
    return false;
}

void GarminFilebasedDevice::setPathesFromConfiguration() {
    if (!writeableDirectories.empty()) { writeableDirectories.clear(); }
    this->gpxDirectory = this->baseDirectory; // Fallback
    this->fitnessFile = this->baseDirectory+"/Garmin/gpx/current/Current.gpx"; // Fallback

    if (this->deviceDescription != NULL) {
        TiXmlElement * node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("Id"); }
        if (node!=NULL) { deviceId = node->GetText(); }

        node = this->deviceDescription->FirstChildElement("Device");
        if (node!=NULL) { node = node->FirstChildElement("MassStorageMode"); }
        if (node!=NULL) { node = node->FirstChildElement("DataType"); }
        while ( node != NULL) {
            TiXmlElement * node2 = node->FirstChildElement("Name");
            if (node2 != NULL) {
                string nameText = node2->GetText();
                node2 = node->FirstChildElement("File");
                while (node2 != NULL) {
                    TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                    string transDir = transferDirection->GetText();

                    // Get directory to write GPX
                    if ((transDir.compare("InputToUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
                        TiXmlElement * loc = NULL;
                        string path = "";
                        string ext = "";
                        if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                        if (loc!=NULL)   { node2 = loc->FirstChildElement("Path"); }

                        if (node2!=NULL) {
                            path = node2->GetText();
                        }
                        if (loc!=NULL)   { node2 = loc->FirstChildElement("FileExtension"); }
                        if (node2!=NULL) {
                            ext = node2->GetText();
                        }
                        if ((nameText.compare("GPSData") == 0) && (path.length() > 0)) {
                            Log::dbg("Found path: "+ string(node2->GetText()) + " for GPSData");
                            this->gpxDirectory = this->baseDirectory + "/" + path;
                            this->gpxFileExtension  = ext;
                        }
                        // add to list of writeable directories
                        if (Log::enabledDbg()) { Log::dbg("Adding directory to writeable directory list: "+path); }
                        writeableDirectories.push_back(path);
                    }

                    // Get location of current.gpx file
                    if ((nameText.compare("GPSData") == 0) && ((transDir.compare("OutputFromUnit") == 0) || (transDir.compare("InputOutput") == 0))) {
                        TiXmlElement * loc = NULL;
                        TiXmlElement * path = NULL;
                        TiXmlElement * basename = NULL;
                        TiXmlElement * ext = NULL;
                        if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                        if (loc!=NULL)   { path = loc->FirstChildElement("Path"); }
                        if (loc!=NULL)   { basename = loc->FirstChildElement("BaseName"); }
                        if (loc!=NULL)   { ext = loc->FirstChildElement("FileExtension"); }

                        if ((path != NULL) && (basename != NULL) && (ext != NULL)) {
                            this->fitnessFile = this->baseDirectory + "/" + path->GetText() +"/"+basename->GetText()+"."+ext->GetText();
                            Log::dbg("Fitness file is: "+this->fitnessFile);
                        }
                    }

                    node2 = node2->NextSiblingElement("File");
                }
            }
            node = node->NextSiblingElement( "DataType" );
        }
    }
}

int GarminFilebasedDevice::startReadFITDirectory() {
    Log::err("startReadFITDirectory is not implemented for this device "+this->displayName);
    return 1;
}

int GarminFilebasedDevice::finishReadFITDirectory() {
    Log::err("finishReadFITDirectory is not implemented for this device "+this->displayName);
    return 3; // transfer finished
}

void GarminFilebasedDevice::cancelReadFITDirectory() {
    Log::err("cancelReadFITDirectory is not implemented for this device "+this->displayName);
}

string GarminFilebasedDevice::getFITData() {
    Log::err("getFITData is not implemented for this device "+this->displayName);

    // Return empty listing
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\
            <DirectoryListing xmlns=\"http://www.garmin.com/xmlschemas/DirectoryListing/v1\" RequestedPath=\"\" UnitId=\"3815526107\" VolumePrefix=\"\">\
            </DirectoryListing>";
}

int GarminFilebasedDevice::startReadFitnessDirectory() {
    Log::err("Reading fitness directory is not implemented for this device "+this->displayName);
    return 0;
}

int GarminFilebasedDevice::finishReadFitnessDirectory() {
    return 3; // Finished
}

void GarminFilebasedDevice::cancelReadFitnessData() {
    cancelThread();
}

int GarminFilebasedDevice::startReadFitnessDetail(string id) {
    Log::err("Please implement me GarminFilebasedDevice::startReadFitnessDetail");
    return 0;
}

int GarminFilebasedDevice::finishReadFitnessDetail() {
    Log::err("Please implement me GarminFilebasedDevice::finishReadFitnessDetail");
    return 0;
}

void GarminFilebasedDevice::cancelReadFitnessDetail() {
    Log::err("Please implement me GarminFilebasedDevice::cancelReadFitnessDetail");
}

int GarminFilebasedDevice::startReadFromGps() {
    struct stat stFileInfo;
    int intStat;

    // Attempt to get the file attributes
    intStat = stat(this->fitnessFile.c_str(),&stFileInfo);
    if(intStat != 0) {
        Log::err("The file "+this->fitnessFile+" could not be found. Unable to read Gpx data.");
        this->transferSuccessful = 0;
        return 0;
    }

    this->transferSuccessful = 1;
    if (Log::enabledDbg()) Log::dbg("No thread necessary to read from device, gpx file exists");

    return 1;
}

int GarminFilebasedDevice::finishReadFromGps() {
/*
    0 = idle
    1 = working
    2 = waiting
    3 = finished
*/

    return 3; // No processing of data necessary, therefore finish instantly
}


string GarminFilebasedDevice::getGpxData() {
    stringstream filecontent;
    std::ifstream file;
    file.open (this->fitnessFile.c_str());
    if (file) {
        string line;
        while (getline(file, line)) {
            filecontent << line << "\n";
        }
        file.close();
    } else {
        Log::err("GetGpxData(): Unable to open file "+this->fitnessFile);
    }

    return filecontent.str();
}

void GarminFilebasedDevice::cancelReadFromGps() {
    this->transferSuccessful = 0;
    Log::dbg("Canceling ReadFromGps...");
    // Nothing much to do here as no thread was started
}

string GarminFilebasedDevice::getBinaryFile(string relativeFilePath) {
    Log::err("getBinaryFile is not yet implemented for "+this->displayName);
    return "";
}

int GarminFilebasedDevice::startDownloadData(string gpsDataString) {
    Log::err("startDownloadData was called for "+this->displayName);

    if (deviceDownloadList.size() > 0) {
        Log::info("There are still files to download in the queue. Erasing these files...");
    }
    deviceDownloadList.clear();

    TiXmlDocument doc;
    doc.Parse(gpsDataString.c_str());

    // It is possible that more than one file can be downloaded
    TiXmlElement * devDown = doc.FirstChildElement("DeviceDownload");
    if (devDown != NULL) {
        TiXmlElement * file = devDown->FirstChildElement("File");
        while (file != NULL) {
            const char * url = file->Attribute("Source");
            const char * dest = file->Attribute("Destination");
            const char * region = file->Attribute("RegionId");
            if ((url != NULL) && (dest != NULL)) {
                string strRegion = "";
                if (region != NULL) {
                    strRegion = region;
                }
                string strUrl = url;
                string strDest = dest;

                if ((strUrl.length() > 0) && (strDest.length() > 0)) {
                    // Replace \ with /
                    string::size_type pos = strDest.find("\\", 0);
                    while(string::npos != pos ) {
                            strDest.replace(pos, 1, "/");
                            pos = strDest.find("\\", 0);
                    }

                    //If anyone knows a better way to detect directory traversal, please notify me
                    pos = strDest.find("../", 0);
                    if (string::npos == pos ) {

                        //Test if target directory is a valid write directory
                        bool directoryIsValid = false;
                        string fileNameOnly = basename(strDest.c_str());
                        string directoryOnly = "";
                        if (fileNameOnly.length() < strDest.length()) {
                            directoryOnly = strDest.substr(0,strDest.length()-fileNameOnly.length()-1);
                        }
                        Log::dbg("Comparing with "+directoryOnly);
                        for (list<string>::iterator it=writeableDirectories.begin(); it!=writeableDirectories.end(); it++) {
                            if (directoryOnly.compare((*it)) == 0) { directoryIsValid = true; }
                        }

                        if (directoryIsValid) {
                            DeviceDownloadData fileElement;
                            fileElement.url = strUrl;
                            fileElement.destination = strDest;
                            fileElement.regionId = strRegion;
                            deviceDownloadList.push_back(fileElement);
                        } else {
                            Log::err("Device does not allow to write to this path: "+strDest);
                        }
                    } else {
                        Log::err("Invalid filename! Found '..' Directory traversal not allowed!");
                    }
                }
            } else {
                if (Log::enabledDbg()) { Log::dbg("Received an element with no Source/Destination Attribute"); }
            }
            file = file->NextSiblingElement("File");
        }
    } else {
        if (Log::enabledDbg()) { Log::dbg("Unable to find xml element DeviceDownload in data"); }
    }

    if (Log::enabledDbg()) {
        stringstream ss;
        ss << "Received a list of " << deviceDownloadList.size() << " files to download!";
        Log::dbg(ss.str());
    }

    if (deviceDownloadList.size() > 0) {
        downloadDataErrorCount = 0;
    }

    return deviceDownloadList.size();
}

string GarminFilebasedDevice::getNextDownloadDataUrl() {
    if (deviceDownloadList.size() > 0) {
        DeviceDownloadData downloadData = deviceDownloadList.front();
        return downloadData.url;
    }
    return "";
}

int GarminFilebasedDevice::writeDownloadData(char * buf, int length) {
    if (deviceDownloadList.size() > 0) {
        DeviceDownloadData downloadData = deviceDownloadList.front();
        string filename = baseDirectory + "/" + downloadData.destination;

        if (Log::enabledDbg()) {
            stringstream ss;
            ss << "Writing " << length << " bytes to file " << filename;
            Log::dbg(ss.str());
        }
        if (!downloadDataOutputStream.is_open()) {
            downloadDataOutputStream.open(filename.c_str(), ios::out | ios::binary);
        }
        if (downloadDataOutputStream.is_open()) {
            downloadDataOutputStream.write (buf, length);
        } else {
            downloadDataErrorCount++;
            Log::err("Unable to open file "+filename);
            return -1;
        }
    }
    return length;
}

void GarminFilebasedDevice::saveDownloadData() {
    Log::dbg("saveDownloadData was called for "+this->displayName);
    if (downloadDataOutputStream.is_open()) {
        downloadDataOutputStream.close();
        if (!deviceDownloadList.empty()) {
            deviceDownloadList.pop_front();
            Log::dbg("Removing file to download from list");
        }
    } else {
        Log::dbg("Not closing anything, since nothing was open...");
    }
}

void GarminFilebasedDevice::cancelDownloadData() {
    Log::dbg("cancelDownloadData was called for "+this->displayName);
    if (downloadDataOutputStream.is_open()) {
        downloadDataOutputStream.close();
    }
    if (!deviceDownloadList.empty()) {
        deviceDownloadList.pop_front();
    }
    downloadDataErrorCount++;
    this->transferSuccessful = false;
}


/**
 * This is used to indicate the status of the write download data process.
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GarminFilebasedDevice::finishDownloadData() {
    if (downloadDataErrorCount > 0) {
        this->transferSuccessful = false;
        return 3; // Finished
    }
    if (deviceDownloadList.size() > 0) {
        return 1;
    } else {
        this->transferSuccessful = true;
        return 3;
    }
}

/**
* Starts a thread that writes the passed xml string to the given filename
* @param filename - filename on disk
* @param data - the filename to write to on the device.
* @param dataTypeName - a Fitness DataType from the GarminDevice.xml retrieved with DeviceDescription
* @return int returns 1 if successful otherwise 0
*/
int GarminFilebasedDevice::startWriteFitnessData(string filename, string data, string dataTypeName) {
    if (Log::enabledDbg()) { Log::dbg("startWriteFitnessData is not yet implemented for "+this->displayName); }
    return 0;
}

/**
 * This is used to indicate the status of the write fitness data process.
 * @return 0 = idle 1 = working 2 = waiting 3 = finished
 */
int GarminFilebasedDevice::finishWriteFitnessData() {
    if (Log::enabledDbg()) { Log::dbg("finishWriteFitnessData is not yet implemented for "+this->displayName); }
    return 3;
}

/**
 * Cancels the current write of fitness data
 */
void GarminFilebasedDevice::cancelWriteFitnessData() {
    if (Log::enabledDbg()) { Log::dbg("cancelWriteFitnessData is not yet implemented for "+this->displayName); }
}
