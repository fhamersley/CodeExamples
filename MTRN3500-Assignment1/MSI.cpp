#include <sys/io.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include "MSI.h"


using namespace std;

MSI::MSI()		//Default contructor
{
	BaseSet();		//Calls function for checking if base is set and will ask user to set a base
	if (ioperm(BASE, 16, 1) != 0) {
		cout << "No I/O permission" << endl;
		BASE = 0;		//Sets base to 0 as no I0 permission on this base
	}
}

MSI::MSI(int BaseInput)		//Contructor with base set
{
	BASE = BaseInput;
	BaseSet();		//Calls function for checking if base is set and will ask user to set a base
	if (ioperm(BASE, 16, 1) != 0) {
		cout << "No I/O permission" << endl;
		BASE = 0;		//Sets base to 0 as no I0 permission on this base
	}
}

MSI::~MSI()		//Default destructor
{
}

void MSI::CheckIO() {
	if (ioperm(BASE, 16, 1) != 0) {		//Checks for IO permission on set base, and if not clears base
		cout << "No I/O permission" << endl;
		BASE = 0;
	}
}

void MSI::SetBase(int BaseInput)		//Function for setting the base
{
	BASE = BaseInput;		//Value passed to function set as BASE
}

bool MSI::BaseSet()		//Function for checking base has been set and asking user to enter a base if base has not been set
{
	int temp;		//temp storage for entered base
	if ((BASE <= 0) || (BASE >= 1000)) {		//If base has not been set correctly, asks uses to set a base
		cout << "Error: Base has not been correctally set" << endl << "Please enter base: " << endl;
		cin >> temp;
		if ((temp > 0) && (temp < 1000)) {		//Checks base value is suitable before setting
			BASE = temp;
			cout << "Base has been set to: " << BASE << endl;		//Informs what base has been set to
			return 1;		//Return 1 as base set correctally
		}
		else {
			cout << "Error: Entered base out of range (0 < Base < 1000)" << endl;		//Error message for base out of range
			return 0;		//Return 0 as base not set
		}
	}
	else {
		return 1;		//Base has been correctally set
	}
}

int MSI::GetBase()		//Function for returning BASE value
{
	return BASE;
}

void MSI::ChannelReset(int ChannelNum)			//Function for setting analog output on specific channel and with specified output
{
	if ((ChannelNum >= 0) && (ChannelNum < 8) && BaseSet()) {
		outb(0x00, BASE + ChannelNum);
		cout << "Channel " << ChannelNum << " Reset" << endl;
	}
	else if (BaseSet()) {
		cout << "Error: Cannot reset as channel number out of range" << endl;
	}
	else {
		cout << "Error: Base not set correctally" << endl;
	}
} 

bool MSI::IndexCheck(int IndexBase)			//Function for checking index pulse of only channel 0 and then resets count to 0
{
	//int temp = 0;
	if (ioperm(IndexBase, 16, 1) != 0) {
		cout << "No Index permission" << endl;
	}
	
	if (inb(IndexBase) & 0x80) {
		ChannelReset(0);
		//cout << "Index Reset" << endl;
		return 1;
	}
	else {
		return 0;
	}
}

int MSI::ChannelRead(int ChannelNum)		//Function for reading from specified channel
{
	int temp = 0;
	temp = ChannelNum * 4;		//Adjusts temp for correct adjustment for base value
	if ((ChannelNum >= 0) && (ChannelNum < 8) && BaseSet()) {		//Checks channel number is in range and that base is also set
		for (int i = 0; i < 4; i++) {		//Loop for storing each byte into Union enc
			enc.b[i] = inb(BASE + temp + i);
		}
		return enc.a;		//Returns int a from Union enc
	}
	else if (BaseSet()) {
		cout << "Error: Channel number out of range" << endl;		//Error for channel number out of range
		return -1;
	}
	else {
		cout << "Error: Base not set correctally" << endl;			//Error for base out of range
		return -1;
	}
}

MSI MSI::operator!() {				//Opperator overloading of ! to reset all channels
	for (int i = 0; i < 8; i = i + 1) {		//Loop for reseting all cheannels
		ChannelReset(i);			//Calls ChannelReset function for i=0, 1, 2, ..., 7, which resets all channels.
	}
}
