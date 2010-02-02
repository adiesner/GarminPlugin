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
