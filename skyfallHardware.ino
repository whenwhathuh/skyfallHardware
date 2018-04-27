// The SFE_LSM9DS1 library requires both Wire and SPI be
// included BEFORE including the 9DS1 library.
#include <Wire.h> //used by imu
#include <SPI.h> //used by imu and SD
#include <SparkFunLSM9DS1.h> //used by imu
#include <SD.h> //used by SD
#include <SoftwareSerial.h> //used by gps

//////////////////////////
// LSM9DS1 Library Init //
//////////////////////////
// Use the LSM9DS1 class to create an object.
LSM9DS1 imu;

///////////////////////
// Example I2C Setup //
///////////////////////
// SDO_XM and SDO_G are both pulled high, so our addresses are:
#define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW (0001 1110)
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW (0110 1011)

////////////////////////////
// Sketch Output Settings //
////////////////////////////
#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 250 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract 
// a declination to get a more accurate heading. Calculate 
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
//#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.

////////////////////////
// SoftwareSerial GPS //
////////////////////////
SoftwareSerial gpsSerial(12,8);
const int sentenceSize = 80;
boolean readGPS = true;
char curChar = ' ';
char prevChar = ' ';

File myFile;
int numWrites = 0;

const int switchPin = 5;
const int ledPin = 4;
int switchState = 0;

boolean fileWritten = false;

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);

  //for imu
  Serial.begin(115200);
  
  // Before initializing the IMU, there are a few settings
  // we may need to adjust. Use the settings struct to set
  // the device's communication mode and addresses:
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;
  // The above lines will only take effect AFTER calling
  // imu.begin(), which verifies communication with the IMU
  // and turns it on.
  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1);
  }//end if
  delay(1000);

  //for SD card
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Serial.println("imu initialized");
  Serial.println("gps initialized");
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");
  delay(1000);
  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  delay(1000);
  Serial.println("waiting to collect data...");
}//end setup()


void loop() {
  switchState = digitalRead(switchPin);

  if(switchState == HIGH){
    //delay(1000);
    digitalWrite(ledPin, HIGH);
    
    Serial.begin(115200);
    // Update the sensor values whenever new data is available
    if (imu.accelAvailable()){
      // To read from the accelerometer, first call the
      // readAccel() function. When it exits, it'll update the
      // ax, ay, and az variables with the most current data.
      imu.readAccel();
    }
  
    if ((lastPrint + PRINT_SPEED) < millis()){//write to SD here
      float ax = imu.calcAccel(imu.ax);
      float ay = imu.calcAccel(imu.ay);
      float az = imu.calcAccel(imu.az);
      Serial.begin(9600);
      myFile.print("accelerometer: ");
      myFile.print(ax);
      myFile.print(", ");
      myFile.print(ay);
      myFile.print(", ");
      myFile.print(az);
      myFile.print("\n");
    
      lastPrint = millis(); // Update lastPrint time
      fileWritten = true;
    }

    if(gpsSerial.available()){
      readGPS = true;
      while(readGPS){
        if(gpsSerial.available()){
          prevChar = curChar;
          curChar = (char)gpsSerial.read();
          if(curChar == '\n' && prevChar != '\r'){
            myFile.print(" NMEA new line without carriage return");
            myFile.print('\n');
          } else if(curChar == '\n' && prevChar == '\r'){
            //myFile.print("end condition met");
            myFile.print('\n');
            readGPS = false;
          } else if(curChar == '$' && prevChar != '\n'){
            myFile.print("  NMEA string ended prematurely");
            myFile.print('\n');
            myFile.print(curChar);
          } else {
            myFile.print(curChar);  
          }
        }
      }
    }

    
  } else {//if(switchState == HIGH){
    digitalWrite(ledPin, LOW);

    if(fileWritten){
      Serial.begin(9600);
      myFile.close();
      myFile = SD.open("test.txt");
      Serial.println("test.txt:");
  
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      // close the file:
      myFile.close();
      Serial.println("entering infinite while");
      while(1);
    }
  }//end if(switchState == HIGH){

}//end loop()











