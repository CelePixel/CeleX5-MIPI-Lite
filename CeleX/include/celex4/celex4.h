/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef CELEX_H
#define CELEX_H

#ifdef _WIN32
#ifdef CELEX_API_EXPORTS
#define CELEX_EXPORTS __declspec(dllexport)
#else
#define CELEX_EXPORTS __declspec(dllimport)
#endif
#else
#if defined(CELEX_LIBRARY)
#define CELEX_EXPORTS
#else
#define CELEX_EXPORTS
#endif
#endif

#include <stdint.h>
#include <vector>
#include <map>
#include <list>
#include "../celextypes.h"

class FrontPanel;
class HHSequenceMgr;
class HHSequenceSlider;
class CELEX_EXPORTS CeleX4
{
public:
	enum CeleX4Mode {
		Unknown_Mode = -1,
		Full_Picture_Mode = 0,
		Event_Mode = 1,
		FullPic_Event_Mode = 2
	};

	enum emDeviceType {
		Sensor = 0,
		FPGA,
		SensorAndFPGA
	};

	enum emAdvancedBiasType {
		EVT_VL = 0,
		EVT_VH,
		ZONE_MINUS,
		ZONE_PLUS,
		REF_MINUS,
		REF_PLUS,
		REF_MINUS_H,
		REF_PLUS_H,
		EVT_DC,
		LEAK,
		CDS_DC,
		CDS_V1,
		PixBias,
		Gain,
		Clock,
		Resolution
	};

	enum ErrorCode {
		NoError = 0,
		InitializeFPGAFailed,
		PowerUpFailed,
		ConfigureFailed
	};
	CeleX4();
	~CeleX4();

	ErrorCode openSensor();
	bool isSensorReady();
	bool isSdramFull();

	//--- Sensor Operating Mode Interfaces ---
	void setSensorMode(CeleX4Mode mode);
	CeleX4Mode getSensorMode();

	long getFPGADataSize();
	long readDataFromFPGA(long length, unsigned char *data);

	void setThreshold(uint32_t value);
	uint32_t getThreshold();

	void setContrast(uint32_t value);
	uint32_t getContrast();

	void setBrightness(uint32_t value);
	uint32_t getBrightness();

	//--- Sensor Control Interfaces ---
	void resetFPGA();
	void resetSensorAndFPGA();
	void enableADC(bool enable);

	void trigFullPic();

	//--- for clock ---
	uint32_t getClockRate(); //unit: MHz
	void setClockRate(uint32_t value); //unit: MHz

	//--- Set the methods of creating pic frame Interfaces ---
	void setFullPicFrameTime(uint32_t msec); //unit: millisecond
	uint32_t getFullPicFrameTime();
	void setFEFrameTime(uint32_t msec); //unit: millisecond
	uint32_t getFEFrameTime();

	void setIMUIntervalTime(uint32_t value);

private:
	bool powerUp();
	bool configureSettings();
	void parseSliderSequence();
	bool excuteCommand(std::string strCommand);
	bool setAdvancedBias(std::string strBiasName);
	bool setAdvancedBias(std::string strBiasName, int value);

private:
	std::map<std::string, uint32_t>  m_mapSliderNameValue; //All Setting Names & Initial Values
	std::vector<std::string>         m_vecAdvancedNames;   //Advanced Setting Names

	FrontPanel*                      m_pFrontPanel;
	HHSequenceMgr*                   m_pSequenceMgr;

	CeleX4Mode                       m_emSensorMode;
	uint32_t                         m_uiFullPicFrameTime;
	uint32_t                         m_uiEventFrameTime;
	uint32_t                         m_uiFEFrameTime;
	uint32_t                         m_uiClockRate;
};

#endif // CELEX_H
