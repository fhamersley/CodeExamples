#define _USE_MATH_DEFINES
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <SMObject.h>
#include <SMStructs.h>
#include <LMS151.h>

using namespace std;

LMS151::LMS151() : EthernetClient()
{
	StartAngle = -45;
	EndAngle = 225;
	Resolution = 0.5;
}

LMS151::LMS151(char * portaddress, char* port) : EthernetClient(portaddress, port)
{
	StartAngle = -45;
	EndAngle = 225;
	Resolution = 0.5;
}


LMS151::~LMS151()
{
}

void LMS151::Login()
{
	string Fragments;
	string LoginResp;
	int Return;
	char Login[256] = "sMN SetAccessMode 03 F4724744";		// Sets login code and pw
	if (SendData(Login, strlen(Login)) == 0)				//SendData return 0 if completes action otherwise sets 'Error' to corresponding error defined in EthCli
	{
		ReceiveData();										// Catches return
		LoginResp = RxData;
		istringstream is(LoginResp);
		is >> Fragments >> Fragments >> hex >> Return;
		if (Return == 0)
			Error = LOGIN_FAIL;
		else
			Error = NOERROR;
	}
}

void LMS151::Logout()
{
	string Fragments;
	string LogoutResp;
	int Return;
	char Logout[256] = "sMN Run";
	SendData(Logout, strlen(Logout));	// SendData return 0 if completes action otherwise sets 'Error' to corresponding error defined in EthCli
	//Sleep(1000);
	ReceiveData();
	LogoutResp = RxData;
	istringstream is(LogoutResp);
	is >> Fragments >> Fragments >> hex >> Return;
	if (Return == 0) {
		Error = LOGOUT_FAIL;
		cout << "LOGOUT FAIL" << endl;
	}
	else {
		Error = NOERROR;
		cout << "NO ERROR" << endl;
	}
	
}

void LMS151::Configure(int startAngle, int endAngle, double resolution)
{
	string Fragments;
	string ConfigResp;
	int Return;
	char SetFreqRange[256]; // = "sMN mLMPsetscansfg"
	if ((startAngle >= -45) && (startAngle < 225) && (endAngle > -45) && (endAngle <= 225) && (startAngle < endAngle))
	{
		StartAngle = startAngle;
		EndAngle = endAngle;
		Resolution = resolution;
		sprintf_s(SetFreqRange, "sMN mLMPsetscansfg 5000 1 %4d %d %d", (int)Resolution, StartAngle * 10000, EndAngle * 10000);
		if (SendData(SetFreqRange, strlen(SetFreqRange)) == 0)		// SendData return 0 if completes action otherwise sets 'Error' to corresponding error defined in EthCli
		{
			ReceiveData();
			ConfigResp = RxData;
			istringstream is(ConfigResp);
			is >> Fragments >> Fragments >> hex >> Return;
			if (Return == 0)
				Error = NOERROR;
			else
				Error = SCAN_CONFIG_ERROR;
		}
	}
	else
		Error = SCAN_CONFIG_ERROR;
}

void LMS151::GetRangeCoords()
{
	double Angle = StartAngle;
	unsigned int Range;
	string Fragments;
	string ScanResp;
	//string ScanResp = "sRA LMDscandata 0 1 9BF210 0 0 F7F7 9276 D5EC0726 D5EC0CD2 0 0 7 0 0 1388 168 0 1 DIST1 3F800000 00000000 6DDD0 1388 B5 AA3 BC0 C69 CD0 CEE CCB 19D9 19A2 1993 BB1 BC0 BB8 B7A B99 191F 177E 1772 163A 15FF 166E 1675 14C8 1448 1415 13F6 13C6 13AE 139B 1373 1349 133A 131C 12FA 12E3 12C1 12B7 1291 127A 1266 1244 1233 1216 1202 11ED 11DC 11D3 11B1 1199 118F 1177 1175 1159 1159 1132 1118 111A 1112 10FC 10E1 10DC 10D9 10D3 10B3 10AD 10B2 1076 105A 1061 1045 106B 1069 10B5 10C7 10B9 10B3 10B1 10B0 109F 108C 1099 108B 377 360 359 351 349 352 34E 355 106C 105A 1067 104F 1054 104E 1054 1054 104D 104A 1047 1043 1042 1041 104D 1050 1056 105C 1049 104D 1051 106A 105B 1063 1069 1064 1052 107D 1075 108C 1087 FE4 EE5 EB6 EBB F09 F22 F49 F5C F39 F62 FD0 FB9 FAB FA4 F98 F68 F50 F7E FEF FFB FE0 F85 E47 FAB FCC F7D F56 F52 F6B FD1 109D 13E9 13FF 143A 1442 1453 1470 1465 145D 1460 1481 10B2 FCC FA7 FBB FD5 1068 0 0 115E 11B2 1191 116A 1181 FBA E6C E79 E66 E5B EA3 17A6 0 0 0 0 0 0";
	char ScanOnce[256] = "sRN LMDscandata";
	if (SendData(ScanOnce, strlen(ScanOnce)) == 0)		// SendData return 0 if completes action otherwise sets 'Error' to corresponding error defined in EthCli
	{
		SendData(ScanOnce, strlen(ScanOnce));
		//cout << "1" << endl;
		ReceiveData();
		//cout << "2" << endl;
		ScanResp = RxData;
		istringstream is(ScanResp);
		for (int i = 0; i < 25; i++)
			is >> Fragments;
		is >> hex >> NumPoints;
		if (NumPoints > 0)
		{
			for (int i = 0; i < NumPoints; i++)
			{
				is >> hex >> Range;
				RangeData[i][0] = Range / 1000.0*cos(Angle*M_PI / 180.0);
				RangeData[i][1] = -(Range / 1000.0*sin(Angle*M_PI / 180.0));
				Angle += Resolution;
			}
		}
	}
}