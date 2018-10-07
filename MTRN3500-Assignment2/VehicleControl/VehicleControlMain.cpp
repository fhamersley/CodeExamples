#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif

#include <iostream>
#include <conio.h>
#include <SMObject.h>
#include <SMStructs.h>
#include <VehicleController.h>

using namespace std;

int main()
{
	__int64 Frequency, HPCCount;
	double VCTimestamp;

	VehicleController VehCon("192.168.1.200", "25000");

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject VCObj(TEXT("VehicleControl"), sizeof(VehicleControl));
	SMObject XboxObj(TEXT("Xbox"), sizeof(Xbox));

	PMObj.SMAccess();
	VCObj.SMAccess();
	XboxObj.SMAccess();

	ProcessManagement * PM = (ProcessManagement*)PMObj.pData;		// Pointer 'PM' for accessing the data
	VehicleControl * pVC = (VehicleControl*)PMObj.pData;			// Pointer 'pVC' for accessing the data
	Xbox * pXbox = (Xbox*)XboxObj.pData;							// Point 'pXbox' for accessing SM

	cout << "Vehicle Control Module started" << endl;

	/*//--------------------------------------------------------------------------------------------//
	PM->ShutDown.Flags.Vehicle = 0;
	while (!PM->ShutDown.Flags.Vehicle)
	{
		PM->HeartBeat.Flags.Vehicle = 1;
		cout << "waiting" << endl;
	}
	return 0;
	//--------------------------------------------------------------------------------------------//*/

	if (VehCon.ConnectToServer() != 0)
	{
		cout << "Error: Vehicle Control could no connected to server" << endl;
		return -1;
	}

	cout << "Sent ZNUM" << endl;
	char ZNUM[256] = "3416362\n";
	VehCon.SendZNum(ZNUM, strlen(ZNUM));

	// Get HPC Frequency
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
	PM->ShutDown.Flags.Vehicle = 0;

	while (!PM->ShutDown.Flags.Vehicle)
	{
		// Set VC Heartbeat
		PM->HeartBeat.Flags.Vehicle = 1;
		// Get HPC Count
		QueryPerformanceCounter((LARGE_INTEGER *)&HPCCount);
		// Calculate VCTimeStamp
		VCTimestamp = (double)HPCCount / (double)Frequency;
		// Send TimeStamp to SM
		pVC->VCTimeStamp = VCTimestamp;

		VehCon.Drive(pXbox->SetSpeed, -pXbox->SetSteering);

		// Close module if PM is dead
		if (PM->HeartBeat.Flags.PM == 0)
		{
			double TempTS = PM->PMTimeStamp;
			Sleep(200);
			if (TempTS == PM->PMTimeStamp)
			{
				cout << "PM not detected. Shutting down Vehicle Control" << endl;
				PM->ShutDown.Flags.Vehicle = 1;
			}
		}

		PM->HeartBeat.Flags.Vehicle = 1;
		Sleep(100);
		if (_kbhit())
			break;
	}
	return 0;
}