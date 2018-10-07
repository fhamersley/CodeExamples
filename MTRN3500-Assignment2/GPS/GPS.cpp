#define _USE_MATH_DEFINES
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <GPS.h>
#include <conio.h>

GPSC::GPSC() :EthernetClient() {

};

GPSC::GPSC(char *portaddress, char *port) :EthernetClient(portaddress, port) {

};

GPSC::~GPSC() {

};

int GPSC::GetData() {
	GPSdata GPSRecord;
	unsigned char *BytePtr;
	unsigned char Byte;
	unsigned char	Buffer[128] = { 0xaa, 0x44, 0x12, 0x1c };
	unsigned long	MyCRC;
	unsigned int	Header = 0xaa44121c;

		GPSRecord.Sync = 0;
		do {
			if (!(recv(ConnectSocket, (char *)&Byte, (int)sizeof(Byte), 0)))
			{
				std::cout << "Failed to read" << std::endl;
				return 2;
			}
			GPSRecord.Sync <<= 8;
			GPSRecord.Sync |= Byte;
		} while (GPSRecord.Sync != Header);

		BytePtr = ((unsigned char *)&GPSRecord);
		BytePtr += sizeof(GPSRecord.Sync);
		for (int i = 0; i < 108; i++) {
			recv(ConnectSocket, (char *)&Byte, (int)sizeof(Byte), 0);
			Buffer[i + 4] = *BytePtr++ = Byte; 
		}

		MyCRC = CalculateBlockCRC32(108, Buffer);

		if ((MyCRC == GPSRecord.CRC)) {
			Northing = GPSRecord.Northing;
			Easting = GPSRecord.Easting;
			Height = GPSRecord.Height;
			return 0;
		}
		else {
			return 1;
		}
};

long GPSC::CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
};

// Calculates the CRC-32 of a block of data all at once

unsigned long GPSC::CalculateBlockCRC32(unsigned long ulCount, unsigned char *ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
};

//Note: Data Block ucBuffer should contain all data bytes of the full data record except the checksum bytes.
