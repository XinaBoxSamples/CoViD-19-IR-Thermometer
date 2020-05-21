float swversion = 0.2; // Software Version
#include "xCore.h" // XinaBox Arduino Core: https://github.com/xinabox/arduino-core
#include "xOD01.h" // XinaBox OD01 Library: https://github.com/xinabox/arduino-od01
xOD01 OD01; //Start OD01
#include <SparkFunMLX90614.h> // SparkFun's Library: https://github.com/sparkfun/SparkFun_MLX90614_Arduino_Library
IRTherm therm; //Start SL19
float newEmissivity = 0.985; // The human skin emissivity: http://www-eng.lbl.gov/~dw/projects/DW4229_LHC_detector_analysis/calculations/emissivity2.pdf
float feverT = 35.6; // The fever limit: https://pubmed.ncbi.nlm.nih.gov/15877017/
#define R A4   //Red RGB LED
#define G 8  //Green RGB LED
#define B 9   //Blue RGB LED
#define limitT 30 // Don't bother below 30 deg C
#define Serial SerialUSB // Serial is actually on SerialUSB
#define DEBUG false
void(* resetFunc) (void) = 0; //declare reset function @ address 0
void setup() {
  Wire.begin();
  therm.begin();
  therm.setUnit(TEMP_C);
  while (!therm.read()) delay(10);
  Serial.begin(115200);
  //while (!Serial); // When testing, don't start before the console is ready.
  delay(1000);
  if (therm.object() < 0 | therm.object() > 50) {
    Serial.println(therm.object());
    Serial.flush();
    resetFunc();
  }
  delay(20);

  Serial.println();
  Serial.println("CoViD-19 Thermometer");
  Serial.print("Version: ");
  Serial.println(swversion);
  OD01.begin();
  OD01.set2X();
  OD01.println("READY");
  OD01.set1X();
  OD01.println("CoViD-19 Thermometer");
  OD01.println();
  OD01.print("Version: ");
  OD01.println(swversion);
  OD01.println();

  Serial.println("Old Emissivity: " + String(therm.readEmissivity(), 3));
  therm.setEmissivity(newEmissivity);
  OD01.print("Emissivity: ");
  OD01.println(therm.readEmissivity(), 3); // Just to be sure
  Serial.println("New Emissivity: " + String(therm.readEmissivity(), 3));
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  delay(100);
  digitalWrite(R, LOW); digitalWrite(G, LOW); digitalWrite(B, HIGH);
}

void loop() {
  float oneT, maxT;
  int i = 1, j = 0;
  oneT = 0; maxT = limitT;
  if (DEBUG) Serial.print("L");
  do {
    if (DEBUG) Serial.print("D");
    if (therm.read()) {
      if (DEBUG) Serial.print("R");
      delay(10); //Don't read too quickly - crashes the MCU
      oneT = therm.object();
      if (DEBUG) Serial.print(oneT);
      if (oneT > maxT) { //Report the highest temperature
        maxT = oneT;
      }
      if (maxT != limitT) {
        if (int(j++ / 10)) {
          i = 1 - i;
          j = 0;
        }
        digitalWrite(R, LOW); digitalWrite(G, LOW); digitalWrite(B, i);
      }
    }
  } while (oneT > limitT);
  if (DEBUG) Serial.print("E");
  if (maxT != limitT) { // Did we get a human temperature (over 30)?
    if (DEBUG) Serial.print("P");
    Serial.println(maxT);
    OD01.clear();
    OD01.set2X();
    OD01.print("Temp: ");
    OD01.print(maxT, 1);
    OD01.println(" C");
    OD01.set1X();
    //OD01.println("Press reset for next");
    OD01.println("");
    OD01.print("Temperature > ");
    OD01.print(feverT, 1);
    OD01.println(" C");
    OD01.println("is considered a fever");
    OD01.println();
    OD01.set2X();
    if (maxT < feverT) {
      OD01.print("  HEALTHY");
      digitalWrite(R, LOW); digitalWrite(G, HIGH); digitalWrite(B, LOW);
    }
    else {
      OD01.print("   FEVER");
      digitalWrite(R, HIGH); digitalWrite(G, LOW); digitalWrite(B, LOW);
    }
  }
}
