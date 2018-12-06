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

#include "../include/celex5/celex5.h"
#include "../frontpanel/frontpanel.h"
#include "../driver/CeleDriver.h"
#include "../configproc/hhsequencemgr.h"
#include "../configproc/hhwireincommand.h"
#include "../base/xbase.h"
#include <cstring>

CeleX5::CeleX5() 
	: m_bLoopModeEnabled(false)
	, m_emSensorFixedMode(CeleX5::Event_Address_Only_Mode)
	, m_emSensorLoopMode{ CeleX5::Full_Picture_Mode, CeleX5::Event_Address_Only_Mode, CeleX5::Full_Optical_Flow_S_Mode }
	, m_uiClockRate(100)
	, m_pCeleDriver(NULL)
	, m_bAutoISPEnabled(false)
	, m_arrayISPThreshold{60, 500, 2500}
	, m_arrayBrightness{100, 130, 150, 175}
	, m_uiAutoISPRefreshTime(80)
{
	m_pSequenceMgr = new HHSequenceMgr;
	m_pSequenceMgr->parseCeleX5Cfg(FILE_CELEX5_CFG);
	m_mapCfgDefaults = getCeleX5Cfg();
}

CeleX5::~CeleX5()
{
	if (m_pCeleDriver)
	{
		m_pCeleDriver->clearData();
		m_pCeleDriver->Close();
		delete m_pCeleDriver;
		m_pCeleDriver = NULL;
	}
	if (m_pSequenceMgr)
	{
		delete m_pSequenceMgr;
		m_pSequenceMgr = NULL;
	}
}

bool CeleX5::openSensor()
{
	if (NULL == m_pCeleDriver)
	{
		m_pCeleDriver = new CeleDriver;
		if (!m_pCeleDriver->openUSB())
			return false;
	}
	if (!configureSettings())
		return false;
	return true;
}

bool CeleX5::getMIPIData(vector<uint8_t> &buffer)
{
	m_pCeleDriver->getimage(buffer);
	if (buffer.size() > 0)
	{
		return true;
	}	
	return false;
}

// Set the Sensor operation mode in fixed mode
// address = 53, width = [2:0]
void CeleX5::setSensorFixedMode(CeleX5Mode mode)
{
	m_pCeleDriver->clearData();
	//Disable ALS read and write, must be the first operation
	setALSEnabled(false);

	enterCFGMode();
	//Write Fixed Sensor Mode
	wireIn(53, static_cast<uint32_t>(mode), 0xFF);

	//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
	wireIn(221, 0, 0xFF); //AUTOISP_BRT_EN
	wireIn(223, 0, 0xFF); //AUTOISP_TRIGGER
	wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
	writeRegister(233, -1, 232, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
	writeRegister(22, -1, 23, 140); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0
	enterStartMode();
	m_emSensorFixedMode = mode;
}

// Set the Sensor operation mode in loop mode
// loop = 1: the first operation mode in loop mode, address = 53, width = [2:0]
// loop = 2: the second operation mode in loop mode, address = 54, width = [2:0]
// loop = 3: the third operation mode in loop mode, address = 55, width = [2:0]
void CeleX5::setSensorLoopMode(CeleX5Mode mode, int loopNum)
{
	m_pCeleDriver->clearData();
	if (loopNum < 1 || loopNum > 3)
	{
		cout << "CeleX5::setSensorMode: wrong loop number!";
		return;
	}
	enterCFGMode();
	wireIn(52 + loopNum, static_cast<uint32_t>(mode), 0xFF);
	enterStartMode();
}

CeleX5::CeleX5Mode CeleX5::getSensorFixedMode()
{
	return m_emSensorFixedMode;
}

CeleX5::CeleX5Mode CeleX5::getSensorLoopMode(int loopNum)
{
	if (loopNum > 0 & loopNum < 4)
	{
		return m_emSensorLoopMode[loopNum - 1];
	}
	return CeleX5::Unknown_Mode;
}

// enable or disable loop mode
// address = 64, width = [0], 0: fixed mode / 1: loop mode
void CeleX5::setLoopModeEnabled(bool enable)
{
	m_bLoopModeEnabled = enable;

	if (m_bAutoISPEnabled)
		setALSEnabled(false);

	enterCFGMode();
	if (enable)
	{
		wireIn(64, 1, 0xFF);
		//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
		wireIn(221, 0, 0xFF); //AUTOISP_BRT_EN, disable auto isp
		wireIn(223, 1, 0xFF); //AUTOISP_TRIGGER
		wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
		writeRegister(233, -1, 232, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
		writeRegister(22, -1, 23, 140); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0
	}
	else
	{
		wireIn(64, 0, 0xFF);
	}
	enterStartMode();
}

bool CeleX5::isLoopModeEnabled()
{
	return m_bLoopModeEnabled;
}

// Set the duration of event mode (Mode_A/B/C) when sensor operates in loop mode
// low byte address = 57, width = [7:0]
// high byte address = 58, width = [1:0]
void CeleX5::setEventDuration(uint32_t value)
{
	enterCFGMode();

	uint32_t valueH = value >> 8;
	uint32_t valueL = 0xFF & value;
	wireIn(58, valueH, 0xFF);
	wireIn(57, valueL, 0xFF);

	enterStartMode();
}

// Set the mumber of pictures to acquire in Mode_D/E/F/G/H
// Mode_D: address = 59, width = [7:0]
// Mode_E: address = 60, width = [7:0]
// Mode_F: address = 61, width = [7:0]
// Mode_G: address = 62, width = [7:0]
// Mode_H: address = 63, width = [7:0]
void CeleX5::setPictureNumber(uint32_t num, CeleX5Mode mode)
{
	enterCFGMode();

	if (Full_Picture_Mode == mode)
		wireIn(59, num, 0xFF);
	else if (Full_Optical_Flow_S_Mode == mode)
		wireIn(60, num, 0xFF);
	else if (Full_Optical_Flow_M_Mode == mode)
		wireIn(62, num, 0xFF);

	enterStartMode();
}

// BIAS_EVT_VL : 341 address(2/3)
// BIAS_EVT_DC : 512 address(4/5)
// BIAS_EVT_VH : 683 address(6/7)
void CeleX5::setThreshold(uint32_t value)
{
	m_uiThreshold = value;

	enterCFGMode();

	int EVT_VL = 512 - value;
	if (EVT_VL < 0)
		EVT_VL = 0;
	writeRegister(2, -1, 3, EVT_VL);

	int EVT_VH = 512 + value;
	if (EVT_VH > 1023)
		EVT_VH = 1023;
	writeRegister(6, -1, 7, EVT_VH);

	enterStartMode();
}

uint32_t CeleX5::getThreshold()
{
	return m_uiThreshold;
}

// Set Contrast
// COL_GAIN: address = 45, width = [1:0]
void CeleX5::setContrast(uint32_t value)
{
	m_uiContrast = value;
	if (value < 1)
		m_uiContrast = 1;
	else if (value > 3)
		m_uiContrast = 3;
	enterCFGMode();
	writeRegister(45, -1, -1, m_uiContrast);
	enterStartMode();
}

uint32_t CeleX5::getContrast()
{
	return m_uiContrast;
}

// Set brightness
// <BIAS_BRT_I>
// high byte address = 22, width = [1:0]
// low byte address = 23, width = [7:0]
void CeleX5::setBrightness(uint32_t value)
{
	m_uiBrightness = value;

	enterCFGMode();
	writeRegister(22, -1, 23, value);
	enterStartMode();
}

uint32_t CeleX5::getBrightness()
{
	return m_uiBrightness;
}

uint32_t CeleX5::getClockRate()
{
	return m_uiClockRate;
}

void CeleX5::setClockRate(uint32_t value)
{
	if (value > 100 || value < 20)
		return;
	m_uiClockRate = value;
	enterCFGMode();

	// Disable PLL
	wireIn(150, 0, 0xFF);
	int clock[15] = { 20,  30,  40,  50,  60,  70,  80,  90, 100, 110, 120, 130, 140, 150, 160 };

	int PLL_DIV_N[15] = { 12,  18,  12,  15,  18,  21,  12,  18,  15,  22,  18,  26,  21,  30,  24 };
	int PLL_DIV_L[15] = { 2,   3,   2,   2,   2,   2,   2,   3,   2,   3,   2,   3,   2,   3,   2 };
	int PLL_FOUT_DIV1[15] = { 3,   2,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0 };
	int PLL_FOUT_DIV2[15] = { 3,   2,   3,   3,   3,   3,   3,   2,   3,   3,   3,   3,   3,   3,   3 };

	int MIPI_PLL_DIV_I[15] = { 3,   2,   3,   3,   2,   2,   3,   2,   3,   2,   2,   2,   2,   2,   1 };
	//int MIPI_PLL_DIV_N[15] = { 120, 120, 120, 96, 120, 102, 120, 120, 96, 130, 120, 110, 102, 96, 120 };
	//int MIPI_PLL_DIV_N[15] = { 100, 120, 100, 96, 120, 102, 100, 120, 96, 130, 120, 110, 102, 96, 120 };
	int MIPI_PLL_DIV_N[15] = { 180, 180, 180, 144, 180, 153, 180, 180, 144, 195, 180, 165, 153, 144, 180 };

	int index = value / 10 - 2;

	cout << "CeleX5::setClockRate: " << clock[index] << " MHz" << endl;

	// Write PLL Clock Parameter
	writeRegister(159, -1, -1, PLL_DIV_N[index]);
	writeRegister(160, -1, -1, PLL_DIV_L[index]);
	writeRegister(151, -1, -1, PLL_FOUT_DIV1[index]);
	writeRegister(152, -1, -1, PLL_FOUT_DIV2[index]);

	// Enable PLL
	wireIn(150, 1, 0xFF);

	disableMIPI();
	writeRegister(113, -1, -1, MIPI_PLL_DIV_I[index]);
	writeRegister(115, -1, 114, MIPI_PLL_DIV_N[index]);
	enableMIPI();

	enterStartMode();
}

map<string, vector<CeleX5::CfgInfo>> CeleX5::getCeleX5Cfg()
{
	map<string, vector<HHCommandBase*>> mapCfg = m_pSequenceMgr->getCeleX5Cfg();
	//
	map<string, vector<CeleX5::CfgInfo>> mapCfg1;
	for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
	{
		//cout << "CelexSensorDLL::getCeleX5Cfg: " << itr->first << endl;
		vector<HHCommandBase*> pCmdList = itr->second;
		vector<CeleX5::CfgInfo> vecCfg;
		for (auto itr1 = pCmdList.begin(); itr1 != pCmdList.end(); itr1++)
		{
			WireinCommandEx* pCmd = (WireinCommandEx*)(*itr1);
			//cout << "----- Register Name: " << pCmd->name() << endl;
			CeleX5::CfgInfo cfgInfo;
			cfgInfo.name = pCmd->name();
			cfgInfo.min = pCmd->minValue();
			cfgInfo.max = pCmd->maxValue();
			cfgInfo.value = pCmd->value();
			cfgInfo.high_addr = pCmd->highAddr();
			cfgInfo.middle_addr = pCmd->middleAddr();
			cfgInfo.low_addr = pCmd->lowAddr();
			vecCfg.push_back(cfgInfo);
		}
		mapCfg1[itr->first] = vecCfg;
	}
	return mapCfg1;
}

void CeleX5::writeRegister(int16_t addressH, int16_t addressM, int16_t addressL, uint32_t value)
{
	if (addressL == -1)
	{
		wireIn(addressH, value, 0xFF);
	}
	else
	{
		if (addressM == -1)
		{
			uint32_t valueH = value >> 8;
			uint32_t valueL = 0xFF & value;
			wireIn(addressH, valueH, 0xFF);
			wireIn(addressL, valueL, 0xFF);
		}
	}
}

void CeleX5::writeCSRDefaults(string csrType)
{
	cout << "CeleX5::writeCSRDefaults: " << csrType << endl;
	for (auto itr = m_mapCfgDefaults.begin(); itr != m_mapCfgDefaults.end(); itr++)
	{
		//cout << "group name: " << itr->first << endl;
		string tapName = itr->first;
		if (csrType == tapName)
		{
			vector<CeleX5::CfgInfo> vecCfg = itr->second;
			for (auto itr1 = vecCfg.begin(); itr1 != vecCfg.end(); itr1++)
			{
				CeleX5::CfgInfo cfgInfo = (*itr1);
				writeRegister(cfgInfo);
			}
			break;
		}
	}
}

bool CeleX5::configureSettings()
{
	setALSEnabled(false);
	if (m_pCeleDriver)
		m_pCeleDriver->openStream();

	//--------------- Step1 ---------------
	wireIn(94, 0, 0xFF); //PADDR_EN

						 //--------------- Step2: Load PLL Parameters ---------------
						 //Disable PLL
	cout << "--- Disable PLL ---" << endl;
	wireIn(150, 0, 0xFF); //PLL_PD_B
						  //Load PLL Parameters
	cout << endl << "--- Load PLL Parameters ---" << endl;
	writeCSRDefaults("PLL_Parameters");
	//Enable PLL
	cout << "--- Enable PLL ---" << endl;
	wireIn(150, 1, 0xFF); //PLL_PD_B

						  //--------------- Step3: Load MIPI Parameters ---------------
	cout << endl << "--- Disable MIPI ---" << endl;
	disableMIPI();

	cout << endl << "--- Load MIPI Parameters ---" << endl;
	writeCSRDefaults("MIPI_Parameters");
	writeRegister(115, -1, 114, 120); //MIPI_PLL_DIV_N

									  //Enable MIPI
	cout << endl << "--- Enable MIPI ---" << endl;
	enableMIPI();

	//--------------- Step4: ---------------
	cout << endl << "--- Enter CFG Mode ---" << endl;
	enterCFGMode();

	//----- Load Sensor Core Parameters -----
	wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR
	writeCSRDefaults("Sensor_Core_Parameters");
	//if (m_bAutoISPEnabled)
	{
		//--------------- for auto isp --------------- 
		wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR
							  //Load Sensor Core Parameters
		writeCSRDefaults("Sensor_Core_Parameters");
		writeRegister(22, -1, 23, 140);

		wireIn(220, 1, 0xFF); //AUTOISP_PROFILE_ADDR
							  //Load Sensor Core Parameters
		writeCSRDefaults("Sensor_Core_Parameters");
		writeRegister(22, -1, 23, m_arrayBrightness[1]);

		wireIn(220, 2, 0xFF); //AUTOISP_PROFILE_ADDR
							  //Load Sensor Core Parameters
		writeCSRDefaults("Sensor_Core_Parameters");
		writeRegister(22, -1, 23, m_arrayBrightness[2]);

		wireIn(220, 3, 0xFF); //AUTOISP_PROFILE_ADDR
							  //Load Sensor Core Parameters
		writeCSRDefaults("Sensor_Core_Parameters");
		writeRegister(22, -1, 23, m_arrayBrightness[3]);

		wireIn(221, 0, 0xFF); //AUTOISP_BRT_EN, disable auto ISP
		wireIn(222, 0, 0xFF); //AUTOISP_TEM_EN
		wireIn(223, 0, 0xFF); //AUTOISP_TRIGGER

		writeRegister(225, -1, 224, m_uiAutoISPRefreshTime); //AUTOISP_REFRESH_TIME

		writeRegister(235, -1, 234, m_arrayISPThreshold[0]); //AUTOISP_BRT_THRES1
		writeRegister(237, -1, 236, m_arrayISPThreshold[1]); //AUTOISP_BRT_THRES2
		writeRegister(239, -1, 238, m_arrayISPThreshold[2]); //AUTOISP_BRT_THRES3

		writeRegister(233, -1, 232, 1500); //AUTOISP_BRT_VALUE
	}
	writeCSRDefaults("Sensor_Operation_Mode_Control_Parameters");
	writeCSRDefaults("Sensor_Data_Transfer_Parameters");
	cout << endl << "--- Enter Start Mode ---" << endl;
	enterStartMode();

	return true;
}

void CeleX5::wireIn(uint32_t address, uint32_t value, uint32_t mask)
{
	if (m_pCeleDriver)
	{
		if (isAutoISPEnabled())
		{
			setALSEnabled(false);
#ifdef _WIN32
			Sleep(2);
#else
			usleep(1000 * 2);
#endif
		}
		if (m_pCeleDriver->i2c_set(address, value))
		{
			//cout << "CeleX5::wireIn(i2c_set): address = " << address << ", value = " << value << endl;
		}
		if (isAutoISPEnabled())
		{
			setALSEnabled(true);
		}
	}
}

void CeleX5::writeRegister(CfgInfo cfgInfo)
{
	if (cfgInfo.low_addr == -1)
	{
		wireIn(cfgInfo.high_addr, cfgInfo.value, 0xFF);
	}
	else
	{
		if (cfgInfo.middle_addr == -1)
		{
			uint32_t valueH = cfgInfo.value >> 8;
			uint32_t valueL = 0xFF & cfgInfo.value;
			wireIn(cfgInfo.high_addr, valueH, 0xFF);
			wireIn(cfgInfo.low_addr, valueL, 0xFF);
		}
	}
}

void CeleX5::setAutoISPEnabled(bool enable)
{
	m_bAutoISPEnabled = enable;
	if (enable)
	{
		enterCFGMode();

		wireIn(221, 1, 0xFF); //AUTOISP_BRT_EN, enable auto ISP
		if (isLoopModeEnabled())
			wireIn(223, 1, 0xFF); //AUTOISP_TRIGGER
		else
			wireIn(223, 0, 0xFF); //AUTOISP_TRIGGER

		wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
		writeRegister(233, -1, 232, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
		writeRegister(22, -1, 23, 80); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0		

		enterStartMode();

		setALSEnabled(true);
	}
	else
	{
		setALSEnabled(false); //Disable ALS read and write

		enterCFGMode();

		//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
		wireIn(221, 0, 0xFF); //AUTOISP_BRT_EN, disable auto ISP
		if (isLoopModeEnabled())
			wireIn(223, 1, 0xFF); //AUTOISP_TRIGGER
		else
			wireIn(223, 0, 0xFF); //AUTOISP_TRIGGER

		wireIn(220, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
		writeRegister(233, -1, 232, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
		writeRegister(22, -1, 23, 140); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0
		
		enterStartMode();
	}
}

bool CeleX5::isAutoISPEnabled()
{
	return m_bAutoISPEnabled;
}

void CeleX5::setALSEnabled(bool enable)
{
	if (enable)
		m_pCeleDriver->i2c_set(254, 0);
	else
		m_pCeleDriver->i2c_set(254, 2);
}

void CeleX5::setISPThreshold(uint32_t value, int num)
{
	m_arrayISPThreshold[num - 1] = value;
	if (num == 1)
		writeRegister(235, -1, 234, m_arrayISPThreshold[0]); //AUTOISP_BRT_THRES1
	else if (num == 2)
		writeRegister(237, -1, 236, m_arrayISPThreshold[1]); //AUTOISP_BRT_THRES2
	else if (num == 3)
		writeRegister(239, -1, 238, m_arrayISPThreshold[2]); //AUTOISP_BRT_THRES3
}

void CeleX5::setISPBrightness(uint32_t value, int num)
{
	m_arrayBrightness[num - 1] = value;
	wireIn(220, num - 1, 0xFF); //AUTOISP_PROFILE_ADDR
	writeRegister(22, -1, 23, m_arrayBrightness[num - 1]);
}

//Enter CFG Mode
void CeleX5::enterCFGMode()
{
	wireIn(93, 0, 0xFF);
	wireIn(90, 1, 0xFF);
}

//Enter Start Mode
void CeleX5::enterStartMode()
{
	wireIn(90, 0, 0xFF);
	wireIn(93, 1, 0xFF);
}

void CeleX5::disableMIPI()
{
	wireIn(139, 0, 0xFF);
	wireIn(140, 0, 0xFF);
	wireIn(141, 0, 0xFF);
	wireIn(142, 0, 0xFF);
	wireIn(143, 0, 0xFF);
	wireIn(144, 0, 0xFF);
}

void CeleX5::enableMIPI()
{
	wireIn(139, 1, 0xFF);
	wireIn(140, 1, 0xFF);
	wireIn(141, 1, 0xFF);
	wireIn(142, 1, 0xFF);
	wireIn(143, 1, 0xFF);
	wireIn(144, 1, 0xFF);
}