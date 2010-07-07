#ifndef GPSFUNCTIONS_H_INCLUDED
#define GPSFUNCTIONS_H_INCLUDED

#include <iostream>
#include <math.h>
#include <sstream>

#define d2r (M_PI / 180.0)

//calculate haversine distance for linear distance
double haversine_km(double lat1, double long1, double lat2, double long2)
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6367 * c;

    return d;
}

double haversine_mi(double lat1, double long1, double lat2, double long2)
{
    double dlong = (long2 - long1) * d2r;
    double dlat = (lat2 - lat1) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 3956 * c;

    return d;
}

double haversine_m_str(string lat1, string long1, string lat2, string long2) {
    double dCoords[4];
    std::istringstream ss( lat1 + " " + long1 + " " +lat2 + " " + long2, istringstream::in);
    for (int n=0; n<4; n++) {
        ss >> dCoords[n];
    }
    return haversine_km(dCoords[0],dCoords[1],dCoords[2],dCoords[3]) * 1000;
}


#endif // GPSFUNCTIONS_H_INCLUDED
