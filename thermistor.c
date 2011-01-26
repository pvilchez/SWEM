#include <math.h> 
#define ECHO_TO_SERIAL 1 // echo data to serial port 
#include <Fat16.h> 
#include <Fat16util.h> // use functions to print strings from flash memory 

SdCard card; 
Fat16 file; 

//declarations 
double thermistorarray[200]; 
int i=0; 
int j=0; 
double minimum = 100; 
double maximum = 0; 
double average = 0; 
double sum = 0; 

double Thermister(int RawADC) { 
	double Temp; 

	Temp = (-0.0059 * pow (RawADC, 2)) + (1.5167*RawADC) - 18.419; 
	return Temp; 
} 

// store error strings in flash to save RAM 
#define error(s) error_P(PSTR(s)) 
void error_P(const char *str) 
{ 
	PgmPrint("error: "); 
	SerialPrintln_P(str); 
	
	if (card.errorCode) { 
		PgmPrint("SD error: "); 
		Serial.println(card.errorCode, HEX); 
	}
	 
	while(1); 
} 

void setup() { 
	Serial.begin(9600); 
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
		
		file.println(" Maxiumum "); 
		file.print(" Minimum "); 
		file.print(" Average");
		
		for (uint8_t k=0; k<3; k++){ 
			for(i=0; i<=9; i++){ 

				thermistorarray[i] =Thermister(analogRead(0)); 
				//Serial.println("iteration"); 
				Serial.print(i); 
				 
				Serial.println("data:"); 
				Serial.println(thermistorarray[i]); 
				minimum = min(thermistorarray[i], minimum); 
				maximum = max(thermistorarray[i], maximum); 
				sum = thermistorarray[i]+sum; 
				delay(1000); 
			} 

			average = sum/10; 
			sum = 0; 
			//write this data to the file Thermistor.csv 
			// Maximum value 
			file.print(maximum, DEC); 
		#if ECHO_TO_SERIAL 
			PgmPrint("Maximum"); 
			Serial.print(maximum); 
		#endif //ECHO_TO_SERIAL 
			//Minimum value 
			file.print(minimum, DEC); 
		#if ECHO_TO_SERIAL 
			Serial.println("Minimum"); 
			Serial.print(minimum); 
		#endif //ECHO_TO_SERIAL 
			//Average value 
			file.print(average, DEC); 
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

void loop(void){ 
} 



