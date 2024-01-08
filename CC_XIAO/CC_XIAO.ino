// https://blog.berrybase.de/seeeduino-xiao-erste-schritte/
// https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

#include <movingAvg.h>



movingAvg mySensor(32);   // https://github.com/JChristensen/movingAvg

#define PIN_MESSAGELED 2
#define PIN_ADC_CURRENT  A1


long intCurrentSum = 0;
long intCurrentAvg = 0;
int intSumCycle = 0;
uint64_t intChargeTotal = 0;
long intChargeTotalReduced = 0;
long intVoltage = 13;
long intCurrentFactor = 0;
long intSumLimit = 64;
long intLastTime = 0;


void setup() {
  
  // pinMode(PIN_POWERLED, OUTPUT);
  pinMode(PIN_MESSAGELED, OUTPUT);
  


  digitalWrite(PIN_MESSAGELED, HIGH);

  mySensor.begin();

  Serial.begin(9600);
  Serial1.begin(9600);
  
  Serial.println("Hello I am your AVS Node!");

  intCurrentFactor = 3300000 / (1024 * intSumLimit);
}

void loop() {
  
  int intCurrent = analogRead(PIN_ADC_CURRENT);

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
    
    
    if (Serial1.available() > 0) {
      
      int incomingByte = Serial1.read();
      
      if (incomingByte == '#') 
      {
        int secondByte = Serial1.read();
        
        if (secondByte == 'c') {
          intChargeTotalReduced =  intChargeTotal / 1000000;   //  in Ws
          Serial1.println(String(intChargeTotalReduced));
        }
        
        if (secondByte == 'i') {   
          Serial1.println(String(intCurrentAvg / 1000));       //  in mA
        }
        
        if (secondByte == 'e') {
          intChargeTotalReduced =  intChargeTotal / 3600000;   //  in mWh
          Serial1.println(String(intChargeTotalReduced));
        }

        if (secondByte == 'p') {
          Serial1.println(String(intVoltage * intCurrentAvg / 1000));  // in mW
        }
        
        if (secondByte == 'a') {
          Serial1.println(String(intPeriod));
        }
      }

    }

  }
  
  delay(1); 
}
