#ifndef TCXAUTHOR_H_INCLUDED
#define TCXAUTHOR_H_INCLUDED

#include <string>

#define TIXML_USE_TICPP
#include "ticpp.h"

using namespace std;

class TcxAuthor
{
public:
    TcxAuthor();
    ~TcxAuthor();

    TiXmlElement * getTiXml();

    void setName(string name);
    void setVersion(string version);
    void setVersion(string major, string minor);
    void setBuild(string build);
    void setBuild(string major, string minor);
    void setType(string type);
    void setPartNumber(string number);
    void setLangId(string id);

private:
    string name;
    string versionMajor;
    string versionMinor;
    string buildMajor;
    string buildMinor;
    string partNumber;
    string type;
    string langId;

};

#endif // TCXAUTHOR_H_INCLUDED
