#ifndef GPS_H
#define GPS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define CRC32_POLYNOMIAL			0xEDB88320L

#include <EthernetClient.h>

#pragma pack(1)

struct GPSdata
{
	// Header bytes 0xAA, 0x44, 0x12, 0x1C (number of header bytes)
	unsigned		Sync;
	unsigned char	Empty[9];
	unsigned char	GPSQuality;
	unsigned short	Week;
	unsigned long	TimeStamp; //4 bytes
	unsigned char	Empty2[24];
	double			Northing;
	double			Easting;
	double			Height;
	unsigned char	Empty3[40];
	unsigned long	CRC;
};

class GPSC : public EthernetClient {
public:
	double Northing;
	double Easting;
	double Height;
public:
	GPSC();
	GPSC(char *portaddress, char *port);
	~GPSC();
	int GetData();
	long CRC32Value(int i);
	unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char *ucBuffer);
};

#endif // GPS_H