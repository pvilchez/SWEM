#include <math.h> 
#define ECHO_TO_SERIAL 1 // echo data to serial port 
#include <Fat16.h> 
#include <Fat16util.h> // use functions to print strings from flash memory 

SdCard card; 
Fat16 file;
//declarations 
volatile byte count;

unsigned int duration; 
unsigned long timePrevious; 
float windSpeed = 0; 
int windVane = 1;

int i=0;
int j=0;
int k=0;
int m=0;

float speedarray[200];
float directionarray[200];
float directionarray_cos[200];
float directionarray_sin[200];

double minimum_wind = 1000;
double maximum_wind = 0;
double average_wind = 0;
double sdiv_wind = 0;
double num = 0;
double sum_wind = 0;

double minimum_dir = 1000;
double maximum_dir = 0;
double average_dir = 0;
double sdiv_dir = 0;
double temp;
double sum_dir_cos = 0;
double sum_dir_sin = 0;
double sum_dir = 0;

//Error checking 
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

void setup()
{
	Serial.begin(9600);
 
	count = 0;
	duration = 0;
	timePrevious = 0;
	attachInterrupt(1, arduino_anemometer, RISING);

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
	
	for (uint8_t i = 0; i < 1; i++) {
		// O_CREAT - create the file if it does not exist
		// O_APPEND - seek to the end of the file prior to each write
		// O_WRITE - open for write
		
		if (!file.open(name, O_CREAT | O_APPEND | O_WRITE)) error("open");
		
		for(j=0; j<=10; i++) {
			windVane = (analogRead(windVane));
			if (count >= 1) {
				duration = (millis() - timePrevious); 
				timePrevious = millis(); 
				count = 0; 
				windSpeed = (3000/duration); 
				speedarray[j] = windSpeed; 
			}

			//determining maximum, minimum wind speeds 
			maximum_wind = max(maximum_wind, windSpeed); 
			minimum_wind = min(minimum_wind, windSpeed); 
			sum_wind = speedarray[j]+sum_wind; 
			directionarray[j] = windVane;
	
			//breaking wind direction into its component parts 
			directionarray_cos[j] = cos(windVane); 
			directionarray_sin[j] = sin(windVane); 
			sum_dir_cos = directionarray_cos[i] + sum_dir_cos; 
			sum_dir_sin = directionarray_sin[i] + sum_dir_sin;
	 
			delay(3000); 
		}
	
		//determine average values for wind speed
		average_wind = sum_wind/200; 
		sum_wind = 0;
		
		// standard deviation of wind 
		for(k=0; k<=199; k++){ 
			num = (pow((speedarray[k]-average_wind), 2)+num); 
		}
		
		sdiv_wind = sqrt(num/200); 

		//determine average and standard deviation values for direction 
		average_dir = atan(sum_dir_sin/sum_dir_cos); 
		sum_dir_sin = 0; 
		sum_dir_cos = 0;
		
		//calculating maximum and minimum wind directions 
		for(k =0; k<=199; k++){ 
			maximum_dir = max(average_dir, directionarray[k]); 
			minimum_dir = min(average_dir, directionarray[k]); 
		} 

		//standard deviation of the direction 
		temp = average_dir - 180; 
		for(m=0; m<=199; m++){ 
			num = (pow ((directionarray[m] - temp), 2)+num); 
		} 
		sdiv_dir = sqrt(num/200); 

// Writing to a file 
// WIND DATA 
		// Maximum value 
		file.println("Maximum"); 
		file.print(maximum_wind); 
	#if ECHO_TO_SERIAL 
		Serial.print("Maximum"); 
		Serial.print(maximum_wind); 
	#endif //ECHO_TO_SERIAL
	
		//Minimum value 
		file.println("Minimum"); 
		file.print(minimum_wind); 
	#if ECHO_TO_SERIAL 
		Serial.println("Minimum"); 
		Serial.print(minimum_wind); 
	#endif //ECHO_TO_SERIAL 
		//Average value 
		file.println("Average"); 
		file.print(average_wind);
		
	#if ECHO_TO_SERIAL 
		Serial.println("Average"); 
		Serial.print(average_wind); 
	#endif //ECHO_TO_SERIAL 

	//Standard deviation value 
		file.println("Standard deviation"); 
		file.print(sdiv_wind); 
	#if ECHO_TO_SERIAL 
		Serial.println("Standard deviation"); 
		Serial.print(sdiv_wind); 
	#endif //ECHO_TO_SERIAL 

// WIND DIRECTION DATA 
	// Maximum value 
		file.println("Maximum"); 
		file.print(maximum_dir); 
	#if ECHO_TO_SERIAL 
		Serial.print("Maximum"); 
		Serial.print(maximum_dir); 
	#endif //ECHO_TO_SERIAL 
	
	//Minimum value 
		file.println("Minimum"); 
		file.print(minimum_dir); 
	#if ECHO_TO_SERIAL 
		Serial.println("Minimum"); 
		Serial.print(minimum_dir); 
	#endif //ECHO_TO_SERIAL 
	//Average value 
		file.println("Average"); 
		file.print(average_dir); 
	#if ECHO_TO_SERIAL 
		Serial.println("Average"); 
		Serial.print(average_dir);
	#endif //ECHO_TO_SERIAL	
	//Standard deviation value 
		file.println("Standard deviation"); 
		file.print(sdiv_dir); 
	#if ECHO_TO_SERIAL 
		Serial.println("Standard deviation"); 
		Serial.print(sdiv_dir); 
	#endif //ECHO_TO_SERIAL 
	 
		if (file.writeError) error("write"); 
	}
	 
	if (!file.close()) error("close"); 
	if (i > 0 && i%25 == 0)Serial.println(); 
	Serial.print(’.’); 
}

void loop(){} 

void arduino_anemometer(){ 
	count++;
} 
