#ifndef GPSFUNCTIONS_H_INCLUDED
#define GPSFUNCTIONS_H_INCLUDED

#include <iostream>
#include <math.h>
#include <sstream>

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
};

#endif // GPSFUNCTIONS_H_INCLUDED
