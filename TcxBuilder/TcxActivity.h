#ifndef TCXACTIVITY_H_INCLUDED
#define TCXACTIVITY_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"

#include "TcxLap.h"
#include "TcxCreator.h"
#include "TcxTypes.h"

using namespace std;

class TcxActivity
{
public:
    TcxActivity(string id);
    ~TcxActivity();

    void addLap(TcxLap* lap);
    string getId();

    TiXmlElement * getTiXml(bool readTrackData);

    void setSportType(TrainingCenterDatabase::Sport_t type);
    void setId(string id);

    friend TcxActivity& operator<<(TcxActivity& activity, TcxLap* lap);
    friend TcxActivity& operator<<(TcxActivity& activity, TcxCreator* creator);

private:
    string id;
    TrainingCenterDatabase::Sport_t sportType;
    vector<TcxLap*> lapList;
    TcxCreator* creator;
};

#endif // TCXACTIVITY_H_INCLUDED
