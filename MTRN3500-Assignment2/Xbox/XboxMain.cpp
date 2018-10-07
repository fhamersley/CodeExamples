#include <iostream>
#include <conio.h>
#include <SMObject.h>
#include <SMStructs.h>
#include <Controller.h>

#define BUT_A 0x1000
#define BUT_B 0x2000
#define BUT_X 0x4000
#define BUT_Y 0x8000


using namespace std;

int main()
{
	__int64 Frequency, HPCCount;
	double XboxTimestamp;
	double Speed, Steering;

	// INITIALISE XBOX CLASS

	SMObject PMObj(TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject XboxObj(TEXT("Xbox"), sizeof(Xbox));

	PMObj.SMAccess();
	XboxObj.SMAccess();

	Controller Control;

	cout << "XBOX Module started" << endl;

	ProcessManagement * PM = (ProcessManagement*)PMObj.pData;		//Pointer 'PM' for accessing the data
	Xbox * pXbox = (Xbox*)XboxObj.pData;							// Point 'pXbox' for accessing SM

	// Set Controller Deadzone
	Control.SetDeadzone(8000);

	QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);

	PM->ShutDown.Flags.Xbox = 0;
	pXbox->Terminate = 0;

	// Check for Xbox controller connected, if not then shutdown module
	if (!Control.IsConnected())
	{
		PM->ShutDown.Flags.Xbox = 1;
	}

	while (!PM->ShutDown.Flags.Xbox)
	{
		// Set heartbeat
		PM->HeartBeat.Flags.Xbox = 1;
		// Get HPC Count
		QueryPerformanceCounter((LARGE_INTEGER *)&HPCCount);
		// Calculate VCTimeStamp
		XboxTimestamp = (double)HPCCount / (double)Frequency;
		// Send TimeStamp to SM
		pXbox->XboxTimeStamp = XboxTimestamp;
		
		// Get button and send to SM
		switch (Control.GetButton())
		{
		case BUT_A: pXbox->Button = BUT_A;
			break;
		case BUT_B: pXbox->Button = BUT_B;
			break;
		case BUT_X: pXbox->Button = BUT_X;
			break;
		case BUT_Y: pXbox->Button = BUT_Y;
			break;
		case 0: pXbox->Button = 0;
			break;
		}

		// Get thumbsticks and send to SM
		Control.GetThumbstick();
		Steering = double(Control.RX / 819.175);			// Convert to between -40 and 40
		if (Steering > 40) Steering = 40;
		if (Steering < -40) Steering = -40;
		pXbox->SetSteering = Steering;						// Send to SM

		// Get triggers and set speed in SM
		Control.GetTriggers();
		if ((Control.RT >= 0) && (Control.LT == 0))			// Right trigger is pulled but left is not
		{
			Speed = double(Control.RT);							// Set speed to RT
			pXbox->SetSpeed = Speed;							// Send to SM
		}
		else if ((Control.LT >= 0) && (Control.RT == 0))	// Left trigger is pulled but right is not
		{
			Speed = double(-Control.LT);						//Set speed to neg LT
			pXbox->SetSpeed = Speed;							// Send to SM
		}
		else if ((Control.RT >= 0) && (Control.LT >= 0))	// Both triggers pulled
		{
			Speed = 0;											// Set speed to 0
			pXbox->SetSpeed = Speed;							// Send to SM
		}
		else 
		{
			Speed = 0;
			pXbox->SetSpeed = Speed;							// Send to SM
		}

		// Close module if Start Button is pressed
		if (Control.IsButtonPressed(0x0010))		// Start button
		{
			pXbox->SetSpeed = 0;			// Set Speed to 0 for safety
			pXbox->SetSteering = 0;			// Set Steerign to 0 for safety
			pXbox->Terminate = 1;			// Set terminate flag to 1
		}

		// Check for Xbox controller connected, if not then shutdown module
		if (!Control.IsConnected())
		{
			pXbox->SetSpeed = 0;
			pXbox->SetSteering = 0;
			PM->ShutDown.Flags.Xbox = 1;
		}

		// Close module if PM is dead
		if (PM->HeartBeat.Flags.PM == 0)
		{
			double TempTS = PM->PMTimeStamp;
			Sleep(200);
			if (TempTS == PM->PMTimeStamp)
			{
				cout << "PM not detected. Shutting down Xbox" << endl;
				PM->ShutDown.Flags.Xbox = 1;
			}
		}

		PM->HeartBeat.Flags.Xbox = 1;


		if (_kbhit())
		{
			break;
		}
	}
	return 0;
}