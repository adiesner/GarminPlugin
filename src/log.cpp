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


#include "log.h"

#include <iostream>
#include <fstream>
# include <ctime>

Log * Log::instance;
LogLevel Log::level = Error;

Log::Log() {
    this->logfile = "";
}

Log::~Log() {
}

void Log::print(const string text) {
    string outtext = getTimestamp() + text;
    if (this->logfile == "") {
        cerr << outtext<< endl;
    } else {
        ofstream logf;
        logf.open (this->logfile.c_str(), ios::out | ios::app );
        logf << outtext << endl;
        logf.close();
    }
}

Log * Log::getInstance() {
    if (Log::instance == NULL) {
        Log::instance = new Log();
    }
    return Log::instance;
}

void Log::dbg(const string text) {
    if (Log::level <= Debug) {
        Log::getInstance()->print(text);
    }
}

void Log::info(const string text) {
    if (Log::level <= Info) {
        Log::getInstance()->print(text);
    }
}

void Log::err(const string text) {
    if (Log::level <= Error) {
        Log::getInstance()->print(text);
    }
}

bool Log::enabledDbg() {
    if (Log::level <= Debug) {
        return true;
    }
    return false;
}

bool Log::enabledInfo() {
    if (Log::level <= Info) {
        return true;
    }
    return false;
}

bool Log::enabledErr() {
    if (Log::level <= Error) {
        return true;
    }
    return false;
}

void Log::setConfiguration(TiXmlDocument * config) {
    /* <GarminPlugin logfile="" level="ERROR"> */
    TiXmlElement * plugin = config->FirstChildElement( "GarminPlugin" );
    const char * logfileAttr = plugin->Attribute("logfile");
    const char * levelAttr = plugin->Attribute("level");

    if (levelAttr != NULL) {
        string levelStr = levelAttr;
        if (levelStr == "DEBUG") {
            this->level = Debug;
        } else if (levelStr == "INFO") {
            this->level = Info;
        } else if (levelStr == "ERROR") {
            this->level = Error;
        } else {
            this->level = None;
        }
    }

    if (logfileAttr != NULL) {
        this->logfile = logfileAttr;
    } else {
        this->logfile = "";
    }

}

string Log::getTimestamp() {
    time_t now = time ( NULL );
    const struct tm* tm = localtime ( &now );

    char* s = new char[40];
    strftime ( s, 40, "%d.%m.%y %H:%M:%S ", tm );
    string str = s;
    delete[] s;
    return str;
}
