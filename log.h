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


#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
using namespace std;

/**
 * Specifies all possible loglevel modes.
 * The ones below the current mode are always included.
 */
enum LogLevel
{
  Debug, /**< enum value Debug. Developer Information Output */
  Info,  /**< enum value Debug. Output additional informations */
  Error, /**< enum value Debug. Output only errors like file i/o */
  None   /**< enum value None. No output at all */
};

class Log
{
public:
  /**
   * Prints a string to the log output
   * @param text Text to print.
   */
    void print(const string text);

  /**
   * Returns the instance of this class
   * @return Instance to this class
   */
    static Log * getInstance();

  /**
   * Prints text if level is at least Debug
   * @param text to print
   */
    static void dbg(const string text);

  /**
   * Prints text if level is at least Info
   * @param text to print
   */
    static void info(const string text);

  /**
   * Prints text if level is at least Error
   * @param text to print
   */
    static void err(const string text);

  /**
   * Check debug level setting
   * @return true if debug is enabled
   */
    static bool enabledDbg();

  /**
   * Check info level setting
   * @return true if info is enabled
   */
    static bool enabledInfo();
  /**
   * Check error level setting
   * @return true if error is enabled
   */
    static bool enabledErr();

  /**
   * Initializes the Logger with current configuration settings
   * @param config Current configuration
   */
    void setConfiguration(TiXmlDocument * config);

private:
  /**
   * Private constructor.
   * Use Log::getInstance() to get an instance of this class
   * @see getInstance()
   */
    Log();

  /**
   * Private destructor.
   */
    ~Log();


  /**
   * Stores the only instance of this class.
   */
    static Log * instance;

  /**
   * Logfile to write to. If empty stderr is used.
   */
    string logfile;

  /**
   * Loglevel to use
   */
    static LogLevel level;

  /**
   * Returns the current time "DD.MM.YY HH:MM:SS "
   */
    string getTimestamp();
};

#endif // LOG_H_INCLUDED
