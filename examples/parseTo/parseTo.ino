#include <Arduino.h>
#include <GSON.h>

void setup() {
    gson::Parser p;
    p.parse("[1,2,3]");
    p.stringify(Serial);
    
    int arr[3];
    p.parseTo(arr);
    Serial.println(arr[0]);
    Serial.println(arr[1]);
    Serial.println(arr[2]);
}

void loop() {
}