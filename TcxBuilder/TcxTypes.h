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
#ifndef TCXTYPES_H_INCLUDED
#define TCXTYPES_H_INCLUDED

#include <sstream>
#include <string>

using namespace std;

class TrainingCenterDatabase {
    public:
        enum Intensity_t {
            Active,
            Resting
        };

        enum TriggerMethod_t {
            Manual,
            Distance,
            Location,
            Time,
            HeartRate
        };

        enum CadenceSensorType_t
        {
          Footpod,
          Bike,
          UndefinedCadenceType
        };

        enum SensorState_t
        {
            Present,
            Absent,
            UndefinedSensorState
        };

        enum Sport_t
        {
            Running,
            Biking,
            Other
        };

        static string limitIntValue(string value, int min, int max) {
            stringstream newValue;
            int intValue;
            std::istringstream ss( value );
            ss >> intValue;
            if (intValue < min) {
                newValue << min;
            } else if (intValue > max) {
                newValue << max;
            } else {
                newValue << value;
            }
            return newValue.str();
        };



};

#endif // TCXTYPES_H_INCLUDED
