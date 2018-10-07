#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <Windows.h>
#include <conio.h>
#include <SMObject.h>
#include <SMStructs.h>
#include <LMS151.h>
#include <iostream>

using namespace std;

int main()
{
	__int64 Frequency, HPCCount;
	double LTimeStamp;

	LMS151 LMS151Laser("192.168.1.200", "23000");		// Need to confirm IP and port

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject LaserObj(TEXT("Laser"), sizeof(Laser));

	PMObj.SMAccess();
	LaserObj.SMAccess();

	ProcessManagement * PM = (ProcessManagement*)PMObj.pData;		//Pointer 'PM' for accessing the data
	Laser * pLaser = (Laser*)LaserObj.pData;						//Pointer 'PLaser' for accessing the data

	cout << "Laser Module started" << endl;

	/*//--------------------------------------------------------------------------------------------//
	PM->ShutDown.Flags.Laser = 0;
	while (!PM->ShutDown.Flags.Laser)
	{
		PM->HeartBeat.Flags.Laser = 1;
		if (_kbhit()) {
			cout << "Breaking" << endl;
			Sleep(1000);
			break;
		}
	}
	return 0;

	//--------------------------------------------------------------------------------------------//*/

	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);


	// Laser specfic initializations
		// Connect to laser
	if (LMS151Laser.ConnectToServer() == 0)
	{
		cout << "Connected to Laser" << endl;
	}
		// Log in to laser
	//LMS151Laser.Login();
		// Configure laser
	//LMS151Laser.Configure(45, 135, HALF_DEG);
		// Logout
		// Start measurement

	cout << "Sent ZNUM" << endl;
	char ZNUM[256] = "3416362\n";
	LMS151Laser.SendZNum(ZNUM, strlen(ZNUM));
	Sleep(100);

	while (!PM->ShutDown.Flags.Laser)
	{
		// Set heartbeat
		PM->HeartBeat.Flags.Laser = 1;
		// Get HPC Count
		QueryPerformanceCounter((LARGE_INTEGER *)&HPCCount);
		// Calculate LTimeStamp
		LTimeStamp = (double)HPCCount / (double)Frequency;
		pLaser->LaserTimeStamp = LTimeStamp;

		// Read laser
		LMS151Laser.GetRangeCoords();
		
		// Put data to SM
		if (LMS151Laser.NumPoints > 0)
		{
			for (int i = 0; i < LMS151Laser.NumPoints; i++)
			{
				pLaser->x[i] = LMS151Laser.RangeData[i][0];
				pLaser->y[i] = LMS151Laser.RangeData[i][1];
			}
			pLaser->NumberPoints = LMS151Laser.NumPoints;		// Send Number of points to SM
		}
		
		cout << "NP" << pLaser->NumberPoints << endl;
		Sleep(20);

		// Close module if PM is dead
		if (PM->HeartBeat.Flags.PM == 0)
		{
			double TempTS = PM->PMTimeStamp;
			Sleep(200);
			if (TempTS == PM->PMTimeStamp)
			{
				cout << "PM not detected. Shutting down module Laser" << endl;
				PM->ShutDown.Flags.Laser = 1;
			}
		}
		// Set heartbeat
		PM->HeartBeat.Flags.Laser = 1;

		if (_kbhit())
			break;
	}
	cout << "Terminated Laser module normally" << endl;
	return 0;
}