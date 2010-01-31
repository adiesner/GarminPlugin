
#include "deviceManager.h"
#include "log.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager() {
    if (Log::enabledDbg()) Log::dbg("DeviceManager destructor");
    while (gpsDeviceList.size() > 0)
    {
        GpsDevice *dev = gpsDeviceList.back();
        gpsDeviceList.pop_back();
        delete(dev);
    }
}


const std::string DeviceManager::getDevicesXML()
{
    // <?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<Devices xmlns=\"http://www.garmin.com/xmlschemas/PluginAPI/v1\">\n<Device DisplayName=\"Oregon (/mnt/Oregon/)\" Number=\"0\"/>\n</Devices>\n
    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
	TiXmlElement * devices = new TiXmlElement( "Devices" );
    devices->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/PluginAPI/v1");

    for(unsigned int ii=0; ii < gpsDeviceList.size(); ii++)
    {
        if (gpsDeviceList[ii]->isDeviceAvailable()) {
            TiXmlElement *device = new TiXmlElement ( "Device" );
            device->SetAttribute("DisplayName", gpsDeviceList[ii]->getDisplayName());
            device->SetAttribute("Number", ii);
            devices->LinkEndChild( device );
        }
    }

    doc.LinkEndChild( decl );
    doc.LinkEndChild( devices );

    TiXmlPrinter printer;
	printer.SetIndent( "\t" );
	doc.Accept( &printer );
    string str = printer.Str();

    return str;
}

void DeviceManager::startFindDevices() {

}

void DeviceManager::createDeviceList(TiXmlDocument * doc) {
    if (doc == NULL) { return; }
/*  <GarminPlugin>
      <Devices>
        <Device>
          <Name>My Oregon 300</Name>
          <StoragePath>/tmp</StoragePath>
          <StorageCommand></StorageCommand>
        </Device>
      </Devices>
    </GarminPlugin> */
    TiXmlElement * pRoot = doc->FirstChildElement( "GarminPlugin" );
    if (pRoot) {
        TiXmlElement * devices = pRoot->FirstChildElement("Devices");
        TiXmlElement * device = devices->FirstChildElement("Device");
        while ( device )
        {
            GpsDevice *dev = new GpsDevice();
            TiXmlElement * name = device->FirstChildElement("Name");
            if (name) {
                if (name->GetText() != NULL)
                    dev->setDisplayName(name->GetText());
            }
            TiXmlElement * dir = device->FirstChildElement("StoragePath");
            if (dir) {
                if (dir->GetText() != NULL)
                    dev->setStorageDirectory(dir->GetText());
            }
            TiXmlElement * cmd = device->FirstChildElement("StorageCommand");
            if (cmd) {
                if (cmd->GetText() != NULL)
                    dev->setStorageCommand(cmd->GetText());
            }

            gpsDeviceList.push_back(dev);

            device = device->NextSiblingElement( "Device" );
        }
    }
}

void DeviceManager::setConfiguration(TiXmlDocument * config) {
    createDeviceList(config);
}


void DeviceManager::cancelFindDevices() {

}

int DeviceManager::finishedFindDevices() {
    return 1;
}

GpsDevice * DeviceManager::getGpsDevice(int number)
{
    return gpsDeviceList[number];
}
