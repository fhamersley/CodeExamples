#ifndef MSI_H
#define MSI_H

//MSI Class for MSI-404 encoder input interface card

// Start of MSI class definition 
class MSI
{
private:		//Private members
				//DATA
	int BASE;
	union encode {
		int a;
		char b[4];
	};

public:			//Public members
				//CONTRUCTORS
	MSI();					//Default contructor
	MSI(int BaseInput);		//Contructor with base set

								//DESTRUCTOR
	~MSI();					//Default destructor

	encode enc;				//DATA STORAGE
								//FUNCTIONS
	void CheckIO();
	void SetBase(int BaseInput);
	bool BaseSet();
	int GetBase();
	void ChannelReset(int ChannelNum);
	bool IndexCheck(int IndexBase);
	int ChannelRead(int ChannelNum);
	MSI operator!();
};

#endif
