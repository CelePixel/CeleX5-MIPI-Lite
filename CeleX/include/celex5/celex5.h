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

#ifndef CELEX5_H
#define CELEX5_H

#include <stdint.h>
#include <vector>
#include <map>
#include "../celextypes.h"

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

using namespace std;

class CeleDriver;
class HHSequenceMgr;
class CommandBase;
class CELEX_EXPORTS CeleX5
{
public:
	enum CeleX5Mode {
		Unknown_Mode = -1,
		Event_Address_Only_Mode = 0,
		Event_Optical_Flow_Mode = 1,
		Event_Intensity_Mode = 2,
		Full_Picture_Mode = 3,
		Full_Optical_Flow_S_Mode = 4,
		Full_Optical_Flow_M_Mode = 6,
	};

	typedef struct CfgInfo
	{
		std::string name;
		uint32_t    min;
		uint32_t    max;
		uint32_t    value;
		uint32_t    step;
		int16_t     high_addr;
		int16_t     middle_addr;
		int16_t     low_addr;
	} CfgInfo;

	CeleX5();
	~CeleX5();

	bool openSensor();
	bool getMIPIData(vector<uint8_t> &buffer);

	void setSensorFixedMode(CeleX5Mode mode);
	CeleX5Mode getSensorFixedMode();

	void setSensorLoopMode(CeleX5Mode mode, int loopNum); //LopNum = 1/2/3
	CeleX5Mode getSensorLoopMode(int loopNum); //LopNum = 1/2/3
	void setLoopModeEnabled(bool enable);
	bool isLoopModeEnabled();

	//------- for loop mode -------
	void setEventDuration(uint32_t value);
	void setPictureNumber(uint32_t num, CeleX5Mode mode);

	//------- sensor control interfaces -------
	void setThreshold(uint32_t value);
	uint32_t getThreshold();
	void setContrast(uint32_t value);
	uint32_t getContrast();
	void setBrightness(uint32_t value);
	uint32_t getBrightness();
	uint32_t getClockRate(); //unit: MHz
	void setClockRate(uint32_t value); //range: 20 ~ 100, step: 10, unit: MHz

	void setAutoISPEnabled(bool enable);
	bool isAutoISPEnabled();
	void setISPThreshold(uint32_t value, int num);
	void setISPBrightness(uint32_t value, int num);

	map<string, vector<CfgInfo>> getCeleX5Cfg();
	void writeRegister(int16_t addressH, int16_t addressM, int16_t addressL, uint32_t value);
	void writeCSRDefaults(string csrType);

private:
	bool configureSettings();
	//for write register
	void wireIn(uint32_t address, uint32_t value, uint32_t mask);
	void writeRegister(CfgInfo cfgInfo);
	void setALSEnabled(bool enable);
	void enterCFGMode();
	void enterStartMode();
	void disableMIPI();
	void enableMIPI();

private:
	CeleDriver*                    m_pCeleDriver;

	HHSequenceMgr*                 m_pSequenceMgr;
	//
	map<string, vector<CfgInfo>>   m_mapCfgDefaults;

	CeleX5Mode                     m_emSensorFixedMode;
	CeleX5Mode                     m_emSensorLoopMode[3];

	uint32_t                       m_uiContrast;
	uint32_t                       m_uiBrightness;
	uint32_t                       m_uiThreshold;
	uint32_t                       m_uiClockRate;

	bool                           m_bLoopModeEnabled;
	bool                           m_bAutoISPEnabled;

	uint32_t                       m_arrayISPThreshold[3];
	uint32_t                       m_arrayBrightness[4];
	uint32_t                       m_uiAutoISPRefreshTime;
};

#endif // CELEX5_H
