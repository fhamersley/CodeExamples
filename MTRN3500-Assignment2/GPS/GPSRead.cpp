#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <time.h>
#include <serialcomms.h>

#pragma pack(1)

using namespace std;

#define CRC32_POLYNOMIAL	0XEDB88320L
#define PI					3.14159265358
#define SIZE				100000
#define GPS_OFFSET			0.230
#define TWIN_GPS_SEPARATION	1.45 // meters?

struct GPS {
	/*message header structure */
	unsigned int FullHeader;		/*#1-4, header bytes 0xAA, 0x44, 0x12, 0x1C (number of header bytes)*/
	unsigned char Rejects1[9];		/*#5-10, not needed*/
	unsigned char GPSQuality;		/*#11, time status*/
	unsigned short Week;			/*#12, GPS w eek number*/
	unsigned long TimeStamp;		/*#13, Milliseconds from the beginning of GPS week*/
	unsigned char Rejects2[24];		/*#14-16 header, #1-5 BestUTM*/
									/*Best Avaliable UTM Data*/
	double Northing;				/*#6, lattitude */
	double Easting;					/*#7, longitude */
	double Height;					/*#8, above mean sea level*/
	unsigned char Rejects3[40];		/*#9-24*/
	unsigned long CRC;				/*#25, Chain Reduction Cycle*/
};

/* from CRCGeneratin.cpp START */
unsigned long CRC32Value(int i)
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
}
/* --------------------------------------------------------------------------
Calculates the CRC-32 of a block of data all at once
-------------------------------------------------------------------------- */
unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char *ucBuffer) /* Data block */
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
}
/* from CRCGeneratin.cpp END*/

int main(int argc, char**argv) {
	
	GPS GPSRecord;

	unsigned char *BytePtr;
	unsigned char Byte;
	unsigned char Buffer[128] = { 0xAA, 0x44, 0x12, 0x1C };
	unsigned long MyCRC;
	unsigned long CRC;
	unsigned long i;
	unsigned long numBytes;
	unsigned long GPSHeader = 0xAA44121C;
	unsigned long OldTimeStamp = 0;
	int ErrorCount = 0;
	_int64 Start, End, Frequency;
	double PCTimeStamp;
	HANDLE hCom;

	/* get HPC frequency */
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);

	/* open a handle for the specified 9com Port */
	hCom = OpenSerialPort(TEXT("//./COM1"), CBR_115200, 8, NOPARITY, ONESTOPBIT);

	if ((hCom == INVALID_HANDLE_VALUE) || (hCom == NULL)) {
		cout << "could not open port 1" << endl;
		_getch();
		return 1;
	}
	
	while (!_kbhit()) {
		/* get HPC at the start of new data recv attempt */
		QueryPerformanceCounter((LARGE_INTEGER*)&Start);

		GPSRecord.FullHeader = 0x00;
		do {
			if (!(ReadFile(hCom, &Byte, 1, &numBytes, NULL))) {
				cout << "Time out!" << endl;
				return 2;
			}
			GPSRecord.FullHeader <<= 8;
			GPSRecord.FullHeader |= Byte;
		} while (GPSRecord.FullHeader != GPSHeader);
	}
	BytePtr = ((unsigned char*)&GPSRecord);
	BytePtr += 4;
	for (int i = 0; i < 108; i++) {
		ReadFile(hCom, &Byte, 1, &numBytes, NULL);
		Buffer[i + 4] = *BytePtr++ = Byte;
	}

	MyCRC = CalculateBlockCRC32(108, Buffer);
	Sleep(25);
	QueryPerformanceCounter((LARGE_INTEGER*)&End);
	PCTimeStamp = (double)(End - Start) / (double)Frequency*1000.0; // ms
	if (GPSRecord.TimeStamp - OldTimeStamp > 50) {
		ErrorCount++;
	}
	OldTimeStamp = GPSRecord.TimeStamp;
	if (MyCRC == GPSRecord.CRC) { //&& MyCRCR ==GPSRecord.CRCR){
		printf("%10.3f\t %101d\t %10.3f\t !1.3f %d\n", PCTimeStamp, GPSRecord.TimeStamp);
	};
	cout << "terminating normally" << endl;

	CloseHandle(hCom);
	//CloseHandle(hComR);
	Sleep(1000);
	return 0;

};