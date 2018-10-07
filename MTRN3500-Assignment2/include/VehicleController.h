#ifndef VEHICLECONTROLLER_H
#define VEHICLECONTROLLER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <EthernetClient.h>

class VehicleController : public EthernetClient
{
private:
	double SetSpeed;
	double SetSteering;
	int Flag;
public:
	VehicleController();
	VehicleController(char * portaddress, char* port);
	~VehicleController();
	void SetSetSpeed(double setSpeed);
	void SetSetSteering(double setSteering);
	double GetSpeed();
	double GetSteering();
	void Drive(double speed, double steering);
};

#endif // !VEHICLECONTROLLER_H
