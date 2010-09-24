#ifndef OREGONDEVICE_H_INCLUDED
#define OREGONDEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"
#include "garminFilebasedDevice.h"
#include "TcxBuilder/TcxBase.h"

using namespace std;

class MessageBox;

class OregonDevice : public GarminFilebasedDevice
{
public:
    OregonDevice();

    virtual ~OregonDevice();


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
   * Sets the path where the fitness data is located
   * @param path where the tcx files are located on the device
   */
    void setFitnessDataPath(string path);


  /**
   * Gets the fitness data xml
   * @return xml containing fitness data read from garmin device
   */
    string getFitnessData();

    void setStorageDirectory(string path);

  /**
   * Starts reading the fitness data without points
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startReadFITDirectory();

  /**
   * Starts reading the fitness data without points
   * @return int returns 1 if successful otherwise 0
   */
    virtual int startReadFitnessDirectory();

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


protected:
    virtual void doWork();
    void readFitnessDataFromDevice(bool readTrackData, string fitnessDetailId);

    virtual void setPathesFromConfiguration();


private:

  /**
   * Stores the fitnessData which was read from the device
   */
  string fitnessDataTcdXml;

  /**
   * Stores the id of the track that should be read
   */
  string readFitnessDetailId;

  /**
   * Stores the part number of the device
   */
  string partNumber;

  /**
   * Stores the unit id of the device
   */
  string unitId;

  /**
   * Stores the fitness data read from current.gpx
   */
  TcxBase *fitnessData;

};



#endif // OREGONDEVICE_H_INCLUDED
