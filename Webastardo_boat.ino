//*********************************************************************************************
// DIY Webasto Controller
// Board:  Adafruit Feather M0 Express board
// Add the following board link in preferences: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
// Code based on a Webasto Shower Controller by David McLuckie
// https://davidmcluckie.com/arduino-webasto-shower-project/
// His project was based on the work of Mael Poureau
// https://maelpoureau.com/webasto_shower/
//
// Simon changed it from a Shower to a general purpose controller which tries to regulate the
// Temperature to a pre-defined target by adjusting the Fueling and Combustion fan
// He converted the code to run on an Adafruit Feather M0 SAMD21 and designed a drop-in
// replacement PCB for Webasto ThermoTop C & E Heaters.
//
// The latest revision of the PCB (https://oshwlab.com/SimonRafferty/webasto-controller) (V3.0)
// has provision for a Thermal Fuse.  This simply cuts the fueling if the heater really overheats
// to prevent it catching fire.  I used RS Part Number 797-6042 which fuses at 121C
// If you prefer the excitement of waiting for it to catch fire, you can always bridge the
// contacts with a bit of wire.
//
// Simon Rafferty SimonSFX@Outlook.com 2022
//
// Steve Lovejoy Edits Oct 2023.
//
// The wiring harness is similar to the original, except:
// 6 Pin Connector - Pin:  [Changes indicated by *].
// 1 - Clock (+12V = Heater On)/(0V = Heater Off).
// 2*- TX. yellow
// 3*- Water Thermistor (100k NTC between pin 3 & Gnd) [See Note] (wrong markings on pcb).
// 4*- RX. orange
// 5*- Exahust Thermistor (100k NTC between pin 5 & Gnd) (wrong markings on pcb). room stat on aux screen.2 
// 6 - Fuel Dosing Pump.
//
// [Note] The PCB has holes for the original Water Thermistor which you can salvage from an old unit
// It has a different 25C Resistance around 4.7k.  You will need to change R11 to 4.7k
// and in get_webasto_temp, change "Nominal resistance at 25 ºC" from 100000 to 4700.
//*********************************************************************************************

//Build options
#define FLAME_SENSOR_ENABLE  //Uncomment if using V3.0 board with ACS711 Current Sensor
#define AUX_SCREEN_ENABLE    // Uncomment if using steves aux screen board..

#include <math.h>  // needed to perform some calculations.

//Heater Config
//*********************************************************************************
//**Change these values to suit your application **
int heater_min = 55;      // Increase fuel if below
int heater_target = 65;   // degrees C Decrease fuel if above, increase if below.
int water_warning = 75;   // degrees C - At this temperature, the heater idles
int water_overheat = 85;  // degrees C - This is the temperature the heater will shut down

int flame_threshold = 75;  //Exhaust temperature above which we assume it's alight

//Fuel Mixture
//If you find the exhaust is smokey, increase the fan or reduce the fuel
float throttling_high_fuel = 1.8;
//float throttling_high_fuel = 1.6; //In summer, exhaust gets too hot on startup
float throttling_high_fan = 90;
float throttling_steady_fuel = 1.3;
float throttling_steady_fan = 70;
float throttling_low_fuel = 0.83;
float throttling_low_fan = 60;
//Just enough to keep it alight at idle
float throttling_idle_fuel = 0.6;  //Do not reduce this value
float throttling_idle_fan = 35;

/*
  //David's settings
  float throttling_high_fuel = 1.8;
  float throttling_high_fan = 90;
  float throttling_steady_fuel = 1.5;
  float throttling_steady_fan = 65;
  float throttling_low_fuel = 0.83;
  float throttling_low_fan = 50;
  float throttling_idle_fuel = 0.5; //Just enough to keep it alight
  float throttling_idle_fan = 30;
*/
//*********************************
// ToDo:  Winter & summer need slightly different idle settings
// I'm guessing because the air intake temperature is higher in summer, it doesn't
// need as much fuel to heat to a given temperature.
// Using the winter setting in Summer causes it to overheat & shut down before the
// water tank has heated properly.
// * Need to find a way of switching automatically
//********************************

// LED
int led = LED_BUILTIN;

//Fuel Pump Setting
//Different after-market pumps seem to deliver different amounts of fuel
//If the exhaust is consistently smokey, reduce this number
//If you get no fuel (pump not clicking) increase this number
//Values 22,30 or 60 seem to work in most cases.

int pump_size = 22;  //22,30,60
//**********************************************************************************

//Prime
float prime_low_temp = -10;  //Exhaust temp inaccurate at low temp. -10 is approx 10C
//float prime_low_temp = 4; //Wasn't always starting cold, increase fueling a bit
float prime_high_temp = 20;
bool Fuel_Purge = false;  //Set by Aux switch.  Delivers fuel rapidly without running anything else

float prime_fan_speed = 15;
float prime_low_temp_fuelrate = 3.5;
float prime_high_temp_fuelrate = 2.0;

//Inital
float start_fan_speed = 40;
float start_fuel = 1;              //Summer setting.
float start_fuel_Threshold = -10;  //Exhaust temperature, below which to use start_fuel_Cold.
float start_fuel_Cold = 1.2;       //Winter Setting (use below 10C).
float start_fuel_Warm = 1.0;       //Summer Setting (use above 10C).

int full_power_increment_time = 30;  //seconds

  //Simons Pin Connections
  int fuel_pump_pin = 11;
  int glow_plug_pin = 5;
  int burn_fan_pin = 10;
  int water_pump_pin = 9;
  int water_temp_pin = A1;
  int exhaust_temp_pin = A2;
  //int lambda_pin = A3;
  int push_pin = A0;
  int flame_sensor = A4;
  //Simons Pin Connections
/*
//Pin Connections for Adafruit Feather M0 Express.
int fuel_pump_pin = 11;
int glow_plug_pin = 5;
int burn_fan_pin = 10;
int water_pump_pin = 9;
int lambda_pin = A1;
int exhaust_temp_pin = A3;
int water_temp_pin = A2;
int flame_sensor = A4;
int push_pin = A5;
//Pin Connections
*/


//Temperature Filtering
#define filterSamples 13                  // filterSamples should  be an odd number, no smaller than 3.
float rawDataWater, smoothDataWater;      // variables for sensor1 data.
float rawDataExhaust, smoothDataExhaust;  // variables for sensor2 data.

float WaterSmoothArray[filterSamples];    // array for holding raw sensor values for sensor1.
float ExhaustSmoothArray[filterSamples];  // array for holding raw sensor values for sensor2.
float RoomSmoothArray[filterSamples];     // array for holding raw sensor values for sensor3.

float Last_Exh_T = 0;
float Last_Wat_T = 0;
float Last_Mute_T = 0;
int GWTLast_Sec;
int Last_TSec;
bool EX_Mute = false;
float Last_Temp = 0;
float Max_Change_Per_Sec_Exh = 4;  //Used to slow down changes in exhaust temperature to remove spikes.
float Max_Change_Per_Sec_Wat = 2;  //Used to slow down changes in water temperature to remove spikes.
//Flame Sensor workings
float Flame_Diff = 0;
float Flame_Threshold = 1.000;
long Flame_Timer = millis();  //prevent flame sensor being called too often.
float Flame_Last = 0;

//Serial Settings
String message = "Off";
//bool pushed;
//bool long_press;
bool heater_on;
bool debug_glow_plug_on = false;
int debug_water_percent_map = 999;

//Varaiables
int Ignition_Failures = 0;
float fan_speed;             // percent
float water_pump_speed;      // percent
float fuel_need;             // percent
int glow_time;               // seconds
float water_temp;            // degrees C
float exhaust_temp;          // degrees C
float exhaust_temp_sec[11];  // array of last 10 sec exhaust temp, degres C.
int water_temp_sec[11];      // array of last 10 sec water temp, degres C.
int glow_left = 0;
int last_glow_value = 0;
bool burn = false;
bool webasto_fail = false;
int Start_Failures = 0;
int seconds;
int room_target_temp = 30;  // target room temp degrees C.
int temp_room_target_temp = 28;  // temp target room temp degrees C for input from aux screen.
float room_temp = 0;            // degrees C.
long restart_timer = 0;     //Holder for restart time.
long restart_delay = 60;    //(minutes) After an overheat, delay the restart.
bool lean_burn;
int delayed_period = 0;
unsigned long water_pump_started_on;
int water_pump_started = 0;
long glowing_on = 0;
int burn_mode = 0;
const int glow_channel = 0;
const int water_channel = 1;
const int air_channel = 2;

//Aux Write Variables
int AuxHeaterOn = 0;
int AuxPurgeFuel = 0;

// Variables to hold the parsed data.
const byte numChars = 16;
bool newData = false;
char receivedChars[numChars];
char tempChars[numChars];  // temporary array for use when parsing.
char messageIn[numChars] = { 0 };

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  delay(2000);

  pinMode(led, OUTPUT);
  pinMode(glow_plug_pin, OUTPUT);
  pinMode(fuel_pump_pin, OUTPUT);
  pinMode(burn_fan_pin, OUTPUT);
  pinMode(water_pump_pin, OUTPUT);
  pinMode(water_temp_pin, INPUT);
  pinMode(exhaust_temp_pin, INPUT);
  pinMode(push_pin, INPUT);
  pinMode(lambda_pin, INPUT);
  pinMode(flame_sensor, INPUT);

  analogWrite(water_pump_pin, 100);  //Run water pump on startup for a few seconds
  //Pulse Burn fan - to test & indicate startup
  fan_speed = 70;
  burn_fan();
  delay(1000);
  fan_speed = 0;
  burn_fan();
  delay(1000);
  fan_speed = 70;
  burn_fan();
  delay(1000);
  fan_speed = 0;
  burn_fan();
  delay(3000);

  analogReadResolution(12);

}  // end startup

void loop() {  // runs over and over again, calling the functions one by one

  //============================
  // DATA RETREVAL
  //============================
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    // because strtok() used in parseData() replaces the commas with \0.
    parseData();
    showParsedData();
    newData = false;
  }

  temp_data();
  control();
  webasto();

}  // end loop

void Fuel_Purge_Action() {
  //If it's safe to do so (heater & glow plug switched off), run the fuel pump rapidly to purge air
  if (!heater_on && !debug_glow_plug_on) {
    if (Fuel_Purge) {
      fuel_need = prime_ratio(prime_low_temp);  //Set fuel rate to max
    } else {
      fuel_need = 0;
    }
    fuel_pump();
  }
}  // end fuel purge



/*
  #ifdef AUX_SCREEN_ENABLE
  //Receive data from aux screen

  BLYNK_WRITE(V50)
  {
    //When the heater is switched on via the aux screen - AUXHeaterOn changes from 0 to 1
    AUXHeaterOn = param.asInt(); // assigning incoming value from pin V1 to a variable
  }

  BLYNK_WRITE(V51)
  {
    //When selected from the aux screen, deliver fuel rapidly
    Fuel_Purge = param.asInt(); // assigning incoming value from pin V1 to a variable
  }

  BLYNK_WRITE(V52)
  {
    //When selected from the aux screen, deliver fuel rapidly
    room_temp_set = param.asInt(); // assigning incoming value from pin V1 to a variable
  }
  #endif
*/



/*
  #ifdef BLYNK_ENABLE
  //Receive data from Blynk App

  BLYNK_WRITE(V50)
  {
    //When the heater is switched on via the aux screen - BlynkHeaterOn changes from 0 to 1
    BlynkHeaterOn = param.asInt(); // assigning incoming value from pin V1 to a variable
  }

  BLYNK_WRITE(V51)
  {
    //When selected from the aux screen, deliver fuel rapidly
    Fuel_Purge = param.asInt(); // assigning incoming value from pin V1 to a variable
  }

  BLYNK_WRITE(V52)
  {
    //When selected from the aux screen, deliver fuel rapidly
    room_temp_set = param.asInt(); // assigning incoming value from pin V1 to a variable
  }
  #endif
*/
//==============================================
// GET DATA.
//==============================================
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial1.available() > 0 && newData == false) {
    rc = Serial1.read();
    //Serial.println(rc);
    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }  // end while serial available.
}  // end recvWithStartEndMarkers.

//=======================
// PARSING DATA atoi for ints & atof for floats.
//=======================
void parseData() {  // split the data into its parts
  // Index.
  char* strtokIndx;  // this is used by strtok() as an index
  // strings.
  strtokIndx = strtok(tempChars, ",");  // get the string
  strcpy(messageIn, strtokIndx);        // copy it to messageIn
  AuxHeaterOn = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
  temp_room_target_temp = atoi(strtokIndx);  // convert this part to an integer
  // floats.
  
  

}  // end parsing.

//========================
// Show parsed data.
//==========================
void showParsedData() {

  Serial.print(" | AuxHeaterOn : ");
  Serial.print(AuxHeaterOn);
  Serial.print(" | TRTT: ");
  Serial.println(temp_room_target_temp);
  //Serial.println(message);
}  // end showParsedData
