//#include <stdafx.h>
#include <iostream>
#include <fstream>
#include <set>
#include <sys/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "PCM.h"

using namespace std;

PCM::PCM()		//Default contructor
{
	BaseSet();			//Calls function for checking if base is set and will ask user to set a base
	Data[0] = 0;		//Initalises data with 0's
	Data[1] = 0;
	if (ioperm(BASE, 16, 1) != 0) {
		cout << "No I/O permission" << endl;
		BASE = 0;		//Sets base to 0 as no I0 permission on this base
		BaseSetFlag = 0;
	}
}

PCM::PCM(int BaseInput)		//Contructor with base set
{
	BASE = BaseInput;
	BaseSetFlag = 1;
	Data[0] = 0;		//Initalises data with 0's
	Data[1] = 0;
	if (ioperm(BASE, 16, 1) != 0) {
		cout << "No I/O permission" << endl;
		BASE = 0;
		BaseSetFlag = 0;
	}
}

PCM::~PCM()		//Default destructor
{
}

void PCM::CheckIO() {
	if (ioperm(BASE, 16, 1) != 0) {		//Checks for IO permission on set base, and if not clears base
		cout << "No I/O permission" << endl;
		BASE = 0;
		BaseSetFlag = 0;
	}
}

void PCM::SetBase(int BaseInput)		//Function for setting the base
{
	BASE = BaseInput;		//Value passed to function set as BASE
}

bool PCM::BaseSet()		//Function for checking base has been set
{
	int temp;		//temp storage for entered base
	if ((BASE <= 0) || (BASE >= 1000)) {		//If base has not been set correctly, asks uses to set a base
		cout << "Error: Base has not been correctally set" << endl << "Please enter base: " << endl;
		cin >> temp;
		if ((temp > 0) && (temp < 1000)) {		//Checks base value is suitable before setting
			BASE = temp;
			cout << "Base has been set to: " << BASE << endl;		//Informs what base has been set to
			BaseSetFlag = 1;
			return 1;		//Return 1 as base set correctally
		}
		else {
			cout << "Error: Entered base out of range (0 < Base < 1000)" << endl;		//Error message for base out of range
			BaseSetFlag = 0;
			return 0;		//Return 0 as base not set
		}
	}
	else {
		BaseSetFlag = 1;
		return 1;
	}
}


int PCM::GetBase()		//Function for returning BASE value
{
	return BASE;
}

int PCM::GetData(bool HighLow)		//Function for returning current data in either high (1) or low (0)
{
	if (!HighLow) {
		return Data[0];
	}
	else if (HighLow) {
		return Data[1];
	}
}

void PCM::SetByte(bool HighLow, int Output)		//Function for setting a byte on either high (1) or low (0)
{
	if (!HighLow && (Output >= 0) && (Output < 0x100) && BaseSet()) {		//Low requires HighLow = 0 and entered output to be +ve and < 0x100
		Data[0] = Output;													//Store latest output into Data
		outb(Output, BASE + 0x03);
	}
	else if (HighLow && (Output >= 0) && (Output < 0x100) && BaseSet()) {		//High requires HighLow = 1 and entered output to be +ve and < 0x100
		Data[1] = Output;														//Store latest output into Data
		outb(Output, BASE + 0x0B);
	}
	else if (BaseSet()) {
		cout << "Error: Passed output byte out of range" << endl;		//Error message for incorrect input
	}
	else {
	}
}

void PCM::SetBit(bool HighLow, int BitNum)								//Function for setting/unsetting a bit on either high (1) or low (0) 
{
	int temp = 0;														//Initialise temp with 0
	if (!HighLow && (BitNum >= 0) && (BitNum < 8) && BaseSet()) {		//Low requires HighLow = 0 and entered bit number output to be +ve and < 8
		temp = 1 << BitNum;												//Temp stores a 1 in the bit position determined by BitNum
		Data[0] = Data[0] ^ temp;										//XOR swaps the bit using the byte currently set in data[0] and stores it in data[0]
		outb(Data[0], BASE + 0x03);
	}
	else if (HighLow && (BitNum >= 0) && (BitNum < 8) && BaseSet()) {		//High requires HighLow = 0 and entered bit number output to be +ve and < 8
		temp = 1 << BitNum;
		Data[1] = Data[1] ^ temp;
		outb(Data[1], BASE + 0x0B);
	}
	else if (BaseSet()) {
		cout << "Error: Passed output bit out of range" << endl;		//Error message for incorrect input bit
	}
	else {
	}
}

int PCM::GetByte(bool HighLow)				//Function for receiving byte on either high (1) or low (0)
{
	int temp;
	if (!HighLow && BaseSet()) {			//Low requires HighLow = 0 and checks base is set
		Data[0] = inb(BASE + 0x03); 		//Gets data from low byte and stores in Data[0]
		return Data[0];
	}
	else if (HighLow && BaseSet()) {			//High requires HighLow = 1 and checks base is set
		Data[1] = inb(BASE + 0x0B);			//Gets data from high byte and stores in Data[1]
		return Data[1];
	}
	else {
		cout << "Error: Base not set?" << endl;
		return -1;
	}
}

int PCM::GetBit(bool HighLow, int BitNum)		//Function for returning 1 if bit chosen is set or 0 if not set on high (1) or low (0)
{
	int temp, temp1, temp2 = 0;
	if (!HighLow && (BitNum >= 0) && (BitNum < 8) && BaseSet()) {
		temp = inb(BASE + 0x03); 		//Gets data from low byte and stores in temp
		//temp = Data[0];
		temp1 = 1 << BitNum;		//Shift bit accross into position and stores in temp1
		temp2 = temp & temp1;		//Compares temp1 (single bit set in position chosen) with data pulled from byte and stores result in temp2
		if (temp1 & temp2) {		//If temp1 and temp2 are the same them chosen but is set and return 1, otherwise bit is not set, return 0
			return 1;
		}
		else {
			return 0;
		}
	}
	else if (HighLow && (BitNum >= 0) && (BitNum < 8) && BaseSet()) {
		temp = inb(BASE + 0x0B); 	//Gets data from low byte and stores in temp
		//temp = Data[1];
		temp1 = 1 << BitNum;
		temp2 = temp & temp1;
		if (temp1 & temp2) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else if (BaseSet()) {
		cout << "Error: Passed input bit out of range" << endl;		//Error if input bit is out of range
		return -1;
	}
	else {
		cout << "Error: Base not set?" << endl;		//Error for base not set correctly
		return -1;
	}
}

int PCM::AnalogInput(int Range, int ChannelNum) const	//Function for receving an analog input on selected channel, with selected range
{
	int LowByte, HighByte;
	if ((Range >= 0) && (Range < 9) && (ChannelNum >= 0) && (ChannelNum < 8) && BaseSetFlag) {		//Checks that Range and Channel value passesed are valid
		outb(Range, BASE + 0x01);		//Set the A/D range

			//	Ranges		Send		Ranges		Send
			//	+-5		0x00		0-10		0x04
			//	+-2.5		0x01		0-5		0x05
			//	+-1.25		0x02		0-2.5		0x06
			//	+-0.625		0x03		0-1.25		0x07

		outb(ChannelNum | (ChannelNum << 4), BASE + 0x02);		//Splits the chosen channel number to the corresponding bits and sends

		outb(0x01, BASE + 0);						//Trigger a conversion by writing any data to Base + 0

		while (inb(BASE + 0x08) & 0x80)	{}				//Wait for conversion to complete by 'watching' bit 8 of the A/D status register
		LowByte = inb(BASE + 0x00);					//Pull data from low byte and store
		HighByte = inb(BASE + 0x01);					//Pull data from high byte and store
		return ((LowByte >> 4) | (HighByte << 4));		//Return data shifted correctally
	}
	else if (((Range < 0) || (Range >= 9) || (ChannelNum < 0) || (ChannelNum >= 8)) && BaseSetFlag) {
		cout << "Error: Range and/or Channel Number passed invalid" << endl;
		return -1;
	}
	else {
		cout << "Error: Base not set" << endl;
		return -1;
	}
}

std::ostream &operator<<(std::ostream &os, PCM const &p)			//Opperator overloading of << to print all analog in channels with tab seperation using default value of +-5V
{
	for (int i = 0; i < 8; i = i + 1) {
		os << p.AnalogInput(0x00, i) << "\t";			//Cout AnalogInput for +-5V for each of 8 channels (0-7) with tab seperation
	} 
	return os;
	//cout << endl << "All channels printed" << endl;
}
