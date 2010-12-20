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
                if (nameText.compare("GPSData") == 0) {
                    node2 = node->FirstChildElement("File");
                    while (node2 != NULL) {
                        TiXmlElement * transferDirection = node2->FirstChildElement("TransferDirection");
                        string transDir = transferDirection->GetText();

                        // Get directory to write GPX
                        if ((transDir.compare("InputToUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
                            TiXmlElement * loc = NULL;
                            if (node2!=NULL) { loc = node2->FirstChildElement("Location"); }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("Path"); }

                            if (node2!=NULL) {
                                Log::dbg("Found path: "+ string(node2->GetText()) + " for GPSData");
                                this->gpxDirectory = this->baseDirectory + "/" + node2->GetText();
                            }
                            if (loc!=NULL)   { node2 = loc->FirstChildElement("FileExtension"); }
                            if (node2!=NULL) {
                                this->gpxFileExtension = node2->GetText();
                            }
                        }

                        // Get location of current.gpx file
                        if ((transDir.compare("OutputFromUnit") == 0) || (transDir.compare("InputOutput") == 0)) {
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
