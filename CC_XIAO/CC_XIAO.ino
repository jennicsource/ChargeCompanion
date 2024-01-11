// https://blog.berrybase.de/seeeduino-xiao-erste-schritte/
// https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

#include <movingAvg.h>

movingAvg mySensor(32);   // https://github.com/JChristensen/movingAvg

#include <SparkFun_ADS1015_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ADS1015
#include <Wire.h>

ADS1015 adcSensor;


#define PIN_USERLED        2
#define PIN_ADC_CURRENT    A1
#define PIN_SWITCH_SERIAL  10


long intCurrentFactor = 0;
long intSumLimit = 64;


void setup() {
  
  pinMode(PIN_USERLED, OUTPUT);
  

  pinMode(PIN_SWITCH_SERIAL, INPUT_PULLUP);
 

  mySensor.begin();

  Wire.begin();

  Serial.begin(9600);

  //Serial1.begin(9600);


  
  
  if (adcSensor.begin() == true)
  {
    Serial.println("Device found. I2C connections are good.");
  }
  else
  {
    Serial.println("Device not found. Check wiring.");
    while (1); // stall out forever
  }
  
  digitalWrite(PIN_USERLED, HIGH);

  intCurrentFactor = 3300000 / (1024 * intSumLimit);
}


long    intCurrentSum = 0;
long    intCurrentAvg = 0;
int     intSumCycle = 0;
uint64_t intChargeTotal = 0;
long intChargeTotalReduced;

long    intVoltage = 13;
long    intLastTime = 0;

String strRequest = "";
String strResponse = "";
int FirstByte;
int SecondByte;


void loop() {
  
  //int intCurrent = analogRead(PIN_ADC_CURRENT);

  uint16_t intCurrent = adcSensor.getSingleEnded(3);

  Serial.println(intCurrent);


  intCurrentSum = intCurrentSum + intCurrent;
 
  intSumCycle++;

  if (intSumCycle > intSumLimit)
  {
    intSumCycle = 0;
    
    intCurrentAvg = mySensor.reading(intCurrentSum) * intCurrentFactor;   //  max 3300000 µA

    
    unsigned long intTime = millis();

    unsigned long intPeriod = intTime - intLastTime;   // about 66 [ms]
    intLastTime = intTime;

    long intChargeStep = (intCurrentAvg * intVoltage * intPeriod) / 1000;   //  in µWs  , typical 40.000 to 500.000

    intChargeTotal = intChargeTotal + intChargeStep;   //  in µWs 

    intCurrentSum = 0;

 /*

   #v    send Voltage
   #i    send Current    
   #p    send Power
   #e    send Energy
   #c    send Charge
   #t    send Time
   #a    send Sampling Time
    
 */   
    
    strRequest = "";
    strResponse = "";

    FirstByte = 0;
    SecondByte = 0;
    
    if ( digitalRead(PIN_SWITCH_SERIAL) == HIGH)
    {
      if (Serial.available() > 0) { 
        FirstByte = Serial.read();    
        if (FirstByte == '#') 
        {
          SecondByte = Serial.read();
        }
      }
    }
    else
    {
      if (Serial1.available() > 0) { 
        
        FirstByte = Serial1.read();    
        if (FirstByte == '#') 
        {
          SecondByte = Serial1.read();
        }
      }
    }


   
    if (FirstByte > 0)   
    {
      switch (SecondByte) {
        case 'c':
          intChargeTotalReduced = intChargeTotal / 1000000;   
          strResponse = String(intChargeTotalReduced);      //  in Ws
          break;
     
        case 'i':
          strResponse = String(intCurrentAvg / 1000);         //  in mA
          break;
      
        case 'e':
          intChargeTotalReduced = intChargeTotal / 3600000;
          strResponse = String(intChargeTotalReduced);         //  in mWh
          break;
      
        case 'p':
          strResponse = String(intVoltage * intCurrentAvg / 1000);         //  in mW
          break;
      
        case 'a':
          strResponse = String(intPeriod);         //  in ms
          break;
      }
    }

    if (strResponse != "")
    {
      if ( digitalRead(PIN_SWITCH_SERIAL) == HIGH)
      {
          Serial.println(strResponse);     
      }
      else
      {
          Serial1.println(strResponse); 
          Serial.println(strResponse); 

      }
    }

  }
  
  delay(1); 
}
