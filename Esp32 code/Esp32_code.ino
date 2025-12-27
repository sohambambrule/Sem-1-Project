#define BLYNK_TEMPLATE_ID "TMPL3TsHNYIo0"
#define BLYNK_TEMPLATE_NAME "esp32"
#define BLYNK_AUTH_TOKEN "xgE7-JQO7kbZHOw0qD86pJftzshFfpSa"
#define BLYNK_PRINT Serial

#include "EmonLib.h"   
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

EnergyMonitor emon;

// Calibration settings 
#define vCalibration 128.0   // Calibrated for ~244V reading
#define currCalibration 4.8  // Calibrated for 220-ohm burden resistor
#define ADC_BITS 12
#define ADC_COUNTS (1 << ADC_BITS)

BlynkTimer timer;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "******";      
char pass[] = "******";

float kWh = 0;
unsigned long lastmillis = millis();
bool alertSent = false; 

void myTimerEvent() {
    // Calculate Voltage and Current 
    emon.calcVI(40, 2000); 
    
    // Dividing by 5 as we looped the wire 5 times through the sensor to catch the readings
    float actualIrms = emon.Irms / 05.0;
    float actualPower = emon.apparentPower / 05.0;

    // Power Calculation
    unsigned long currentMillis = millis();
    kWh += actualPower * (currentMillis - lastmillis) / 3600000.0; 
    lastmillis = currentMillis;

    // Sends alert for power consumption when reached a certain limit
    if (kWh >= 05.0 && !alertSent) {
        Blynk.logEvent("energy_alert", "Alert! Your energy usage has reached 5 kWh.");
        alertSent = true; 
        Serial.println(" ALERT! NOTIFICATION SENT TO BLYNK ");
    }
    
    // Reset alert flag if kWh is ever reset to zero
    if (kWh < 05.0) alertSent = false; 

    //  Printing the output on serial monitor
    Serial.print("Vrms: "); Serial.print(emon.Vrms, 2); Serial.print("V");
    Serial.print("\tIrms: "); Serial.print(actualIrms, 4); Serial.print("A");
    Serial.print("\tPower: "); Serial.print(actualPower, 2); Serial.print("W");
    Serial.print("\tkWh: "); Serial.println(kWh, 4);

    // Update Blynk cloud dashboard
    Blynk.virtualWrite(V0, emon.Vrms);
    Blynk.virtualWrite(V1, actualIrms);  
    Blynk.virtualWrite(V2, actualPower);
    Blynk.virtualWrite(V3, kWh);
}

void setup() {
  Serial.begin(115200); 
  
  // GPIO 35 for Voltage and GPIO 34 for Current
  emon.voltage(35, vCalibration, 1.7); 
  emon.current(34, currCalibration); 

  Serial.println("IEMS System Initializing...");
  Blynk.begin(auth, ssid, pass);
  
  // Set timer to give updates every 5 seconds
  timer.setInterval(5000L, myTimerEvent);
}

void loop() {
  Blynk.run();
  timer.run();
}
