#ifndef CONFIGMANAGER_H_INCLUDED
#define CONFIGMANAGER_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "messageBox.h"

using namespace std;

class ConfigManager
{
public:
  /**
   * Creates a new instance of the ConfigManager
   */
    ConfigManager();

  /**
   * The desctructor which frees the configuration xml tree
   */
    ~ConfigManager();

  /**
   * Reads the configuration from the disk
   */
    void readConfiguration();

  /**
   * Returns the current configuration in memory.
   * @return The current configuration as XML tree
   */
     TiXmlDocument * getConfiguration();

  /**
   * Returns a message for the user in case of configuration problems
   * @return Messagebox object to be passed to the user
   */
    MessageBox * getMessage();

private:

  /**
   * Creates and stores a new configuration
   * @return the new configuration as XML tree
   */
    TiXmlDocument * createNewConfiguration();

  /**
   * Stores the current configuration in memory
   */
    TiXmlDocument * configuration;

  /**
   * Stores the current configuration file on disk
   */
    string configurationFile;

  /**
   * Is set to true when the configuration had to be created new
   */
    bool createdNew;
};

#endif // CONFIGMANAGER_H_INCLUDED
