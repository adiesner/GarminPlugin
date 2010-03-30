#ifndef OREGONDEVICE_H_INCLUDED
#define OREGONDEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"
#include "garminFilebasedDevice.h"

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


protected:
    virtual void doWork();
    void readFitnessDataFromDevice();

    virtual void setPathesFromConfiguration();


private:

  /**
   * File where this device stores its fitness data
   */
    string fitnessFile;

  /**
   * Stores the fitnessData which was read from the device
   */
    string fitnessDataTcdXml;

};



#endif // OREGONDEVICE_H_INCLUDED
