#ifndef TCXBASE_H_INCLUDED
#define TCXBASE_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"

#include <vector>
#include "TcxActivities.h"
#include "TcxAuthor.h"

using namespace std;

class TcxBase
{
public:
    TcxBase();

    ~TcxBase();

    void addActivities(TcxActivities* activities);

    TiXmlDocument * getTcxDocument(bool readTrackData, string fitnessDetailId);
    TiXmlDocument * getGpxDocument();

    friend TcxBase& operator<<(TcxBase& base, TcxActivities* activities);
    friend TcxBase& operator<<(TcxBase& base, TcxAuthor* author);

private:
    vector<TcxActivities*> activitiesList;
    TcxAuthor * author;


};

#endif // TCXBASE_H_INCLUDED
