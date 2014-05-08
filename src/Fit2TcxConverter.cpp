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

#include "Fit2TcxConverter.h"
#include "gpsFunctions.h"
#include "fit/fitDefines.hpp"

#define DEGREES      180.0
#define SEMICIRCLES  0x80000000
#define SEMI2DEG(a)  (double)(a) * DEGREES / SEMICIRCLES


Fit2TcxConverter::Fit2TcxConverter() : 	tcxBase(NULL),
 	 	 	 	 	 	 	 	 	 	 tcxActivities(NULL),
 										tcxActivity(NULL),
 	 	 	 	 	 	 	 	 	 	 tcxAuthor(NULL),
										tcxLap(NULL),
										tcxTrack(NULL),
										tcxCreator(NULL)
{
}

Fit2TcxConverter::~Fit2TcxConverter() {
	// TODO Auto-generated destructor stub
}

void Fit2TcxConverter::fitMsgReceived(FitMsg *msg) {
    if (msg == NULL) { return; }
    if (this->tcxBase == NULL) {
    	tcxBase = new TcxBase();
    	tcxActivities = new TcxActivities();
    	*(tcxBase) << tcxActivities;
    	tcxActivity = new TcxActivity("dummy");
    	*(tcxActivities) << tcxActivity;
		tcxCreator = new TcxCreator();
		(*tcxActivity)<< tcxCreator;
		tcxAuthor = new TcxAuthor();
		(*tcxBase) << tcxAuthor;
    }

    if (msg->GetType() == FIT_MESSAGE_FILE_ID) {
        FitMsg_File_ID *fileid = dynamic_cast<FitMsg_File_ID*> (msg);
        if (fileid != NULL) { handle_File_ID(fileid); }
    } else if (msg->GetType() == FIT_MESSAGE_FILE_CREATOR) {
    	FitMsg_File_Creator *filecreator = dynamic_cast<FitMsg_File_Creator*> (msg);
        if (filecreator != NULL) { handle_File_Creator(filecreator); }
    } else if (msg->GetType() == FIT_MESSAGE_LAP) {
    	FitMsg_Lap *filelap = dynamic_cast<FitMsg_Lap*> (msg);
        if (filelap != NULL) { handle_Lap(filelap); }
    } else if (msg->GetType() == FIT_MESSAGE_ACTIVITY) {
    	FitMsg_Activity *fileact = dynamic_cast<FitMsg_Activity*> (msg);
        if (fileact != NULL) { handle_Activity(fileact); }
    } else if (msg->GetType() == FIT_MESSAGE_RECORD) {
    	FitMsg_Record *filerec = dynamic_cast<FitMsg_Record*> (msg);
        if (filerec != NULL) { handle_Record(filerec); }
    } else if (msg->GetType() == FIT_MESSAGE_SESSION) {
    	FitMsg_Session *session = dynamic_cast<FitMsg_Session*> (msg);
        if (session != NULL) { handle_Session(session); }
    } else if (msg->GetType() == FIT_MESSAGE_DEVICE_INFO) {
    	FitMsg_DeviceInfo *deviceInfo = dynamic_cast<FitMsg_DeviceInfo*> (msg);
        if (deviceInfo != NULL) { handle_DeviceInfo(deviceInfo); }
    } else {
        // received a message we are not interested in
    }
}

void Fit2TcxConverter::fitDebugMsg(string msg) {
	std::cout << msg << std::endl;
}

void Fit2TcxConverter::handle_Activity(FitMsg_Activity *activity) {
	/*
	 *  Nothing of interest available here
	 *
	activity->getEvent();
	activity->getEventGroup();
	activity->getEventType();
	activity->getLocalTimestamp();
	activity->getNumSessions();
	activity->getTotalTimerTime();
	activity->getType();
	*/
}

void Fit2TcxConverter::handle_Record(FitMsg_Record *record) {

	// Create new lap if needed
	if (tcxLap == NULL) {
		trackpointList.clear();
		tcxLap = new TcxLap();
		*(tcxActivity) << tcxLap;
		tcxTrack = new TcxTrack();
		*(tcxLap) << tcxTrack;
	}

	string timeId = GpsFunctions::print_dtime(record->getTimestamp());

    TcxTrackpoint * point;
    if ((record->getPositionLat() != FIT_POSITION_INVALID) && (record->getPositionLong() != FIT_POSITION_INVALID)) {
        double dlat = SEMI2DEG(record->getPositionLat());
        double dlon = SEMI2DEG(record->getPositionLong());

        if ((dlat >=-90) && (dlat <=90) && // sanity check
            (dlon >=-180) && (dlon <=180) && // + it is very unlikely that you are actually driving through 0/0
            (((dlat != 0) && (dlon != 0)))) {
            stringstream lat;
            lat.precision(10); // default 4 decimal chars which is not enough
            stringstream lon;
            lon.precision(10); // default 4 decimal chars which is not enough
            lat << dlat;
            lon << dlon;
            point = new TcxTrackpoint(timeId, lat.str(), lon.str());
        } else {
            point = new TcxTrackpoint(timeId);
        }
    } else {
        point = new TcxTrackpoint(timeId);
    }

    *(tcxTrack) << point;
	trackpointList.push_back(point);

    stringstream ss;
    ss << record->getAltitude();
    point->setAltitudeMeters(ss.str());

    ss.str("");
    ss << record->getDistance();
    point->setDistanceMeters(ss.str());

    if ((((int)record->getHeartRate()) > 0) && (((int)record->getHeartRate()) != FIT_HEARTRATE_INVALID))  {
        ss.str("");
        ss << (int)record->getHeartRate();
        point->setHeartRateBpm(ss.str());
    }

	if (((int)record->getCadence()) > 0) {
		ss.str("");
		ss << (int)record->getCadence();
		point->setCadence(ss.str());
	}

	ss.str("");
	ss << record->getSpeed();
	point->setSpeed(ss.str());

	if(((int)record->getPower()) > 0){
		ss.str("");
		ss << record->getPower();
		point->setPower(ss.str());
	}

	/*
	 * There is no place for these in a tcx file
	record->getAccumulatedPower();
	record->getCalories();
	record->getCadence256();
	record->getCombinedPedalSmoothness();
	record->getTemperature();
	*/

}

void Fit2TcxConverter::handle_Lap(FitMsg_Lap *lap) {
	// A new lap comes always after the record data of this lap

    // Create new lap if needed
    if (tcxLap == NULL) {
        trackpointList.clear();
        tcxLap = new TcxLap();
        *(tcxActivity) << tcxLap;
        tcxTrack = new TcxTrack();
        *(tcxLap) << tcxTrack;
    }

	stringstream ss;

	// 999km is a randomly choosen maximum, because I observed values like (1.84467e+17)
	// If DistanceMeters is not set, the tcxLap will calculate it itself before output
	if ((lap->getTotalDistance() >0) && (lap->getTotalDistance() < 999000)) {
		ss << lap->getTotalDistance();
		this->tcxLap->setDistanceMeters(ss.str());
	}

	if ((((int)lap->getAvgHeartRate()) > 0) && (((int)lap->getAvgHeartRate()) != FIT_HEARTRATE_INVALID)) {
		ss.str("");
		ss << (int)lap->getAvgHeartRate();
		this->tcxLap->setAverageHeartRateBpm(ss.str());
	}

	if ((((int)lap->getAvgCadence()) > 0) && (((int)lap->getAvgCadence()) != FIT_CADENCE_INVALID)) {
		ss.str("");
		ss << (int)lap->getAvgCadence();
		this->tcxLap->setCadence(ss.str());
	}

	if ((((int)lap->getMaxCadence()) > 0) && (((int)lap->getMaxCadence()) != FIT_CADENCE_INVALID)) {
		ss.str("");
		ss << (int)lap->getMaxCadence();
		this->tcxLap->setMaxCadence(ss.str());
	}

	if ((lap->getAvgSpeed() > 0) && (lap->getAvgSpeed() != FIT_SPEED_INVALID)) {
		ss.str("");
		ss << lap->getAvgSpeed();
		this->tcxLap->setAvgSpeed(ss.str());
	}

	if ((lap->getAvgPower() > 0) && (lap->getAvgPower() != FIT_POWER_INVALID)) {
		ss.str("");
		ss << lap->getAvgPower();
		this->tcxLap->setAvgPower(ss.str());
	}

	if (((int)lap->getMaxHeartRate() > 0) && ((int)lap->getMaxHeartRate() < FIT_HEARTRATE_INVALID)) {
		ss.str("");
		ss << (int)lap->getMaxHeartRate();
		this->tcxLap->setMaximumHeartRateBpm(ss.str());
	}

	if ((lap->getMaxSpeed() > 0) && (lap->getMaxSpeed() != FIT_SPEED_INVALID)) {
		ss.str("");
		ss << lap->getMaxSpeed();
		this->tcxLap->setMaximumSpeed(ss.str());
	}


	if ((lap->getMaxPower() > 0) && (lap->getAvgPower() != FIT_POWER_INVALID)) {
		ss.str("");
		ss << lap->getMaxPower();
		this->tcxLap->setMaxPower(ss.str());
	}

	if (lap->getTotalCalories() > 0) {
		ss.str("");
		ss << lap->getTotalCalories();
		this->tcxLap->setCalories(ss.str());
	}

	ss.str("");
	ss << lap->getTotalTimerTime();
	this->tcxLap->setTotalTimeSeconds(ss.str());


	switch (lap->getIntensity()) {
		case INTENSITY_REST:
			this->tcxLap->setIntensity(TrainingCenterDatabase::Resting);
			break;
		default:
			this->tcxLap->setIntensity(TrainingCenterDatabase::Active);
			break;
	}

	switch (lap->getLapTrigger()) {
		case LAP_TRIGGER_MANUAL:
			this->tcxLap->setTriggerMethod(TrainingCenterDatabase::Manual);
			break;
		case LAP_TRIGGER_DISTANCE:
			this->tcxLap->setTriggerMethod(TrainingCenterDatabase::Distance);
			break;
		case LAP_TRIGGER_POSITION_START:
		case LAP_TRIGGER_POSITION_MARKED:
		case LAP_TRIGGER_POSITION_LAP:
		case LAP_TRIGGER_POSITION_WAYPOINT:
			this->tcxLap->setTriggerMethod(TrainingCenterDatabase::Location);
			break;
		default:
			break;
	}

	switch (lap->getSport()) {
		case SPORT_CYCLING :
			this->tcxActivity->setSportType(TrainingCenterDatabase::Biking);
			this->tcxLap->setCadenceSensorType(TrainingCenterDatabase::Bike);
			setTrackpointCadenceType(TrainingCenterDatabase::Bike);
			break;
		case SPORT_RUNNING:
			this->tcxActivity->setSportType(TrainingCenterDatabase::Running);
			this->tcxLap->setCadenceSensorType(TrainingCenterDatabase::Footpod);
			setTrackpointCadenceType(TrainingCenterDatabase::Footpod);

		    if ((lap->getTotalCycles() > 0) && (lap->getTotalCycles() != FIT_CYCLES_INVALID)) {
		        ss.str("");
		        ss << (lap->getTotalCycles() * 2);
		        this->tcxLap->setSteps(ss.str());
		    }

			break;
		default:
			break;
	}


	// Next RECORD Entry will create tcxLap again
	this->tcxLap = NULL;
}

void Fit2TcxConverter::handle_File_Creator(FitMsg_File_Creator *filecreator) {
	unsigned short minor = filecreator->getSoftwareVersion() % 100;
	unsigned short major = 0;
	if (filecreator->getSoftwareVersion() > 100) {
		major = (filecreator->getSoftwareVersion() - minor ) / 100;
	}
	stringstream ssMaj;
	stringstream ssMin;
	ssMaj << major;
	ssMin << minor;
	tcxCreator->setVersion(ssMaj.str(), ssMin.str());
}

void Fit2TcxConverter::handle_File_ID(FitMsg_File_ID *fileid) {

	if (fileid->getType() != FIT_FILE_ACTIVITY) {
		string type = "Unknown";
		switch (fileid->getType()) {
			case FIT_FILE_DEVICE: 			type="DEVICE"; break;
			case FIT_FILE_SETTINGS: 		type="SETTINGS"; break;
			case FIT_FILE_SPORT: 			type="SPORT"; break;
			case FIT_FILE_ACTIVITY: 		type="ACTIVITY"; break;
			case FIT_FILE_WORKOUT: 			type="WORKOUT"; break;
			case FIT_FILE_COURSE: 			type="COURSE"; break;
			case FIT_FILE_SCHEDULES: 		type="SCHEDULES"; break;
			case FIT_FILE_WEIGHT: 			type="WEIGHT"; break;
			case FIT_FILE_TOTALS: 			type="TOTALS"; break;
			case FIT_FILE_GOALS: 			type="GOALS"; break;
			case FIT_FILE_BLOOD_PRESSURE: 	type="BLOOD_PRESSURE"; break;
			case FIT_FILE_MONITORING: 		type="MONITORING"; break;
			case FIT_FILE_ACTIVITY_SUMMARY:	type="SUMMARY"; break;
			case FIT_FILE_MONITORING_DAILY:	type="MONITORING_DAILY"; break;
			case FIT_FILE_INVALID: 			type="INVALID"; break;
		}
		FitFileException exc("Wrong FIT file type. Expected ACTIVITY, but found: "+type);
		throw exc;
	}

	string manufacturer="Unknown";
	string product="Unknown";
	if (FIT_MANUFACTURER_GARMIN == fileid->getManufacturer()) {
		manufacturer = "Garmin";
		switch (fileid->getProduct()) {
			case FIT_GARMIN_PRODUCT_HRM1: 		product="HRM1"; break; 			// ??
			case FIT_GARMIN_PRODUCT_AXH01: 		product="AXH01"; break;			// AXH01 HRM chipset
			case FIT_GARMIN_PRODUCT_AXB01: 		product="AXB01"; break; 		// ??
			case FIT_GARMIN_PRODUCT_AXB02: 		product="AXB02"; break;			// ??
			case FIT_GARMIN_PRODUCT_HRM2SS: 	product="HRM2SS"; break;		// ??
			case FIT_GARMIN_PRODUCT_DSI_ALF02: 	product="DSI_ALF02"; break;		// ??
			case FIT_GARMIN_PRODUCT_FR405: 		product="Forerunner 405"; break;
			case FIT_GARMIN_PRODUCT_FR50: 		product="Forerunner 50"; break;
			case FIT_GARMIN_PRODUCT_FR60: 		product="Forerunner 60"; break;
			case FIT_GARMIN_PRODUCT_DSI_ALF01: 	product="DSI_ALF01"; break;		// ??
			case FIT_GARMIN_PRODUCT_FR310XT: 	product="Forerunner 310xt"; break;
			case FIT_GARMIN_PRODUCT_EDGE500: 	product="Edge 500"; break;
			case FIT_GARMIN_PRODUCT_FR110: 		product="Forerunner 110"; break;
			case FIT_GARMIN_PRODUCT_EDGE800: 	product="Edge 800"; break;
			case FIT_GARMIN_PRODUCT_CHIRP: 		product="CHIRP"; break;			// ??
			case FIT_GARMIN_PRODUCT_EDGE200: 	product="Edge 200"; break;
			case FIT_GARMIN_PRODUCT_FR910XT: 	product="Forerunner 910XT"; break;
			case FIT_GARMIN_PRODUCT_ALF04: 		product="ALF04"; break;			// ??
			case FIT_GARMIN_PRODUCT_FR610: 		product="Forerunner 610"; break;
			case FIT_GARMIN_PRODUCT_FR70: 		product="Forerunner 70"; break;
			case FIT_GARMIN_PRODUCT_FR310XT_4T:	product="Forerunner 310xt_4t"; break;
			case FIT_GARMIN_PRODUCT_AMX: 		product="AMX"; break;			// ??
			case FIT_GARMIN_PRODUCT_SDM4: 		product="SDM4 footpod"; break;
			case FIT_GARMIN_PRODUCT_TRAINING_CENTER: product="Training Center"; break;
			case FIT_GARMIN_PRODUCT_CONNECT: 	product="Connect website"; break;
			default:
				break;
		}
	}
	if (manufacturer.compare("Unknown")==0) {
		tcxCreator->setName(product);
	} else {
		tcxCreator->setName(manufacturer+" "+product);
	}

	stringstream ss;
	ss << fileid->getSerialNumber();
	tcxCreator->setUnitId(ss.str());

	ss.str("");
	ss << fileid->getProduct();
	tcxCreator->setProductId(ss.str());


	/*
	 * Unknown where to place...
	 fileid->getNumber();
	 fileid->getTimeCreated();
	 */
}

TiXmlDocument * Fit2TcxConverter::getTiXmlDocument(bool readTrackData, string fitnessDetailId) {
	tcxAuthor->setName("Fit2Tcx");
	TiXmlDocument * output = this->tcxBase->getTcxDocument(readTrackData, fitnessDetailId);
	return output;
}

string Fit2TcxConverter::getTcxContent(bool readTrackData, string fitnessDetailId) {
	tcxAuthor->setName("Fit2Tcx");

	TiXmlDocument * output = this->tcxBase->getTcxDocument(readTrackData, fitnessDetailId);
    TiXmlPrinter printer;
    printer.SetIndent( "  " );
    output->Accept( &printer );
    string fitnessXml = printer.Str();
    delete(output);
    return fitnessXml;

}

void Fit2TcxConverter::setTrackpointCadenceType(TrainingCenterDatabase::CadenceSensorType_t type) {
    vector<TcxTrackpoint*>::iterator it;
    for ( it=trackpointList.begin() ; it < trackpointList.end(); ++it )
    {
        TcxTrackpoint* trackpoint = *it;
        trackpoint->setCadenceSensorType(type);
    }
}

void Fit2TcxConverter::handle_Session(FitMsg_Session *session) {
	switch (session->getSport()) {
		case SPORT_CYCLING :
			this->tcxActivity->setSportType(TrainingCenterDatabase::Biking);
			break;
		case SPORT_RUNNING:
			this->tcxActivity->setSportType(TrainingCenterDatabase::Running);
			break;
		default:
			this->tcxActivity->setSportType(TrainingCenterDatabase::Other);
			break;
	}

	this->tcxActivity->setId(GpsFunctions::print_dtime(session->getStartTime()));
}

void Fit2TcxConverter::handle_DeviceInfo(FitMsg_DeviceInfo *deviceInfo) {
	/*
	std::cout << "DEVICE INFO RECEIVED " << std::endl;
	std::cout << "Hardware Version : " << (int)deviceInfo->getHardwareVersion() << endl;
	std::cout << "Serial Number: " << deviceInfo->getSerialNumber() << endl;
	std::cout << "Software Version : " << deviceInfo->getSoftwareVersion() << endl;
	 */
}
