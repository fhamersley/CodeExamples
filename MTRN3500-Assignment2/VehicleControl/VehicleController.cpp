#define _USE_MATH_DEFINES
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <VehicleController.h>
#include <stdio.h>

VehicleController::VehicleController() : EthernetClient()
{
	SetSpeed = 0;
	SetSteering = 0;
	Flag = 1;
}

VehicleController::VehicleController(char * portaddress, char* port) : EthernetClient(portaddress, port)
{
	SetSpeed = 0;
	SetSteering = 0;
	Flag = 1;
}

VehicleController::~VehicleController()
{
}

void VehicleController::SetSetSpeed(double setSpeed)
{
	SetSpeed = setSpeed;
}

void VehicleController::SetSetSteering(double setSteering)
{
	SetSteering = setSteering;
}

double VehicleController::GetSpeed()
{
	return SetSpeed;
}

double VehicleController::GetSteering()
{
	return SetSteering;
}

void VehicleController::Drive(double speed, double steering)
{
	char Buffer[128];
	SetSpeed = speed;
	SetSteering = steering;
	Flag = 1 - Flag;		// To toggle flag
	sprintf_s(Buffer, "# %.3f %.3f %1d #", SetSteering, SetSpeed, Flag);
	SendData(Buffer, strlen(Buffer));
}
