#ifndef TCXTYPES_H_INCLUDED
#define TCXTYPES_H_INCLUDED

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
