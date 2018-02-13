//******************************************************************************************
//  File: EX_RCSwitch.cpp
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
#include "EX_RCSwitch.h"

#include "Constants.h"
#include "Everything.h"

namespace st
{
//private
void EX_RCSwitch::writeStateToPin()
{
	if (st::Executor::debug)
	{
		Serial.print(F("EX_RCSwitch::writeStateToPin Transmitting on pin: "));
		Serial.print(m_nPin);
		Serial.print(F(", protocol: "));
		Serial.print(m_nProtocol);
		Serial.print(F(", pulse-length: "));
		if (m_nPulseLength > 0)
		{
			Serial.print(m_nPulseLength);
		}
		else
		{
			Serial.print(F("using protocol pulse-length"));
		}
		Serial.print(F(", repeats: "));
		Serial.print(m_nRepeatTransmit);
		if (strlen(m_onBitString) > 0)
		{
			Serial.print(F(", on: "));
			Serial.print(m_onBitString);
		}
		else
		{
			Serial.print(F(", on: "));
			Serial.print(m_onCode);
			Serial.print(F(", length: "));
			Serial.print(m_onLength);
		}
		if (strlen(m_offBitString) > 0)
		{
			Serial.print(F(", off: "));
			Serial.print(m_offBitString);
		}
		else
		{
			Serial.print(F(", off: "));
			Serial.print(m_offCode);
			Serial.print(F(", length: "));
			Serial.print(m_offLength);
		}
		Serial.println();
	}

	// Note! For some reason I have to always enable transmit for this to work!
	m_myRCSwitch.enableTransmit(m_nPin);

	if (m_bCurrentState)
	{
		if (strlen(m_onBitString) > 0)
		{
			m_myRCSwitch.send(m_onBitString);
		}
		else
		{
			m_myRCSwitch.send(m_onCode, m_onLength);
		}
	}
	else
	{
		if (strlen(m_offBitString) > 0)
		{
			m_myRCSwitch.send(m_offBitString);
		}
		else
		{
			m_myRCSwitch.send(m_offCode, m_offLength);
		}
	}
}

//public

//constructor
EX_RCSwitch::EX_RCSwitch(const __FlashStringHelper *name, byte transmitterPin, unsigned long onCode, unsigned int onLength, unsigned long offCode, unsigned int offLength, byte protocol, byte repeatTransmits, bool startingState, unsigned int pulseLength) : Executor(name),
																																																																m_myRCSwitch(RCSwitch()),
																																																																m_onCode(onCode),
																																																																m_onLength(onLength),
																																																																m_offCode(offCode),
																																																																m_offLength(offLength),
																																																																m_nProtocol(protocol),
																																																																m_nRepeatTransmit(repeatTransmits),
																																																																m_bCurrentState(startingState),
																																																																m_nPulseLength(pulseLength)
{
	setPin(transmitterPin);
	m_myRCSwitch.setProtocol(protocol);				 // set protocol (default is 1, will work for most outlets)
	m_myRCSwitch.setRepeatTransmit(repeatTransmits); // set number of transmission repetitions.
	if (pulseLength > 0)
	{
		m_myRCSwitch.setPulseLength(pulseLength); // Set pulse length.
	}
}

// new constructor that supports bit strings (with the new RCSwitch library: https://github.com/perivar/rc-switch)
EX_RCSwitch::EX_RCSwitch(const __FlashStringHelper *name, byte transmitterPin, const char *onBitString, const char *offBitString, byte protocol, byte repeatTransmits, bool startingState, unsigned int pulseLength) : Executor(name),
																																																					   m_myRCSwitch(RCSwitch()),
																																																					   m_onBitString(onBitString),
																																																					   m_offBitString(offBitString),
																																																					   m_nProtocol(protocol),
																																																					   m_nRepeatTransmit(repeatTransmits),
																																																					   m_bCurrentState(startingState),
																																																					   m_nPulseLength(pulseLength)
{
	setPin(transmitterPin);
	m_myRCSwitch.setProtocol(protocol);				 // set protocol (default is 1, will work for most outlets)
	m_myRCSwitch.setRepeatTransmit(repeatTransmits); // set number of transmission repetitions.
	if (pulseLength > 0)
	{
		m_myRCSwitch.setPulseLength(pulseLength); // Set pulse length.
	}
}

//destructor
EX_RCSwitch::~EX_RCSwitch()
{
}

void EX_RCSwitch::init()
{
	writeStateToPin();
	Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
}

void EX_RCSwitch::beSmart(const String &str)
{
	String s = str.substring(str.indexOf(' ') + 1);
	if (st::Executor::debug)
	{
		Serial.print(F("EX_RCSwitch::beSmart s = "));
		Serial.println(s);
	}
	if (s == F("on"))
	{
		m_bCurrentState = HIGH;
	}
	else if (s == F("off"))
	{
		m_bCurrentState = LOW;
	}

	writeStateToPin();

	Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
}

void EX_RCSwitch::refresh()
{
	Everything::sendSmartString(getName() + " " + (m_bCurrentState == HIGH ? F("on") : F("off")));
}

void EX_RCSwitch::setPin(byte pin)
{
	m_nPin = pin;
	m_myRCSwitch.enableTransmit(m_nPin);
}
}
