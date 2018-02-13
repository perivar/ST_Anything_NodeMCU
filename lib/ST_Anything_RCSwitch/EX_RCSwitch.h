//******************************************************************************************
//  File: EX_RCSwitch.h
//  Author: Dan G Ogorchock
//
//  Summary:  EX_RCSwitch is a class which implements the SmartThings "Switch" device capability.
//			  It inherits from the st::Executor class.
//
//			  Create an instance of this class in your sketch's global variable section
//			  For Example:
//                static st::EX_RCSwitch executor1(F("switch1"), PIN_RCSWITCH, 35754004, 26, 18976788, 26, 1, 15, LOW);
//                static st::EX_RCSwitch executor4(F("switch2"), PIN_RCSWITCH, 79107, 24, 79116, 24, 1, 4, LOW, 189);
//            or
//			      static st::EX_RCSwitch executor3(F("switch3"), PIN_RCSWITCH, "0000011010100110100101100110010110101010100110101010", "0000011010100110100101100110010110101010100101010101", 9, 4);
//                static st::EX_RCSwitch executor3(F("switch4"), PIN_RCSWITCH, "000000010011010100000011", "000000010011010100001100", 1, 4, LOW, 189);
//
//			  st::EX_RCSwitch() have four constructors.
//			  The first requires the following arguments:
//				- String &name - REQUIRED - the name of the object - must match the Groovy ST_Anything DeviceType tile name  (e.g. switch3)
//				- byte transmitterPin - REQUIRED - the Arduino Pin to be used as a digital output for the RCSwitch object's transmitter pin
//				- unsigned long onCode - REQUIRED - the "on" code for RCSwitch send() command
//				- unsigned int onLength - REQUIRED - the "on" code's length for RCSwitch send() command
//				- unsigned long offCode - REQUIRED - the "off" code for RCSwitch send() command
//				- unsigned int offLength - REQUIRED - the "off" code's length for RCSwitch send() command
//				- byte protocol - OPTIONAL - defaults to "1" - the protocol for RCSwitch send() command
//				- byte repeatTransmits - OPTIONAL - defaults to "4" - the number of repeated transmits for RCSwitch send() command
//				- bool startingState - OPTIONAL - defaults to "LOW = off" - the value desired for the initial state of the switch.  LOW = "off", HIGH = "on"
//				- unsigned int pulseLength - OPTIONAL -  defaults to 0 (which means use the defined protocol's own pulse-length) - the length of the RF pulse for RCSwitch send() command
//
//			  The second supports Bit Strings (and therefore more devices) and requires the following arguments:
//              Requires the new RCSwitch library: https://github.com/perivar/rc-switch
//				- String &name - REQUIRED - the name of the object - must match the Groovy ST_Anything DeviceType tile name  (e.g. switch3)
//				- byte transmitterPin - REQUIRED - the Arduino Pin to be used as a digital output for the RCSwitch object's transmitter pin
//				- const char *onBitString - REQUIRED - the "on" bitstring for RCSwitch send() command
//				- const char *offBitString - REQUIRED - the "off" bitstring for RCSwitch send() command
//				- byte protocol - OPTIONAL - defaults to "1" - the protocol for RCSwitch send() command
//				- byte repeatTransmits - OPTIONAL - defaults to "4" - the number of repeated transmits for RCSwitch send() command
//				- bool startingState - OPTIONAL - defaults to "LOW = off" - the value desired for the initial state of the switch.  LOW = "off", HIGH = "on"
//				- unsigned int pulseLength - OPTIONAL -  defaults to 0 (which means use the defined protocol's own pulse-length) - the length of the RF pulse for RCSwitch send() command
//  Change History:
//
//    Date        Who            What
//    ----        ---            ----
//    2015-01-26  Dan Ogorchock  Original Creation
//    2015-05-20  Dan Ogorchock  Improved to work with Etekcity ZAP 3F 433Mhz RF Outlets
//    2018-08-30  Dan Ogorchock  Modified comment section above to comply with new Parent/Child Device Handler requirements
//	  2018-02-04  P.I. Nerseth	 Changed it to work with Bit Strings and a new RCSwitch library (and thus support more devices)
//	  2018-02-13  P.I. Nerseth	 Changed it to work with Bit Strings and optional Pulse Length (based on input from lehighkid)
//
//******************************************************************************************
#ifndef ST_EX_RCSWITCH
#define ST_EX_RCSWITCH

#include <RCSwitch.h>
#include "Executor.h"

namespace st
{
class EX_RCSwitch : public Executor
{
  private:
	bool m_bCurrentState;		 //HIGH or LOW
	byte m_nPin;				 //Arduino Pin used as a RC Transmitter
	RCSwitch m_myRCSwitch;		 //RCSwitch Object
	int m_nProtocol;			 //RCSwitch Protocol Number
	int m_nRepeatTransmit;		 //RCSwitch Number of Repeats when sending a signal
	const char *m_onBitString;   //RCSwitch On Bit String
	const char *m_offBitString;  //RCSwitch Off Bit String
	unsigned long m_onCode;		 //RCSwitch On Code (if not using bit string)
	unsigned int m_onLength;	 //RCSwitch On Length (if not using bit string)
	unsigned long m_offCode;	 //RCSwitch Off Code (if not using bit string)
	unsigned int m_offLength;	//RCSwitch Off Length (if not using bit string)
	unsigned int m_nPulseLength; //RCSwitch Pulse Length

	void writeStateToPin(); //function to update the Arduino digital output pin via RCSwitch switchOn and switchOff commands

  public:
	//constructor - called in your sketch's global variable declaration section
	EX_RCSwitch(const __FlashStringHelper *name, byte transmitterPin, unsigned long onCode, unsigned int onLength, unsigned long offCode, unsigned int offLength, byte protocol = 1, byte repeatTransmits = 4, bool startingState = LOW, unsigned int pulseLength = 0);
	// new constructor that supports bit strings (with the new RCSwitch library: https://github.com/perivar/rc-switch)
	EX_RCSwitch(const __FlashStringHelper *name, byte transmitterPin, const char *onBitString, const char *offBitString, byte protocol = 1, byte repeatTransmits = 4, bool startingState = LOW, unsigned int pulseLength = 0);

	//destructor
	virtual ~EX_RCSwitch();

	//initialization routine
	virtual void init();

	//SmartThings Shield data handler (receives command to turn "on" or "off" the switch (digital output)
	virtual void beSmart(const String &str);

	//called periodically to ensure state of the switch is up to date in the SmartThings Cloud (in case an event is missed)
	virtual void refresh();

	//gets
	virtual byte getPin() const
	{
		return m_nPin;
	}

	//sets
	virtual void setPin(byte pin);
};
}

#endif
