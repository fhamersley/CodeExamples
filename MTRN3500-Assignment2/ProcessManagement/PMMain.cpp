#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <string>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <tchar.h>
#include <dos.h>
#include <stdlib.h>
#include <cstdio>
#include <SMObject.h>
#include <SMStructs.h>
#include <conio.h>
#include <tlhelp32.h>

using namespace std;

#define CRITICAL_MASK 0x000F
#define NONCRITICAL_MASK 0x0070
#define NUM_MODULES 6
// Unused:HLevel : SM:GPS:Vehicle:Simulator : Xbox:Plot:Laser:PM
// xxxx xxx0 0000 1111 = 0x000F = Critical mask
// xxxx xxx0 0111 0000 = 0x0070 = NonCritcal mask

// Xbox button defines
#define BUT_A 0x1000
#define BUT_B 0x2000
#define BUT_X 0x4000
#define BUT_Y 0x8000


TCHAR* Modules[10] =
{
	TEXT("Laser.exe"),
	TEXT("Plotting.exe"),
	TEXT("Xbox.exe"),
	TEXT("Simulator.exe"),
	TEXT("VehicleControl.exe"),
	TEXT("GPS.exe")
};

// Declartion of IsProcessRunning()
bool IsProcessRunning(const char *processName);
bool RestartModule(int ModuleNum);

int main()
{
	__int64 Frequency, HPCCount;
	int NonCriticalMaskCount = 0;
	int CriticalMaskCount = 0;

	// Module execution based variable declarations
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject GPSObj(TEXT("GPS"), sizeof(GPS));
	SMObject LaserObj(TEXT("Laser"), sizeof(Laser));
	SMObject PlotObj(TEXT("Plotting"), sizeof(Plotting));
	SMObject SimObj(TEXT("Simulator"), sizeof(Simulator));
	SMObject VCObj(TEXT("VehicleControl"), sizeof(VehicleControl));
	SMObject XboxObj(TEXT("Xbox"), sizeof(Xbox));

	// Set up shared memory
	PMObj.SMCreate();
	GPSObj.SMCreate();
	LaserObj.SMCreate();
	PlotObj.SMCreate();
	SimObj.SMCreate();
	VCObj.SMCreate();
	XboxObj.SMCreate();

	// Get access to shared memory
	PMObj.SMAccess();
	//SimObj.SMAccess();
	XboxObj.SMAccess();

	ProcessManagement * PM = (ProcessManagement*)PMObj.pData;		//Pointer 'PM' for accessing the data
	Xbox * pXbox = (Xbox*)XboxObj.pData;							//Pointer 'pXbox' for accessing data

	// Start other modules in specified order
	//Run all other processes =======
	for (int i = 0; i < NUM_MODULES; i++)
	{
		// Check if each process is running
		if (!IsProcessRunning(Modules[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));
			// Start the child processes.

			if (!CreateProcess(NULL,   // No module name (use command line)
				Modules[i],        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,              // No creation flags
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory
				&s[i],            // Pointer to STARTUPINFO structure
				&p[i])           // Pointer to PROCESS_INFORMATION structure
				)
			{
				//printf("%s failed (%d).\n", Modules[i], GetLastError());
				//_getch();
				return -1;
			}
		}
		cout << "Started: " << Modules[i] << endl;
		//Sleep(100);		// Any other initialization
	}
	
	// Get HPC Frequency
	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);

	
	// Run all modules
	PM->ShutDown.Status = 0x00;
	cout << "Process management running. Non-critical processes will be restarted in loss of heartbeat. Critcal processes without heartbeat will cause total shutdown" << endl;
	while (!PM->ShutDown.Flags.PM)
	{
		// Set PM Heartbeat
		PM->HeartBeat.Flags.PM = 1;
		// Get HPC Count
		QueryPerformanceCounter((LARGE_INTEGER *)& HPCCount);
		// Calculate TimeStamp
		PM->PMTimeStamp = (double)HPCCount / (double)Frequency;

		// Keep track of live execution of all modules
		// If a module has failed
		// Restart if not critical
		if ((PM->HeartBeat.Status & NONCRITICAL_MASK) != NONCRITICAL_MASK)
		{
			NonCriticalMaskCount++;
			if (NonCriticalMaskCount > 15)
			{
				// Initate restart
				int const BitMods = 0x007F;
				int ModuleDown;
				ModuleDown = PM->HeartBeat.Status ^ BitMods;
				switch (ModuleDown) 
				{
				case 0x0010: 
					RestartModule(4);		//Simulator
					break;
				case 0x0020: 
					RestartModule(5);		//VC
					break;
				case 0x0040:
					RestartModule(6);		//GPS
					break;
				case 0x0030:
					RestartModule(4);		//Sim
					RestartModule(5);		//VC
					break;
				case 0x0050:
					RestartModule(4);		//Sim
					RestartModule(6);		//GPS
					break;
				case 0x0060:
					RestartModule(5);		//VC
					RestartModule(6);		//GPS
					break;
				case 0x0070:
					RestartModule(4);		//Sim 
					RestartModule(5);		//VC
					RestartModule(6);		//GPS
					break;
				}
				NonCriticalMaskCount = 0;
			}
		}
		else 
		{
			NonCriticalMaskCount = 0;
		}
		if ((PM->HeartBeat.Status & CRITICAL_MASK) != CRITICAL_MASK)
		{
			CriticalMaskCount++;
			//cout << "Crit Mask counter: " << dec << CriticalMaskCount << endl;
			//Sleep(100);
			if (CriticalMaskCount > 100)
			{
				cout << "Shutting down due to critcal process down" << endl;
				//Sleep(10000);
				// Shutdown all modules
				PM->ShutDown.Flags.Vehicle = 1;		// Vehicle control
				Sleep(200);							// Wait for 200ms
				PM->ShutDown.Flags.Laser = 1;		// Laser
				Sleep(200);							// Wait
				PM->ShutDown.Flags.Plot = 1;		// Plotting (camera)
				Sleep(200);							// Wait
				PM->ShutDown.Flags.GPS = 1;			// GPS
				Sleep(200);							// Wait
				PM->ShutDown.Flags.Xbox = 1;		// Xbox
				Sleep(200);							// Wait
				PM->ShutDown.Status = 0xFF;			// Shutdown all
				CriticalMaskCount = 0;
			}
		}
		else
		{
			CriticalMaskCount = 0;
		}
		
		// Otherwise enter into safe mode if critcal failure (could mean shut down)

		// Terminate when Start button is pressed on controller by checking flag
		if (pXbox->Terminate) 
		{
			break; 
		}

		// Keep looping until keyboard stroke
		if (_kbhit())
			break;
		// Force all heartbeats to 0 for each modules to self correct
		PM->HeartBeat.Status = 0x00;
		Sleep(100);
	} 

	// At termination
		// Shut down modules in specified order 
	PM->ShutDown.Flags.Vehicle = 1;		// Vehicle control
	Sleep(200);							// Wait for 200ms
	PM->ShutDown.Flags.Laser = 1;		// Laser
	Sleep(200);							// Wait
	PM->ShutDown.Flags.Plot = 1;		// Plotting (camera)
	Sleep(200);							// Wait
	PM->ShutDown.Flags.GPS = 1;			// GPS
	Sleep(200);							// Wait
	PM->ShutDown.Flags.Xbox = 1;		// Xbox
	Sleep(200);							// Wait
	PM->ShutDown.Status = 0xFF;			// Shutdown all

	// Release allocated memory
	// Automatically done by destructor
	// Exit
	return 0;
}

bool IsProcessRunning(const char *processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp(entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}

bool RestartModule(int ModuleNum)
{
	STARTUPINFO sR[10];
	PROCESS_INFORMATION pR[10];

	// Check if each process is running
	if (!IsProcessRunning(Modules[ModuleNum - 1]))
	{
		ZeroMemory(&sR[ModuleNum - 1], sizeof(sR[ModuleNum - 1]));
		sR[ModuleNum - 1].cb = sizeof(sR[ModuleNum - 1]);
		ZeroMemory(&pR[ModuleNum - 1], sizeof(pR[ModuleNum - 1]));
		// Start the child processes.

		if (!CreateProcess(NULL,   // No module name (use command line)
			Modules[ModuleNum - 1],        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			CREATE_NEW_CONSOLE,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory
			&sR[ModuleNum - 1],            // Pointer to STARTUPINFO structure
			&pR[ModuleNum - 1])           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("%s failed (%d).\n", Modules[ModuleNum - 1], GetLastError());
			_getch();
			return 0;
		}
	}
	cout << "Restarted: " << Modules[ModuleNum - 1] << endl;
	Sleep(100);		// Any other initialization
	return 1;
}
