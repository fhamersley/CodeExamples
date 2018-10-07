
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Windows.h>
#include <iostream>
#include <XInput.h>
#include <math.h>

using namespace std;

//XINPUT_BATTERY_INFORMATION BatteryInformation;
//XINPUT_STATE State;
//XINPUT_VIBRATION Vibration;

class Controller
{
//static int ControllerID;

public:
	bool Quit = false;
	float LX;
	float LY;
	float RX;
	float RY;
	float LT;
	float RT;

	Controller();
	~Controller();
	void SetDeadzone(int Radius);
	bool IsConnected();
	bool IsButtonPressed(int Button);
	int GetButton();
	void GetThumbstick();
	void GetTriggers();
private:
	int DeadzoneRadius;
};

#endif