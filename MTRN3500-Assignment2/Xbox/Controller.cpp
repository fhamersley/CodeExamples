#include "Controller.h"

XINPUT_BATTERY_INFORMATION BatteryInformation;
XINPUT_STATE State;
XINPUT_VIBRATION Vibration;

//Controller::Controller(int CtlrID) { ControllerID = CtlrID; }
Controller::Controller()
{
}

Controller::~Controller()
{
}

void Controller::SetDeadzone(int Radius)
{
	DeadzoneRadius = Radius;
}

bool Controller::IsConnected()
{
	if (XInputGetState(0, &State) == ERROR_DEVICE_NOT_CONNECTED)
	{
		cout << "Joystick not connected " << endl;
		return 0;
	}
	else
	{
		return 1;
	}
}

bool Controller::IsButtonPressed(int Button)
{
	XInputGetState(0, &State);
	if (bool(State.Gamepad.wButtons & Button))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int Controller::GetButton()
{
	XInputGetState(0, &State);
	return State.Gamepad.wButtons;
}

void Controller::GetThumbstick()
{
	XInputGetState(0, &State);
	float lX = State.Gamepad.sThumbLX;
	float lY = State.Gamepad.sThumbLY;
	float rX = State.Gamepad.sThumbRX;
	float rY = State.Gamepad.sThumbRY;
	float SizeLeft = sqrt(lX*lX + lY*lY);
	float SizeRight = sqrt(rX*rX + rY*rY);

	if (SizeLeft > DeadzoneRadius)
	{
		LX = lX;
		LY = lY;
	}
	if (SizeLeft < DeadzoneRadius)
	{
		LX = 0;
		LY = 0;
	}
	if (SizeRight > DeadzoneRadius)
	{
		RX = rX;
		RY = rY;
	}
	if (SizeRight < DeadzoneRadius)
	{
		RX = 0;
		RY = 0;
	}
}

void Controller::GetTriggers()
{
	XInputGetState(0, &State);
	LT = (float) State.Gamepad.bLeftTrigger / 255;
	RT = (float) State.Gamepad.bRightTrigger / 255;
}
