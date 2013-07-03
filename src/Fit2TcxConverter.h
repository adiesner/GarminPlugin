/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Fit2Tcx
 * Copyright (C) Andreas Diesner 2011 <garminplugin [AT] andreas.diesner [DOT] de>
 *
 * Fit2Tcx is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fit2Tcx is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIT2TCXCONVERTER_H_
#define FIT2TCXCONVERTER_H_

#include "fit/fitMsg_Listener.hpp"
#include "fit/fitReader.hpp"
#include "fit/fitMsg.hpp"
#include "fit/fitMsg_File_ID.hpp"
#include "fit/fitMsg_File_Creator.hpp"
#include "fit/fitMsg_Lap.hpp"
#include "fit/fitMsg_Record.hpp"
#include "fit/fitMsg_Activity.hpp"
#include "fit/fitMsg_Session.hpp"
#include "fit/fitMsg_DeviceInfo.hpp"
#include "fit/fitFileException.hpp"

#include "TcxBuilder/TcxBase.h"

class Fit2TcxConverter: public FitMsg_Listener {
public:
	Fit2TcxConverter();
	virtual ~Fit2TcxConverter();

    /**
     * Overwrite to receive fit messages
     */
    virtual void fitMsgReceived(FitMsg *msg);

    /**
     * Overwrite and enable debug to receive debug messages
     */
    virtual void fitDebugMsg(string msg);

    string getTcxContent(bool readTrackData, string fitnessDetailId);

    TiXmlDocument * getTiXmlDocument(bool readTrackData, string fitnessDetailId);

private:

    void handle_File_ID(FitMsg_File_ID *fileid);

    void handle_File_Creator(FitMsg_File_Creator *filecreator);

    void handle_Lap(FitMsg_Lap *filelap);

    void handle_Activity(FitMsg_Activity *fileact);

    void handle_Record(FitMsg_Record *filerec);

    void handle_Session(FitMsg_Session *session);

    void handle_DeviceInfo(FitMsg_DeviceInfo *deviceInfo);

    void setTrackpointCadenceType(TrainingCenterDatabase::CadenceSensorType_t type);

	TcxBase *tcxBase;
	TcxActivities *tcxActivities;
	TcxActivity * tcxActivity;
	TcxAuthor *tcxAuthor;
	TcxLap *tcxLap;
	TcxTrack *tcxTrack;
	TcxCreator *tcxCreator;

	vector<TcxTrackpoint*> trackpointList;

};

#endif /* FIT2TCXCONVERTER_H_ */
