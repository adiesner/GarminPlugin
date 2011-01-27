#ifndef EDGE800DEVICE_H_INCLUDED
#define EDGE800DEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include <list>
#include "messageBox.h"
#include "garminFilebasedDevice.h"


#include "fit/fitReader.hpp"
#include "fit/fitMsg.hpp"

using namespace std;

class MessageBox;

class Edge800Device : public GarminFilebasedDevice, FitMsg_Listener
{
public:
    Edge800Device();

    virtual ~Edge800Device();


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

  /**
   * Starts reading the fitness data without points
   * @return int returns 1 if successful otherwise 0
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
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startReadFitnessDirectory();

    /**
     * Gets the FIT data xml
     * @return xml containing FIT directory data read from garmin device
     */
    virtual string getFITData();

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

    /**
     * Returns a file from the device
     */
    virtual string getBinaryFile(string relativeFilePath);

    /**
     * Receives all decoded messages stored in the fit file
     */
    virtual void fitMsgReceived(FitMsg *msg);

protected:
    virtual void doWork();
    void readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId);

    /**
     * Reads file structure of fit directories for ReadFITDirectory
     */
    void readFITDirectoryFromDevice();

  /**
   * Directory where this device stores its fitness data
   */
    string fitnessDirectory;

    string fitnessFileExtension;

    virtual void setPathesFromConfiguration();

    typedef struct _FitDirectory {
        string path;        /**< Path to the FIT files */
        string extension;   /**< Extension of the fit files */
        string type;        /**< Type of the fit files */
    } FitDirectory;

    list <FitDirectory> fitDirectoryList;

private:


  /**
   * Stores the fitnessData which was read from the device
   */
    string fitnessDataTcdXml;

  /**
   * Stores the FIT Directory Data which was read from the device
   */
    string fitDirectoryXml;

    /**
     * Stores the id of the track that should be read
     */
    string readFitnessDetailId;

    /**
     * Stores the current xml element where the Fit Message Listener (fitMsgReceived())
     * should write the file information data to
     */
    TiXmlElement * fitFileElement;

};



#endif // EDGE800DEVICE_H_INCLUDED
