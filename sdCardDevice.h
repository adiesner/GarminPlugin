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
