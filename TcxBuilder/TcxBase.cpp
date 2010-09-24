#include "TcxBase.h"

TcxBase::TcxBase() {
    this->author = NULL;
}

TcxBase::~TcxBase() {
    vector<TcxActivities*>::iterator it;
    for ( it=activitiesList.begin() ; it < activitiesList.end(); it++ )
    {
        TcxActivities* activities = *it;
        delete(activities);
    }
    activitiesList.clear();
}

void TcxBase::addActivities(TcxActivities* activities) {
    this->activitiesList.push_back(activities);
}

TcxBase& operator<<(TcxBase& base, TcxActivities* activities)
{
    base.addActivities(activities);
    return base;
}

TcxBase& operator<<(TcxBase& base, TcxAuthor* author)
{
    if (base.author != NULL) {
        delete(base.author);
    }
    base.author = author;
    return base;
}


TiXmlDocument * TcxBase::getTcxDocument(bool readTrackData, string fitnessDetailId) {
    TiXmlDocument * doc = new TiXmlDocument();

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    doc->LinkEndChild( decl );

    TiXmlElement * train = new TiXmlElement( "TrainingCenterDatabase" );
    train->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2");
    train->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
    train->SetAttribute("xsi:schemaLocation","http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2 http://www.garmin.com/xmlschemas/TrainingCenterDatabasev2.xsd http://www.garmin.com/xmlschemas/ActivityExtension/v2 http://www.garmin.com/xmlschemas/ActivityExtensionv2.xsd");
    doc->LinkEndChild( train );

    vector<TcxActivities*>::iterator it;
    for ( it=activitiesList.begin() ; it < activitiesList.end(); it++ )
    {
        TcxActivities* activities = *it;
        train->LinkEndChild( activities->getTiXml(readTrackData, fitnessDetailId) );
    }

    if (this->author != NULL) {
        train->LinkEndChild(this->author->getTiXml());
    }

    return doc;
}

TiXmlDocument * TcxBase::getGpxDocument() {
    TiXmlDocument * doc = new TiXmlDocument();

    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no");
    doc->LinkEndChild( decl );

    TiXmlElement * gpx = new TiXmlElement( "gpx" );
    gpx->SetAttribute("xmlns","http://www.topografix.com/GPX/1/1");
    gpx->SetAttribute("xmlns:gpxx","http://www.garmin.com/xmlschemas/GpxExtensions/v3");
    gpx->SetAttribute("xmlns:gpxtpx","http://www.garmin.com/xmlschemas/TrackPointExtension/v1");
    gpx->SetAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
    gpx->SetAttribute("creator","GarminPlugin");
    gpx->SetAttribute("version","1.1");
    gpx->SetAttribute("xsi:schemaLocation","http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www.garmin.com/xmlschemas/GpxExtensionsv3.xsd http://www.garmin.com/xmlschemas/TrackPointExtension/v1 http://www.garmin.com/xmlschemas/TrackPointExtensionv1.xsd");
    doc->LinkEndChild( gpx );

    vector<TcxActivities*>::iterator it;
    for ( it=activitiesList.begin() ; it < activitiesList.end(); it++ )
    {
        TcxActivities* activities = *it;
        vector<TiXmlElement*> trkElem = activities->getGpxTiXml();
        vector<TiXmlElement*>::iterator it;
        for ( it=trkElem.begin() ; it < trkElem.end(); it++ ) {
            TiXmlElement* elem = *it;
            gpx->LinkEndChild( elem );
        }
    }

    return doc;
}
