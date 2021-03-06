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


#ifndef MESSAGEBOX_H_INCLUDED
#define MESSAGEBOX_H_INCLUDED

#define BUTTON_YES    1
#define BUTTON_NO     2
#define BUTTON_OK     4
#define BUTTON_CANCEL 8

#include <string>
#include "gpsDevice.h"

enum MessageType {
    Question
};

class GpsDevice;

class MessageBox
{
public:
  /**
   * Creates a new messagebox which can be displayed to the user and answered by him
   * @param type Sets the icon of the message box displayed to the user (theoretically)
   * @param text Text to display to the user
   * @param buttons define the buttons to display to the user (Current Garmin Javascript always shows Ok/Cancel)
   * @param defaultBtn defines the default button
   * @param device is the answer is important to a device, refer to the device here.
   */
    MessageBox(MessageType type, std::string text, int buttons, int defaultBtn, GpsDevice *device);

  /**
   * Gets the xml string that describes the messageBox
   * @return xml string
   */
    std::string getXml();

  /**
   * If a user answers a message this function will be called
   * If a reference to a gps device is set, the answer will be passed to the gps device
   * @param result containing the id of the button (theoretically - current garmin javascript does not support this)
   */
    void responseReceived(const int result);

private:

  /**
   * Reference to a gps device that cares about the answer
   */
    GpsDevice * device;

  /**
   * Text containing the message to the user
   */
    std::string text;

  /**
   * Buttons that should be displayed in the message box
   */
    int buttons;

  /**
   * Stores the default button
   */
    int defaultButton;

  /**
   * Defines the message type (=icon for the user)
   */
    MessageType type;
};


#endif // MESSAGEBOX_H_INCLUDED
