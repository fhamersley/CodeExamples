#ifndef PC104_H
#define PC104_H

//PCM Class for PC104 analog output card

// Start of PCM class definition 
class PC104
{
private:		//Private members
				//DATA
	int BASE;

public:			//Public members
				//CONTRUCTORS
	PC104();					//Default contructor
	PC104(int BaseInput);		//Contructor with base set

							//DESTRUCTOR
	~PC104();					//Default destructor

							//FUNCTIONS
	void CheckIO();
	void SetBase(int BaseInput);
	bool BaseSet();
	int GetBase();
	void AnalogOutput(int ChannelNum, int VoltValue, bool Voltage);
};
#endif
