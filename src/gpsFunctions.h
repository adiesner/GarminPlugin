/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2011 <garminplugin [AT] andreas.diesner [DOT] de>
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
#ifndef GPSFUNCTIONS_H_INCLUDED
#define GPSFUNCTIONS_H_INCLUDED

#include <iostream>
#include <math.h>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <cerrno>

#define d2r (M_PI / 180.0)

class GpsFunctions {
    public:
    //calculate haversine distance for linear distance
    static double haversine_km(double lat1, double long1, double lat2, double long2)
    {
        double dlong = (long2 - long1) * d2r;
        double dlat = (lat2 - lat1) * d2r;
        double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double d = 6367 * c;

        return d;
    }

    static double haversine_mi(double lat1, double long1, double lat2, double long2)
    {
        double dlong = (long2 - long1) * d2r;
        double dlat = (lat2 - lat1) * d2r;
        double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double d = 3956 * c;

        return d;
    }

    static double haversine_m_str(string lat1, string long1, string lat2, string long2) {
        double dCoords[4];
        std::istringstream ss( lat1 + " " + long1 + " " +lat2 + " " + long2, istringstream::in);
        for (int n=0; n<4; n++) {
            ss >> dCoords[n];
        }
        return haversine_km(dCoords[0],dCoords[1],dCoords[2],dCoords[3]) * 1000;
    }

    /**
     * Prints a time in the format 2007-04-20T23:55:01Z
     * @param t timestamp
     * @return string
     */
     #define TIME_OFFSET  631065600
    static string print_dtime( unsigned int  t ) {
      time_t     tval;
      struct tm  tmval;
      char       buf[128];
      int        len;

      /*
                                      012345678901234567890123
         This will make, for example, 2007-04-20T23:55:01-0700, but that date
         isn't quite ISO 8601 compliant.  We need to stick a ':' in the time
         zone between the hours and the minutes.
      */

      tval = t + TIME_OFFSET;
      //localtime_r(&tval,&tmval);
      gmtime_r(&tval,&tmval);
      strftime(buf,sizeof(buf)-1,"%FT%TZ",&tmval);

      /*
         If the last character is a 'Z', don't do anything.  Otherwise, we
         need to move the last two characters out one and stick a colon in
         the vacated spot.  Let's not forget the trailing '\0' that needs to
         be moved as well.
      */

      len = strlen(buf);
      if ( len > 0 && buf[len-1] != 'Z' ) {
        memmove(buf+len-1,buf+len-2,3);
        buf[len-2] = ':';
      }

      return (string)buf;
    }

    static int mkpath(std::string path, mode_t mode) {
        size_t pre=0,pos;
        std::string dir;
        int mdret;

        if(path[path.size()-1]!='/') {
            path+='/';
        }

        while((pos=path.find_first_of('/',pre))!=std::string::npos){
            dir=path.substr(0,pos++);
            pre=pos;
            if(dir.size()==0) continue; // if leading / first time is 0 length
            if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
                return mdret;
            }
        }

        struct stat status;
        stat( path.c_str(), &status );
        if ( status.st_mode & S_IFDIR ) {
        	return EEXIST;
        }
        return mdret;
    }


    /**
     * Parses a tcx file, and returns the unix timestamp for the start timestamp
     */
    static time_t getStartTimestampFromXml(string xml)
    {
    	if (xml.length() == 0) { return 0; }
    	TiXmlDocument * xmldoc = new TiXmlDocument();
    	xmldoc->Parse(xml.c_str());
    	time_t res = getStartTimestampFromXml(xmldoc);
    	delete(xmldoc);
    	return res;
    }

    /**
     * Parses a tcx file, and returns the unix timestamp for the start timestamp
     */
    static time_t getStartTimestampFromXml(TiXmlDocument * xmldoc)
    {
    	if (xmldoc == NULL) { return 0; }
    	// TrainingCenterDatabase -> Activities -> Activity -> Lap . StartTime;
    	TiXmlElement * pElem = xmldoc->FirstChildElement( "TrainingCenterDatabase" );
    	if (pElem != NULL) {
    		pElem = pElem->FirstChildElement( "Activities" );
    		if (pElem != NULL) {
    			pElem = pElem->FirstChildElement( "Activity" );
    			if (pElem != NULL) {
    				pElem = pElem->FirstChildElement( "Lap" );
    				if (pElem != NULL) {
    					const char * startTimeStr = pElem->Attribute("StartTime");
    					if (startTimeStr!=NULL) {
    						struct tm start;
    						if (strptime(startTimeStr, "%FT%TZ",&start) != NULL) {
    							return mktime(&start);
    						}
    						if (strptime(startTimeStr, "%FT%T.000Z",&start) != NULL) {
    							return mktime(&start);
    						}
    					}
    				}
    			}
    		}
    	}
    	return 0;
    }

    static string str_replace (string rep, string wit, string in) {
    	  int pos;
    	  while (true) {
    	    pos = in.find(rep);
    	    if (pos == -1) {
    	      break;
    	    } else {
    	      in.erase(pos, rep.length());
    	      in.insert(pos, wit);
    	    }
    	  }
    	  return in;
    }

    static bool iequals(const string& a, const string& b)
    {
        unsigned int sz = a.size();
        if (b.size() != sz)
            return false;
        for (unsigned int i = 0; i < sz; ++i)
            if (tolower(a[i]) != tolower(b[i]))
                return false;
        return true;
    }

};

#endif // GPSFUNCTIONS_H_INCLUDED
