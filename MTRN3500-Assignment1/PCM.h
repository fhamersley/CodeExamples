#ifndef PCM_H
#define PCM_H

//PCM Class for PCM-3718 multi-function interface card

// Start of PCM class definition 
class PCM
{
private:		//Private members
	//DATA
	int BASE;
	bool BaseSetFlag;		//Flag for checking if base is set.
	int Data[2];		//Storage of current Data[0] for low byte and Data[1] for high byte 

public:			//Public members
	//CONTRUCTORS
	PCM();					//Default contructor
	PCM(int BaseInput);		//Contructor with base set

	//DESTRUCTOR
	~PCM();					//Default destructor

	//FUNCTIONS
	void CheckIO();
	void SetBase(int BaseInput);
	bool BaseSet();
	int GetBase();
	int GetData(bool HighLow);
	void SetByte(bool HighLow, int Output);
	void SetBit(bool HighLow, int BitNum);
	int GetByte(bool HighLow);
	int GetBit(bool HighLow, int BitNum);
	int AnalogInput(int Range, int ChannelNum) const;
	friend std::ostream &operator<<(std::ostream &os, PCM const &p);
};

#endif
