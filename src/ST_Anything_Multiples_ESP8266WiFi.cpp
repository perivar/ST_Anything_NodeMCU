//******************************************************************************************
//  File: ST_Anything_Multiples_ESP8266WiFi.ino
//  Authors: Dan G Ogorchock & Daniel J Ogorchock (Father and Son)
//
//  Summary:  This Arduino Sketch, along with the ST_Anything library and the revised SmartThings
//            library, demonstrates the ability of one NodeMCU ESP8266 to
//            implement a multi input/output custom device for integration into SmartThings.
//            The ST_Anything library takes care of all of the work to schedule device updates
//            as well as all communications with the NodeMCU ESP8266's WiFi.
//
//            ST_Anything_Multiples implements the following ST Capabilities as a demo of what is possible with a single NodeMCU ESP8266
//              - 1 x Alarm device (using a simple digital output)
//              - 1 x Contact Sensor devices (used to monitor magnetic door sensors)
//              - 1 x Switch devices (used to turn on a digital output (e.g. LED, relay, etc...)
//              - 1 x Motion devices (used to detect motion)
//              - 1 x Smoke Detector devices (using simple digital input)
//              - 1 x Temperature Measurement devices (Temperature from Dallas Semi 1-Wire DS18B20 device)
//              - 1 x Relay Switch devices (used to turn on a digital output for a set number of cycles And On/Off times (e.g.relay, etc...))
//              - 2 x Button devices (sends "pushed" if held for less than 1 second, else sends "held"
//              - 1 x Water Sensor devices (using the 1 analog input pin to measure voltage from a water detector board)
//
//  Change History:
//
//    Date        Who            What
//    ----        ---            ----
//    2015-01-03  Dan & Daniel   Original Creation
//    2017-02-12  Dan Ogorchock  Revised to use the new SMartThings v2.0 library
//    2017-04-17  Dan Ogorchock  New example showing use of Multiple device of same ST Capability
//                               used with new Parent/Child Device Handlers (i.e. Composite DH)
//    2017-05-25  Dan Ogorchock  Revised example sketch, taking into account limitations of NodeMCU GPIO pins
//    2018-02-09  Dan Ogorchock  Added support for Hubitat Elevation Hub
//
//******************************************************************************************
#include <SPI.h>	 // Adafruit MAX31855 library requires SPI.h
#include "secrets.h" // include this file for external parameters like WIFI password etc.

//******************************************************************************************
// SmartThings Library for ESP8266WiFi
//******************************************************************************************
#include <SmartThingsESP8266WiFi.h>

//******************************************************************************************
// ST_Anything Library
//******************************************************************************************
#include <Constants.h>		 //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>			 //Generic Device Class, inherited by Sensor and Executor classes
#include <Sensor.h>			 //Generic Sensor Class, typically provides data to ST Cloud (e.g. Temperature, Motion, etc...)
#include <Executor.h>		 //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <InterruptSensor.h> //Generic Interrupt "Sensor" Class, waits for change of state on digital input
#include <PollingSensor.h>   //Generic Polling "Sensor" Class, polls Arduino pins periodically
#include <Everything.h>		 //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications

#include <PS_Illuminance.h> //Implements a Polling Sensor (PS) to measure light levels via a photo resistor

#include <PS_TemperatureHumidity.h> //Implements a Polling Sensor (PS) to measure Temperature and Humidity via DHT library
#include <PS_DS18B20_Temperature.h> //Implements a Polling Sesnor (PS) to measure Temperature via DS18B20 libraries
#include <PS_Water.h>				//Implements a Polling Sensor (PS) to measure presence of water (i.e. leak detector)
#include <IS_Motion.h>				//Implements an Interrupt Sensor (IS) to detect motion via a PIR sensor
#include <IS_Contact.h>				//Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin
#include <IS_Smoke.h>				//Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin
#include <IS_DoorControl.h>			//Implements an Interrupt Sensor (IS) and Executor to monitor the status of a digital input pin and control a digital output pin
#include <IS_Button.h>				//Implements an Interrupt Sensor (IS) to monitor the status of a digital input pin for button presses
#include <EX_Switch.h>				//Implements an Executor (EX) via a digital output to a relay
#include <EX_Alarm.h>				//Implements Executor (EX)as an Alarm Siren capability via a digital output to a relay
#include <S_TimedRelay.h>			//Implements a Sensor to control a digital output pin with timing capabilities
#include <EX_RCSwitch.h>			//Implements the Executor (EX) as an RCSwitch object

//*************************************************************************************************
//NodeMCU v1.0 ESP8266-12e Pin Definitions (makes it much easier as these match the board markings)
//*************************************************************************************************
//#define LED_BUILTIN 16
//#define BUILTIN_LED 16
//
//#define D0 16  //no internal pullup resistor
//#define D1  5
//#define D2  4
//#define D3  0  //must not be pulled low during power on/reset, toggles value during boot
//#define D4  2  //must not be pulled low during power on/reset, toggles value during boot
//#define D5 14
//#define D6 12
//#define D7 13
//#define D8 15  //must not be pulled high during power on/reset

//******************************************************************************************
//Define which Arduino Pins will be used for each device
//******************************************************************************************
#define PIN_WATER_1 A0 //NodeMCU ESP8266 only has one Analog Input Pin 'A0'

#define PIN_ALARM_1 D0		 //SmartThings Capabilty "Alarm"
#define PIN_SWITCH_1 D1		 //SmartThings Capability "Switch"
#define PIN_CONTACT_1 D2	 //SmartThings Capabilty "Contact Sensor"
#define PIN_BUTTON_1 D3		 //SmartThings Capabilty Button / Holdable Button (Normally Open!)
#define PIN_BUTTON_2 D4		 //SmartThings Capabilty Button / Holdable Button (Normally Open!)
#define PIN_MOTION_1 D5		 //SmartThings Capabilty "Motion Sensor" (HC-SR501 PIR Sensor)
#define PIN_SMOKE_1 D6		 //SmartThings Capabilty "Smoke Detector"
#define PIN_TEMPERATURE_1 D7 //SmartThings Capabilty "Temperature Measurement" (Dallas Semiconductor DS18B20)
#define PIN_TIMEDRELAY_1 D8  //SmartThings Capability "Relay Switch"
#define PIN_RCSWITCH D9		 // SmartThings Capability "Switch" for RF433 devices. Pin 3 = D9/RX on nodemcu, Pin 1 = D10/TX on nodmcu

//******************************************************************************************
//ESP8266 WiFi Information
//******************************************************************************************
String str_ssid = WIFI_SSID;				 //WIFI SSID						// read from secrets.h
String str_password = WIFI_PASSWORD;		 //WIFI Password				// read from secrets.h
IPAddress ip(DEVICE_IP_ADDRESS);			 //Device IP Address		// read from secrets.h
IPAddress gateway(ROUTER_GATEWAY);			 //Router gateway				// read from secrets.h
IPAddress subnet(LAN_SUBNET);				 //LAN subnet mask			// read from secrets.h
IPAddress dnsserver(DNS_SERVER);			 //DNS server						// read from secrets.h
const unsigned int serverPort = SERVER_PORT; // port to run the http server on // read from secrets.h

// Smartthings / Hubitat Hub TCP/IP Address
IPAddress hubIp(HUB_IP_ADDRESS); // smartthings hub ip				// read from secrets.h
// SmartThings / Hubitat Hub TCP/IP Address: UNCOMMENT line that corresponds to your hub, COMMENT the other
const unsigned int hubPort = HUB_PORT; // smartthings or hubitat hub port (smartthings usually 39500, hubitat 39501)

//******************************************************************************************
//st::Everything::callOnMsgSend() optional callback routine.  This is a sniffer to monitor
//    data being sent to ST.  This allows a user to act on data changes locally within the
//    Arduino sktech.
//******************************************************************************************
void callback(const String &msg)
{
	//  Serial.print(F("ST_Anything Callback: Sniffed data = "));
	//  Serial.println(msg);

	//TODO:  Add local logic here to take action when a device's value/state is changed

	//Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line)
	//st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send
}

//******************************************************************************************
//Arduino Setup() routine
//******************************************************************************************
void setup()
{
	//******************************************************************************************
	//Declare each Device that is attached to the Arduino
	//  Notes: - For each device, there is typically a corresponding "tile" defined in your
	//           SmartThings Device Hanlder Groovy code, except when using new COMPOSITE Device Handler
	//         - For details on each device's constructor arguments below, please refer to the
	//           corresponding header (.h) and program (.cpp) files.
	//         - The name assigned to each device (1st argument below) must match the Groovy
	//           Device Handler names.  (Note: "temphumid" below is the exception to this rule
	//           as the DHT sensors produce both "temperature" and "humidity".  Data from that
	//           particular sensor is sent to the ST Hub in two separate updates, one for
	//           "temperature" and one for "humidity")
	//         - The new Composite Device Handler is comprised of a Parent DH and various Child
	//           DH's.  The names used below MUST not be changed for the Automatic Creation of
	//           child devices to work properly.  Simply increment the number by +1 for each duplicate
	//           device (e.g. contact1, contact2, contact3, etc...)  You can rename the Child Devices
	//           to match your specific use case in the ST Phone Application.
	//******************************************************************************************
	//Polling Sensors
	static st::PS_Water sensor1(F("water1"), 60, 20, PIN_WATER_1, 200);
	static st::PS_DS18B20_Temperature sensor2(F("temperature1"), 15, 0, PIN_TEMPERATURE_1, false, 10, 1);

	//Interrupt Sensors
	static st::IS_Contact sensor3(F("contact1"), PIN_CONTACT_1, LOW, true);
	static st::IS_Button sensor4(F("button1"), PIN_BUTTON_1, 1000, LOW, true, 500);
	static st::IS_Button sensor5(F("button2"), PIN_BUTTON_2, 1000, LOW, true, 500);
	static st::IS_Motion sensor6(F("motion1"), PIN_MOTION_1, HIGH, false);
	static st::IS_Smoke sensor7(F("smoke1"), PIN_SMOKE_1, HIGH, true, 500);

	//Special sensors/executors (uses portions of both polling and executor classes)
	static st::S_TimedRelay sensor8(F("relaySwitch1"), PIN_TIMEDRELAY_1, LOW, false, 3000, 0, 1);

	//Executors
	static st::EX_Alarm executor1(F("alarm1"), PIN_ALARM_1, LOW, true);
	static st::EX_Switch executor2(F("switch1"), PIN_SWITCH_1, LOW, true); //Inverted logic for "Active Low" Relay Board

	// Add support for RF 433 Switches:
	// Examples where Pulse Length is used
	//static st::EX_RCSwitch executor3(F("switch2"), PIN_RCSWITCH, "000000010011010100000011", "000000010011010100001100", 1, 4, LOW, 189);
	//static st::EX_RCSwitch executor4(F("switch3"), PIN_RCSWITCH, 79107, 24, 79116, 24, 1, 4, LOW, 189);

	// Examples where Pulse Length is not used
	static st::EX_RCSwitch executor3(F("switch2"), PIN_RCSWITCH, "0000011010100110100101100110010110101010100110101010", "0000011010100110100101100110010110101010100101010101", 9, 4);
	static st::EX_RCSwitch executor4(F("switch3"), PIN_RCSWITCH, "1001100101101010100101101010011001011001100110100110010110101010", "0", 8, 10);

	//*****************************************************************************
	//  Configure debug print output from each main class
	//  -Note: Set these to "false" if using Hardware Serial on pins 0 & 1
	//         to prevent communication conflicts with the ST Shield communications
	//*****************************************************************************
	st::Everything::debug = true;
	st::Executor::debug = true;
	st::Device::debug = true;
	st::PollingSensor::debug = true;
	st::InterruptSensor::debug = true;

	//*****************************************************************************
	//Initialize the "Everything" Class
	//*****************************************************************************

	//Initialize the optional local callback routine (safe to comment out if not desired)
	st::Everything::callOnMsgSend = callback;

	//Create the SmartThings ESP8266WiFi Communications Object
	//STATIC IP Assignment - Recommended
	bool debug = false;
	st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(str_ssid, str_password, ip, gateway, subnet, dnsserver, serverPort, hubIp, hubPort, st::receiveSmartString, "ESP8266Wifi", debug);

	//DHCP IP Assigment - Must set your router's DHCP server to provice a static IP address for this device's MAC address
	//st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(str_ssid, str_password, serverPort, hubIp, hubPort, st::receiveSmartString);

	//Run the Everything class' init() routine which establishes WiFi communications with SmartThings Hub
	st::Everything::init();

	//*****************************************************************************
	//Add each sensor to the "Everything" Class
	//*****************************************************************************
	st::Everything::addSensor(&sensor1);
	st::Everything::addSensor(&sensor2);
	st::Everything::addSensor(&sensor3);
	st::Everything::addSensor(&sensor4);
	st::Everything::addSensor(&sensor5);
	st::Everything::addSensor(&sensor6);
	st::Everything::addSensor(&sensor7);
	st::Everything::addSensor(&sensor8);

	//*****************************************************************************
	//Add each executor to the "Everything" Class
	//*****************************************************************************
	st::Everything::addExecutor(&executor1);
	st::Everything::addExecutor(&executor2);
	st::Everything::addExecutor(&executor3);
	st::Everything::addExecutor(&executor4);

	//*****************************************************************************
	//Initialize each of the devices which were added to the Everything Class
	//*****************************************************************************
	st::Everything::initDevices();
}

//******************************************************************************************
//Arduino Loop() routine
//******************************************************************************************
void loop()
{
	//*****************************************************************************
	//Execute the Everything run method which takes care of "Everything"
	//*****************************************************************************
	st::Everything::run();
}