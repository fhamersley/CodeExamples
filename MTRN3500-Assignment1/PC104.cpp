#include <sys/io.h>
#include <iostream>
#include <math.h>
#include "PC104.h"


using namespace std;

PC104::PC104()		//Default contructor
{
	BaseSet();		//Calls function for checking if base is set and will ask user to set a base
	if (ioperm(BASE, 16, 1) != 0) {
		cout << "No I/O permission" << endl;
		BASE = 0;		//Sets base to 0 as no I0 permission on this base
	}
}

PC104::PC104(int BaseInput)		//Contructor with base set
{
	BASE = BaseInput;
	if (ioperm(BASE, 16, 1) != 0) {		//Checks for IO permission on set base, and if not clears base
		cout << "No I/O permission" << endl;
		BASE = 0;		//Sets base to 0 as no I0 permission on this base
	}
}

PC104::~PC104()		//Default destructor
{
}

void PC104::CheckIO() {
	if (ioperm(BASE, 16, 1) != 0) {		//Checks for IO permission on set base, and if not clears base
		cout << "No I/O permission" << endl;
		BASE = 0;
	}
}

void PC104::SetBase(int BaseInput)		//Function for setting the base
{
	BASE = BaseInput;		//Value passed to function set as BASE
}

bool PC104::BaseSet()		//Function for checking base has been set and asking user to enter a base if base has not been set
{
	int temp;		//temp storage for entered base
	if ((BASE <= 0) || (BASE >= 1000)) {		//If base has not been set correctly, asks uses to set a base
		cout << "Error: Base has not been correctally set" << endl << "Please enter base: " << endl;
		cin >> temp;
		if ((temp > 0) && (temp < 1000)) {		//Checks base value is suitable before setting
			BASE = temp;
			cout << "Base has been set to: " << BASE << endl;		//Informs what base has been set to
			return 1;
		}
		else {
			cout << "Error: Entered base out of range (0 < Base < 1000)" << endl;		//Error message for base out of range
			return 0;
		}
	}
	else {
		return 1;		//Base has been correctally set
	}
}

int PC104::GetBase()		//Function for returning BASE value
{
	return BASE;
}

void PC104::AnalogOutput(int ChannelNum, int VoltValue, bool Voltage)			//Function for setting analog output on specific channel and with specified output
{
	int temp, temp1, temp2 = 0;
	switch(Voltage) {
		case true :		//For using input as a voltage
			if ((ChannelNum >= 0) && (ChannelNum < 6) && (VoltValue >= -5) && (VoltValue <= 5 ) && BaseSet()) {
				temp = (ChannelNum * 2);
				temp1 = int(round((VoltValue + 5) * 409.5)); 	//Converts VoltValue to number from 0 to 4096
				//cout << temp1 << endl;
				outb(temp1, BASE + temp);		//Sends first byte
				temp2 = temp1 >> 8;		//Shifts data by 8 bits
				outb(temp2, BASE + temp + 1);		//Sends second byte
				break;
			}
			else if (BaseSet()) {
				cout << "Error: Channel Number or Voltage value out of range" << endl;		//Error for Channel number or value out of range 
				break;
			}
			else {
				cout << "Error: Base not set" << endl;		//Error for base not set
				break;
			}
		case false :		//For using input as a value between 0 - 4095
			if ((ChannelNum >= 0) && (ChannelNum < 6) && (VoltValue >= 0) && (VoltValue < 4096 ) && BaseSet()) {
				temp = (ChannelNum * 2);
				//cout << temp1 << endl;
				//cout << VoltValue << endl;
				outb(VoltValue, BASE + temp);		//Sends first byte
				temp2 = VoltValue >> 8;		//Shifts data by 8 bits
				outb(temp2, BASE + temp + 1);		//Sends second byte
				break;
			}
			else if ((ChannelNum >= 0) && (ChannelNum < 6) && (VoltValue >= 4096)) {		//If the Voltvalue set is out of range it sets the value to be the max
				VoltValue = 4095;
				temp = (ChannelNum * 2);
				outb(VoltValue, BASE + temp);		//Sends first byte
				temp2 = VoltValue >> 8;		//Shifts data by 8 bits
				outb(temp2, BASE + temp + 1);		//Sends second byte
				break;
			}
			else if ((ChannelNum >= 0) && (ChannelNum < 6) && (VoltValue < 0)) {		//If the Voltvalue set is out of range it sets the value to be the min
				VoltValue = 0;
				temp = (ChannelNum * 2);
				outb(VoltValue, BASE + temp);		//Sends first byte
				temp2 = VoltValue >> 8;		//Shifts data by 8 bits
				outb(temp2, BASE + temp + 1);		//Sends second byte
				break;
			}
			else {
				cout << "Error: Base not set dingaling" << endl;		//Error for base not set
				break;
			}
	}
}
