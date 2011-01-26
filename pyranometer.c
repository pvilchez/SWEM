/* 
Analog Input

Demonstrates analog input by reading an analog sensor on analog pin 3 and 
turning on and off a light emitting diode(LED) connected to digital pin 13. 
The amount of time the LED will be on and off depends on 
the value obtained by analogRead().

The circuit: 
* Pyranometer attached to analog input 3 
* Green Wire to the analog pin 3 
* White Wire to Input Power +3.3V 
* Clear Wire to ground 
*/ 

#include <math.h> 
#define ECHO_TO_SERIAL 1 // echo data to serial port 
#include <Fat16.h> 
#include <Fat16util.h> // use functions to print strings from flash memory 
SdCard card;
Fat16 file;

double radiationarray[200]; 
int i=0; 
int j=0; 
double minimum = 100; 
double maximum = 0; 
double average = 0; 
double sum = 0; 
int sensorPin = 2; // select the input pin for the pyranometer 
int ledPin = 13; // select the pin for the LED 
double Value = 0; // variable to store the value coming from the sensor 
double Value_of_sensor = 0; 

double pyranometer(double RawADC){ 
	Value = RawADC/5; 
	Value_of_sensor = (0.0162*Value) - 0.0035; 
	return Value_of_sensor; 
} 

// store error strings in flash to save RAM 
#define error(s) error_P(PSTR(s)) 
void error_P(const char *str){ 
	PgmPrint("error: "); 
	SerialPrintln_P(str); 
	if (card.errorCode){ 
		PgmPrint("SD error: "); 
		Serial.println(card.errorCode, HEX); 
	} 
	while(1); 
} 

void setup() { 
	Serial.begin (9600); 
	Serial.println(); 
	PgmPrintln("Type any character to start"); 
	while (!Serial.available()); 
	
	// initialize the SD card 
	if (!card.init()) error("card.init"); 

	// initialize a FAT16 volume 
	if (!Fat16::init(card)) error("Fat16::init"); 

	char name[] = "append.txt"; 
	PgmPrint("Appending to: "); 
	Serial.println(name); 
	
	// clear write error 
	file.writeError = false; 

	for (uint8_t j = 0; j < 1; j++) { 
		// O_CREAT - create the file if it does not exist 
		// O_APPEND - seek to the end of the file prior to each write 
		// O_WRITE - open for write 

		if (!file.open(name, O_CREAT | O_APPEND | O_WRITE)) error("open");

		for (uint8_t k=0; k<3; k++){
			for(i = 0; i<300; i++){
				radiationarray[i] = pyranometer(analogRead(sensorPin));
				 
				Serial.print("Pyranometer value = ");
				Serial.println(Value);
				minimum = min(radiationarray[i], minimum);
				maximum = max(radiationarray[i], maximum);
				sum = radiationarray[i]+sum;

				delay(3000);
			}
			
			average = sum/4; 
			sum = 0;
		
		//write this data to the file Thermistor.csv 
			// Maximum value 
			file.print(maximum); 
		#if ECHO_TO_SERIAL 
			PgmPrint("Maximum"); 
			Serial.print(maximum); 
		#endif //ECHO_TO_SERIAL 

			//Minimum value 
			file.print(minimum); 
		#if ECHO_TO_SERIAL 
			Serial.println("Minimum"); 
			Serial.print(minimum); 
		#endif //ECHO_TO_SERIAL 
			//Average value 
			file.print(average); 
		#if ECHO_TO_SERIAL 
			Serial.println("Average"); 
			Serial.print(average); 
		#endif //ECHO_TO_SERIAL 

			if (file.writeError) error("write"); 
		}

		if (!file.close()) error("close"); 
		if (j > 0 && j%25 == 0)Serial.println(); 
		Serial.print(’.’); 
	}

	Serial.println(); 
	Serial.println("Done"); 
}

void loop(){} 
