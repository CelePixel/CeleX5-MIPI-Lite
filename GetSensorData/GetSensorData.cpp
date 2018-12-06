
#include "include/celex4/celex4.h"
#include "include/celex5/celex5.h"
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif

using namespace std;

int main()
{
	bool bCeleX4Device = false;
	if (bCeleX4Device)
	{
		CeleX4 *pCeleX4 = new CeleX4;
		if (NULL == pCeleX4)
			return 0;
		pCeleX4->openSensor();
		pCeleX4->setSensorMode(CeleX4::Event_Mode); //Full_Picture_Mode, Event_Mode, FullPic_Event_Mode

		long max_len = 12800000;
		unsigned char* pBuffer = new unsigned char[max_len];
		if (NULL == pBuffer)
		{
			return 0;
		}
		while (true)
		{
			long data_len = pCeleX4->getFPGADataSize();
			//cout << "data_len = " << data_len << endl;
			if (data_len > 0)
			{
				long read_len = pCeleX4->readDataFromFPGA(data_len > max_len ? max_len : data_len, pBuffer);
				cout << "--- read_len = " << read_len << endl;
				//
				// add you own code to parse the data
				//
#ifdef _WIN32
				Sleep(1);
#else
				usleep(1000);
#endif
			}
		}
	}
	else
	{
		CeleX5 *pCeleX5 = new CeleX5;
		if (NULL == pCeleX5)
			return 0;
		pCeleX5->openSensor();
		pCeleX5->setSensorFixedMode(CeleX5::Full_Picture_Mode); //Full_Picture_Mode, Event_Address_Only_Mode, Full_Optical_Flow_S_Mode
		while (true)
		{
			vector<uint8_t> buffer;
			pCeleX5->getMIPIData(buffer);
			cout << "data size = " << buffer.size() << endl;
			//
			// add you own code to parse the data
			//
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1000 * 1);
#endif
		}	
	}
}