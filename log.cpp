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
    const struct tm *tm;
    size_t len;
    time_t now;
    char *s;

    now = time ( NULL );
    tm = localtime ( &now );

    s = new char[40];
    len = strftime ( s, 40, "%d.%m.%y %H:%M:%S ", tm );
    string str = s;
    delete s;
    return str;
}
