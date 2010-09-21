#ifndef TCXCREATOR_H_INCLUDED
#define TCXCREATOR_H_INCLUDED

#include <string>

#define TIXML_USE_TICPP
#include "ticpp.h"

using namespace std;

class TcxCreator
{
public:
    TcxCreator();
    ~TcxCreator();

    TiXmlElement * getTiXml();

    void setName(string name);
    void setUnitId(string id);
    void setProductId(string id);
    void setVersion(string version);
    void setVersion(string major, string minor);
    void setBuild(string build);
    void setBuild(string major, string minor);

private:
    string name;
    string unitId;
    string productId;
    string versionMajor;
    string versionMinor;
    string buildMajor;
    string buildMinor;
};

#endif // TCXCREATOR_H_INCLUDED
