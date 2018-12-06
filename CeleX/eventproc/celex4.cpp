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

#include <cstring>
#include "../include/celex4/celex4.h"
#include "../frontpanel/frontpanel.h"
#include "../configproc/hhsequencemgr.h"
#include "../base/xbase.h"

// This is the constructor of a class that has been exported.
// see celex4.h for the class definition
CeleX4::CeleX4()
	: m_emSensorMode(CeleX4::Event_Mode)
	, m_uiFullPicFrameTime(20)
	, m_uiEventFrameTime(60)
	, m_uiFEFrameTime(60)
	, m_uiClockRate(25)
{
	m_pFrontPanel = FrontPanel::getInstance();

	m_pSequenceMgr = new HHSequenceMgr;
	m_pSequenceMgr->parseCommandList();
	m_pSequenceMgr->parseSequenceList();
	m_pSequenceMgr->parseSliderList();

	parseSliderSequence();
}

CeleX4::~CeleX4()
{
	if (m_pSequenceMgr)
	{
		delete m_pSequenceMgr;
	}
	if (m_pFrontPanel)
	{
		m_pFrontPanel->uninitializeFPGA();
		delete m_pFrontPanel;
	}
}

CeleX4::ErrorCode CeleX4::openSensor()
{
	m_pFrontPanel->initializeFPGA("top.bit");
	if (isSensorReady())
	{
		if (!powerUp())
			return CeleX4::PowerUpFailed;
		if (!configureSettings())
			return CeleX4::ConfigureFailed;

		setFEFrameTime(60);
		setFullPicFrameTime(40);
	}
	else
	{
		return CeleX4::InitializeFPGAFailed;
	}
	return CeleX4::NoError;
}

// Execute Sensor Power-up Sequence
bool CeleX4::powerUp()
{
	bool bSuccess = false;
	std::string sPowerUp = "Power Up";
	if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(sPowerUp))
	{
		if (!pUpSeq->fire())
			cout << "Power Up failed." << endl;
		else
			bSuccess = true;
	}
	else
	{
		cout << sPowerUp << ": Sequence not defined." << endl;
	}
	return bSuccess;
}

bool CeleX4::configureSettings()
{
	bool bOk = false;
	std::string settingNames;
	if (m_pFrontPanel->isReady())
	{
		bOk = true;
		int index = 0;
		for (vector<string>::iterator itr = m_vecAdvancedNames.begin(); itr != m_vecAdvancedNames.end(); itr++)
		{
			bool ret = setAdvancedBias(m_vecAdvancedNames.at(index));
			if (!ret)
				settingNames += (" " + (*itr));
			bOk = (bOk && ret);
			++index;
		}
	}
	if (bOk)
		cout << "Configure Advanced Settings Successfully!" << endl;
	else
		cout << "Configure Advanced Settings Failed @" << settingNames << endl;
	return bOk;
}

bool CeleX4::isSensorReady()
{
	return m_pFrontPanel->isReady();
}

bool CeleX4::isSdramFull()
{
	uint32_t sdramFull;
	FrontPanel::getInstance()->wireOut(0x20, 0x0001, &sdramFull);
	if (sdramFull > 0)
	{
		cout << "---- SDRAM is full! -----" << endl;
		return true;
	}
	return false;
}

long CeleX4::getFPGADataSize()
{
	if (!isSensorReady())
	{
		return -1;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	//cout << "----------- pageCount = " << pageCount << endl; 
	return 128 * pageCount;
}

long CeleX4::readDataFromFPGA(long length, unsigned char *data)
{
	if (!data)
		return -1;
	if (!isSensorReady())
	{
		return -1;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	//cout << "----------- pageCount = " << pageCount << endl;
	//Return the number of bytes read or ErrorCode (<0) if the read failed. 
	long dataLen = m_pFrontPanel->blockPipeOut(0xa0, 128, length, data);
	if (dataLen > 0)
	{
		return dataLen;
	}
	else if (dataLen < 0) //read failed
	{
		switch (dataLen)
		{
		case okCFrontPanel::InvalidBlockSize:
			cout << "Block Size Not Supported: " << dataLen << endl;
			break;

		case okCFrontPanel::UnsupportedFeature:
			cout << "Unsupported Feature: " << dataLen << endl;
			break;

		default:
			cout << "Transfer Failed with error: " << dataLen << endl;
			break;
		}
	}
	return -1;
}

// Execute Sensor "Event Mode"/"Full Picture" Sequence
void CeleX4::setSensorMode(CeleX4::CeleX4Mode mode)
{
	if (!m_pFrontPanel->isReady())
		return;

	if (mode == CeleX4::Event_Mode)
	{
		excuteCommand("Full Picture");
	}
	else if (mode == CeleX4::Full_Picture_Mode)
	{
		excuteCommand("Event Mode");
	}
	else //AC or AB or ABC Mode
	{
		excuteCommand("Optical Mode");
	}

	m_emSensorMode = mode;
}

CeleX4::CeleX4Mode CeleX4::getSensorMode()
{
	return m_emSensorMode;
}

void CeleX4::resetFPGA()
{
	excuteCommand("Reset-Dereset FPGA");
}

void CeleX4::resetSensorAndFPGA()
{
	excuteCommand("Reset-Dereset All");
}

void CeleX4::enableADC(bool enable)
{
	if (enable)
		excuteCommand("ADC Disable");
	else
		excuteCommand("ADC Enalbe");
}

void CeleX4::setContrast(uint32_t value)
{
	uint32_t uiREF_PLUS = 512 + value / 2;
	uint32_t uiREF_MINUS = 512 - value / 2;
	uint32_t uiREF_PLUS_H = uiREF_PLUS + value / 16;
	uint32_t uiREF_MINUS_H = uiREF_MINUS - value / 16;

	m_mapSliderNameValue["REF+"] = uiREF_PLUS;
	m_mapSliderNameValue["REF-"] = uiREF_MINUS;
	m_mapSliderNameValue["REF+H"] = uiREF_PLUS_H;
	m_mapSliderNameValue["REF-H"] = uiREF_MINUS_H;

	setAdvancedBias("REF+", uiREF_PLUS);
	setAdvancedBias("REF-", uiREF_MINUS);
	setAdvancedBias("REF+H", uiREF_PLUS_H);
	setAdvancedBias("REF-H", uiREF_MINUS_H);
}

uint32_t CeleX4::getContrast()
{
	return (m_mapSliderNameValue["REF+"] - 512) * 2;
}

void CeleX4::setBrightness(uint32_t value)
{
	m_mapSliderNameValue["CDS_DC"] = value;
	setAdvancedBias("CDS_DC", value);
}

uint32_t CeleX4::getBrightness()
{
	return m_mapSliderNameValue["CDS_DC"];
}

void CeleX4::setThreshold(uint32_t value)
{
	unsigned uiEVT_DC = m_mapSliderNameValue["EVT_DC"];
	uint32_t uiEVT_VL = uiEVT_DC - value;
	uint32_t uiEVT_VH = uiEVT_DC + value;

	m_mapSliderNameValue["EVT_VL"] = uiEVT_VL;
	m_mapSliderNameValue["EVT_VH"] = uiEVT_VH;

	setAdvancedBias("EVT_VL", uiEVT_VL);
	setAdvancedBias("EVT_VH", uiEVT_VH);
}

uint32_t CeleX4::getThreshold()
{
	return m_mapSliderNameValue["EVT_VH"] - m_mapSliderNameValue["EVT_DC"];
}

void CeleX4::trigFullPic()
{
	excuteCommand("Force Fire");
}

//In FullPic and FullPic_Event mode, time block is calculated by FPGA, so it need be set the FGPA
void CeleX4::setFullPicFrameTime(uint32_t msec)
{
	cout << "API: setFullPicFrameTime " << msec << " ms" << endl;
	uint32_t value = msec * 25000;
	//--- excuteCommand("SetMode Total Time"); ---
	FrontPanel::getInstance()->wireIn(0x02, value, 0x00FFFFFF);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0x00FFFFFF << endl;

	m_uiFullPicFrameTime = msec;
}

uint32_t CeleX4::getFullPicFrameTime()
{
	return m_uiFullPicFrameTime;
}

//The usrs hope to set the time block to 1s in FE mode, but there aren't enough bits, 
//in order to solve this problem, FPGA multiplys the msec by 4. 
void CeleX4::setFEFrameTime(uint32_t msec)
{
	cout << " API: setFEFrameTime " << msec << " ms" << endl;
	uint32_t value = msec * 6250; //6250 = 25000/4 
	//--- excuteCommand("SetMode Total Time"); ---
	FrontPanel::getInstance()->wireIn(0x02, value, 0x00FFFFFF);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0x00FFFFFF << endl;

	m_uiFEFrameTime = msec;
}

uint32_t CeleX4::getFEFrameTime()
{
	return m_uiFEFrameTime;
}

// init sliders
void CeleX4::parseSliderSequence()
{
	cout << endl << "***** " << __FUNCTION__ << " *****" << endl;
	std::vector<std::string> sliderNames = m_pSequenceMgr->getAllSliderNames();
	for (vector<string>::iterator itr = sliderNames.begin(); itr != sliderNames.end(); itr++)
	{
		std::string name = *itr;
		HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name);

		if (!pSliderSeq)
			continue;
		// show or not
		if (!pSliderSeq->isShown())
			continue;
		if (pSliderSeq->isAdvanced())
		{
			m_vecAdvancedNames.push_back(name);
			if (HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name))
			{
				uint32_t initial = pSliderSeq->getValue();
				// keep the initial values
				m_mapSliderNameValue[name] = initial;
				//cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
			}
			continue;
		}
		uint32_t initial = pSliderSeq->getValue();
		// keep the initial values
		m_mapSliderNameValue[name] = initial;
		//cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
	}
}

bool CeleX4::excuteCommand(string strCommand)
{
	bool bSuccess = false;
	if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(strCommand))
	{
		if (!pUpSeq->fire())
			cout << "excute command failed." << endl;
		else
			bSuccess = true;
	}
	else
	{
		cout << strCommand << ": Sequence not defined." << endl;
	}
	return bSuccess;
}

bool CeleX4::setAdvancedBias(std::string strBiasName)
{
	bool result = false;
	if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(strBiasName))
	{
		uint32_t arg = m_mapSliderNameValue[strBiasName];
		result = pSeqSlider->fireWithArg(arg);
		cout << "Advanced Settings loaded: " << strBiasName << "  " << arg << endl;
	}
	return result;
}

bool CeleX4::setAdvancedBias(std::string strBiasName, int value)
{
	bool result = false;
	if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(strBiasName))
	{
		result = pSeqSlider->fireWithArg(value);
		cout << "Advanced Settings loaded: " << strBiasName << "  " << value << endl;
	}
	return result;
}

uint32_t CeleX4::getClockRate()
{
	return m_uiClockRate;
}
//value * 2 = clock = 100 * M / D
void CeleX4::setClockRate(uint32_t value)
{
	cout << "************* API: setClockRate " << value << " MHz" << endl;
	m_uiClockRate = value;
	if (value > 50)
		value = 50;
	uint32_t valueM, valueD = 0x00630000;
	valueM = (value * 2 - 1) << 24;
	FrontPanel::getInstance()->wireIn(0x03, valueM, 0xFF000000); //M: [31:24]
	FrontPanel::getInstance()->wireIn(0x03, valueD, 0x00FF0000); //D: [23:16]

	FrontPanel::getInstance()->wireIn(0x03, 0, 0x00008000); //Apply OFF [15]
	FrontPanel::getInstance()->wait(1);
	FrontPanel::getInstance()->wireIn(0x03, 0x00008000, 0x00008000); //Apply ON  [15]
}

void CeleX4::setIMUIntervalTime(uint32_t value)
{
	//excuteCommand("SetIMU Interval Time");
	if (value > 255)
		value = 255;
	value = value << 24;
	FrontPanel::getInstance()->wireIn(0x02, value, 0xFF000000);
	//FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0xFF000000 << endl;
}