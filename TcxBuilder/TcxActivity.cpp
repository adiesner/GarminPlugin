
#include "TcxActivity.h"

TcxActivity::TcxActivity(string id) {
    this->id = id;
    this->creator = NULL;
    this->sportType = TrainingCenterDatabase::Other;
}

TcxActivity::~TcxActivity() {
    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); it++ )
    {
        TcxLap* lap = *it;
        delete(lap);
    }
    lapList.clear();

    if (this->creator != NULL) {
        delete(this->creator);
    }
}

void TcxActivity::addLap(TcxLap* lap) {
    this->lapList.push_back(lap);
}

TiXmlElement * TcxActivity::getTiXml(bool readTrackData) {
    TiXmlElement * xmlActivity = new TiXmlElement("Activity");

    switch (this->sportType) {
        case TrainingCenterDatabase::Running:
            xmlActivity->SetAttribute("Sport","Running");
            break;
        case TrainingCenterDatabase::Biking:
            xmlActivity->SetAttribute("Sport","Biking");
            break;
        default:
            xmlActivity->SetAttribute("Sport","Other");
    }

    TiXmlElement * xmlId = new TiXmlElement("Id");
    xmlActivity->LinkEndChild(xmlId);
    xmlId->LinkEndChild(new TiXmlText(this->id));

    vector<TcxLap*>::iterator it;
    for ( it=lapList.begin() ; it < lapList.end(); it++ )
    {
        TcxLap* lap = *it;
        xmlActivity->LinkEndChild( lap->getTiXml(readTrackData) );
    }

    if (this->creator != NULL) {
        xmlActivity->LinkEndChild(this->creator->getTiXml());
    }
    return xmlActivity;
}

string TcxActivity::getId() {
    return this->id;
}

TcxActivity& operator<<(TcxActivity& activity, TcxLap* lap)
{
    activity.addLap(lap);
    return activity;
}

TcxActivity& operator<<(TcxActivity& activity, TcxCreator* creator)
{
    if (activity.creator != NULL) {
        delete activity.creator;
    }
    activity.creator = creator;
    return activity;
}

void TcxActivity::setSportType(TrainingCenterDatabase::Sport_t type) {
    this->sportType = type;
}

void TcxActivity::setId(string id) {
    this->id = id;
}
