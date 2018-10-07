#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <SMObject.h>
#include <SMStructs.h>
#include <iostream>
#include <conio.h>
#include <GPS.h>

int main()
{
	__int64 Frequency, HPCCount;
	double GPSTimestamp;
	int temp;

	// SM set up
	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject GPSObj(TEXT("GPS"), sizeof(GPS));

	PMObj.SMAccess();
	GPSObj.SMAccess();

	ProcessManagement * PM = (ProcessManagement*)PMObj.pData;
	GPS * pGPS = (GPS*)GPSObj.pData;

	std::cout << "GPS Module started" << std::endl;
	/*//--------------------------------------------------------------------------------------------//
	PM->ShutDown.Flags.GPS = 0;
	while (!PM->ShutDown.Flags.GPS) 
	{
		PM->HeartBeat.Flags.GPS = 1;
		if (_kbhit()) {
			cout << "Breaking" << endl;
			Sleep(1000);
			break;
		}
	}
	return 0;
	//--------------------------------------------------------------------------------------------//*/
	// Get high performace counter frequency
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	PM->HeartBeat.Flags.GPS = 1;

	GPSC gps("192.168.1.200", "24000");

	if (gps.ConnectToServer() != 0)
	{
		cout << "Error: GPS could not connected to server" << endl;
		return -1;
	}

	PM->ShutDown.Flags.GPS = 0;

	while (!PM->ShutDown.Flags.GPS) {
		// Set GPS heartbeat
		PM->HeartBeat.Flags.GPS = 1;
		// Get HPC Count
		QueryPerformanceCounter((LARGE_INTEGER *)&HPCCount);
		// Calculate VCTimeStamp
		GPSTimestamp = (double)HPCCount / (double)Frequency;
		// Send TimeStamp to SM
		pGPS->GPSTimeStamp = (double)HPCCount / (double)Frequency;

		// Get data and then send to SM
		if ((temp = gps.GetData()) == 0) {
			pGPS->Northing = gps.Northing;
			pGPS->Easting = gps.Easting;
			pGPS->Height = gps.Height;
			std::cout << "Northing (m): " << gps.Northing << ", Easting (m): " << gps.Easting << ", Height (m): " << gps.Height << ", BUENO CRC: SI" << std::endl;
		}
		else if (temp == 1) {
			std::cout << "CRC did not match calculated" << std::endl;
		}
		else {
			std::cout << "Data not received" << std::endl;
		}

		// Close module if PM is dead
		if (PM->HeartBeat.Flags.PM == 0)
		{
			double TempTS = PM->PMTimeStamp;
			Sleep(200);
			if (TempTS == PM->PMTimeStamp)
			{
				cout << "PM not detected. Shutting down module GPS" << endl;
				PM->ShutDown.Flags.GPS = 1;
			}
		}

		if (_kbhit()) {
			break;
		}
		// Set GPS heartbeat
		PM->HeartBeat.Flags.GPS = 1;
		Sleep(20);
	}
	cout << "GPS Module shutting down normally" << endl;
	return 0;
}