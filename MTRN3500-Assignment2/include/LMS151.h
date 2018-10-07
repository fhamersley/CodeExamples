#ifndef LMS151_H
#define LMS151_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <EthernetClient.h>

#define NOERROR 0
#define LOGIN_FAIL -1
#define LOGOUT_FAIL -2
#define SCAN_CONFIG_ERROR -3


#define HALF_DEG 5000
#define QUART_DEG 2500

class LMS151 : public EthernetClient
{
public:
	double RangeData[1000][2];
	int NumPoints;
private:
	int StartAngle;
	int EndAngle;
	double Resolution;
public:
	LMS151();
	LMS151(char * portaddress, char* port);
	~LMS151();
	void Login();
	void Logout();
	void Configure(int startAngle, int endAngle, double resolution);
	void GetRangeCoords();
};

#endif