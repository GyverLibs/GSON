#include <Arduino.h>
#include <GSON.h>

char json[] = R"raw({"key":"value","int":12345,"nullType":null,"obj":{"float":3.14,"bool":false},"arr":["hello",true]})raw";

void setup() {
    Serial.begin(115200);
    while( !Serial ){
        delay(100);
    }
    Serial.println();

    gson::Parser p;
    p.parse(json);

    Serial.println("==== CHUNKS ====");
    for (int i = 0; i < p.length(); i++) {
        Serial.print(i);
        Serial.print(". [");
        Serial.print(p.readType(i));
        Serial.print("] ");
        Serial.print(p.key(i));
        Serial.print(":");
        Serial.print(p.value(i));
        Serial.print(" {");
        Serial.print(p.parent(i));
        Serial.println("}");
    }
    Serial.println();

}

void loop() {
}