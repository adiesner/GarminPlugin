#ifndef EDGE305DEVICE_H_INCLUDED
#define EDGE305DEVICE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"
#include "gpsDevice.h"
#include "garmin.h"

using namespace std;

class MessageBox;

class Edge305Device : public GpsDevice
{
public:
    Edge305Device();

    Edge305Device(string name);

    virtual ~Edge305Device();


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
     * Checks if a device is attached and returns the device name.
     * If no device is present and empty string is returned
     * @return string with device name
     */
    static string getAttachedDeviceName();

    string getDeviceDescription() const;
    int startWriteToGps(string filename, string xml);
    int finishWriteToGps();
    void cancelWriteToGps();
    int getTransferSucceeded();
    void setStorageCommand(string cmd);
    MessageBox * getMessage();
    void userAnswered(const int answer);
    bool isDeviceAvailable();

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

  /**
   * Directory where this device stores its fitness data
   */
    string fitnessDirectory;

    string fitnessFileExtension;

    /**
     * Reads fitnessdata xml from a garmin device like Edge 305/Forerunner 305
     * @return xml containing fitness data read from garmin device
     */
    string readFitnessData(bool readTrackData, string fitnessDetailId);


    /**
     * Stores the data read from the device to speed up things
     */
    garmin_data * fitnessdata;

private:

    /**
     * Prints an activity read from the garmin device
     * @param run Garmin list of points describing the runs
     * @param lap Garmin list of points describing the laps
     * @param track Garmin list of points describing the tracks
     * @param garmin device descriptor
     * @return xml string that describes all activities provided by the given lists
     */
    string printActivities(garmin_list * run, garmin_list * lap, garmin_list * track, const garmin_unit garmin, bool readTrackData, string fitnessDetailId);


    /**
     * Prints the fitness data header
     * @return xml string
     */
    string getFitnessDataHeader();

    /**
     * Prints the fitness data footer
     * @return xml string
     */
    string getFitnessDataFooter();

    /**
     * Prints the header of a run
     * @param D1009 internal run format of garmintools
     * @return xml string
     */
    string getRunHeader(D1009 * runData) ;

    /**
     * Prints the footer of a run
     * @return xml string
     */
    string getRunFooter();

    /**
     * Prints the header of a lap
     * @param D1011 internal lap format of garmintools
     * @param firstLap if set to true an <ID>Time</ID> header is added
     * @param printTrackData if <Track> should be outputted
     * @return xml string
     */
    string getLapHeader(D1011 * lapData, bool firstLap, bool printTrackData);

    /**
     * Prints the footer of a lap
     * @param printTrackData set to true if you have track data
     * @return xml string
     */
    string getLapFooter(bool printTrackData);

    /**
     * Prints a time in the format 2007-04-20T23:55:01Z
     * @param t timestamp
     * @return string
     */
    string print_dtime( uint32 t );

    /**
     * Prints a track point
     * @param D304 internal track point format of garmintools
     * @return xml string
     */
    string getTrackPoint ( D304 * p);

    /**
     * Prints information about the device
     * @param garmin device descriptor
     * @return xml string
     */
    string getCreator(const garmin_unit garmin);

  /**
   * Stores the fitnessData which was read from the device
   */
    string fitnessDataTcdXml;

    bool transferSuccessful;

    static string filterDeviceName(string name);

    string readFitnessDetailId;

   /**
    * Stores type of current run (0=Biking, 1=Running, 2=Other)
    */
    int runType;  
};



#endif // EDGE305DEVICE_H_INCLUDED
