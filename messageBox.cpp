/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2010 <andreas.diesner [AT] gmx [DOT] de>
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


#include "messageBox.h"

#include "log.h"

using namespace std;

MessageBox::MessageBox(MessageType type, string text, int buttons, int defaultBtn, GpsDevice *device)
{
    this->device = device;
    this->text = text;
    this->buttons = buttons;
    this->defaultButton = defaultBtn;
    this->type = type;
}


string MessageBox::getXml() {
/*
<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<MessageBox xmlns="http://www.garmin.com/xmlschemas/PluginAPI/v1" DefaultButtonValue="2">
  <Icon>Question</Icon>
  <Text>The file F:/Garmin/gpx/GC22K31.gpx already exists on your GPS Device. OK to overwrite the file?</Text>
  <Button Caption="OK" Value="1"/>
  <Button Caption="Cancel" Value="2"/>
</MessageBox>
*/

    TiXmlDocument doc;
    TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "no" );
    doc.LinkEndChild( decl );

	TiXmlElement * msgBox = new TiXmlElement( "MessageBox" );
    msgBox->SetAttribute("xmlns", "http://www.garmin.com/xmlschemas/PluginAPI/v1");
    msgBox->SetAttribute("DefaultButtonValue", this->defaultButton);
    doc.LinkEndChild( msgBox );

	TiXmlElement * icon = new TiXmlElement( "Icon" );
	if (this->type == Question) {
        icon->LinkEndChild(new TiXmlText("Question"));
	} else {
	    Log::err("MessageBox::getXml Message type not yet implemented!");
        icon->LinkEndChild(new TiXmlText("Unknown"));
	}
    msgBox->LinkEndChild( icon );

	TiXmlElement * textelem = new TiXmlElement( "Text" );
    textelem->LinkEndChild(new TiXmlText(this->text));
    msgBox->LinkEndChild( textelem );

    if ((this->buttons & BUTTON_OK) > 0) {
        TiXmlElement * btn = new TiXmlElement( "Button" );
        btn->SetAttribute("Caption", "OK");
        btn->SetAttribute("Value", BUTTON_OK);
        msgBox->LinkEndChild( btn );
    }

    if ((this->buttons & BUTTON_CANCEL) > 0) {
        TiXmlElement * btn = new TiXmlElement( "Button" );
        btn->SetAttribute("Caption", "Cancel");
        btn->SetAttribute("Value", BUTTON_CANCEL);
        msgBox->LinkEndChild( btn );
    }

    if ((this->buttons & BUTTON_YES) > 0) {
        TiXmlElement * btn = new TiXmlElement( "Button" );
        btn->SetAttribute("Caption", "Yes");
        btn->SetAttribute("Value", BUTTON_YES);
        msgBox->LinkEndChild( btn );
    }

    if ((this->buttons & BUTTON_NO) > 0) {
        TiXmlElement * btn = new TiXmlElement( "Button" );
        btn->SetAttribute("Caption", "No");
        btn->SetAttribute("Value", BUTTON_NO);
        msgBox->LinkEndChild( btn );
    }

    TiXmlPrinter printer;
	//printer.SetIndent( "\t" );
	doc.Accept( &printer );
    string str = printer.Str();

    return str;
}

void MessageBox::responseReceived(const int result) {
    if (this->device != NULL) {
        device->userAnswered(result);
    }
}
