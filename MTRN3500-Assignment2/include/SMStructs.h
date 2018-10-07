#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

struct GPS
{
	double Northing;
	double Easting;
	double Height;
	double Heading;
	unsigned char GPSQuality;
	double GPSTimeStamp;
};

struct Laser
{
	double x[1100];
	double y[1100];
	double NumberPoints;
	double LaserTimeStamp;
};

struct Plotting
{
	double X;
	double Y;
	double Heading;
	double Steering;
};

struct UnitFlags
{
	unsigned short PM : 1,		// Process management
		Laser : 1,				// Laser
		Plot : 1,				// Plotting
		Xbox : 1,				// Xbox controller
		Simulator : 1,			// Simulator
		Vehicle : 1,			// Vehicle Control
		GPS : 1,				// GPS
		SM : 1,					// Shared memory
		HLevel : 1,				// High level control
		Unused : 7;				// 7 unused bits
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned short Status;
};

struct ProcessManagement
{
	ExecFlags HeartBeat;
	ExecFlags ShutDown;
	double PMTimeStamp;
};
 
struct Xbox
{
	double SetSteering;
	double SetSpeed;
	unsigned char VC_SIM;		// Choose VC or sim
	int Terminate;
	int Button;
	double XboxTimeStamp;
};

struct Simulator
{
	double X;
	double Y;
	double Heading;
	double SimTimeStamp;
};

struct VehicleControl
{
	double X;
	double Y;
	double Heading;
	double VCTimeStamp;
};

#endif