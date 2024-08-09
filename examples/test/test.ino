#include <Arduino.h>
#include <GSON.h>

void setup() {
    Serial.begin(115200);
    Serial.println();

    gson::string gs;                   // создать строку
    gs.beginObj();                     // начать объект
    gs.addString("str1", F("value"));  // добавить строковое значение
    gs["str2"] = "value2";             // так тоже можно
    gs["int"] = (int32_t)12345;        // целочисленное
    gs.beginObj("obj");                // вложенный объект
    gs.addFloat(F("float"), 3.14);     // float
    gs["float2"] = 3.14;               // или так
    gs["bool"] = false;                // Bool значение
    gs.endObj();                       // завершить объект

    gs.beginObj("obj2");
    gs.beginArr("array");
    gs.addFloat(3.14);
    gs += "text";  // в массиве можно так
    gs += 12345;
    gs += true;
    gs.endArr();
    gs.endObj();
    gs["last"] = "kekpek";

    gs.endObj();  // завершить объект
    gs.end();     // завершить пакет

    Serial.println("==== PACKET ====");
    Serial.println(gs);
    Serial.println();

    gson::Parser p(10);
    p.parse(gs);

    Serial.println("==== READ ====");
    Serial.println(p[0]);
    Serial.println(p["str2"]);
    Serial.println(p[F("int")]);
    Serial.println(p["obj"]["float"]);
    Serial.println(p["obj2"]["array"][0]);
    Serial.println(p["obj2"]["array"][1]);
    Serial.println(p["obj2"]["array"][2]);

    Serial.println(p["no-key"]);
    Serial.println(p["no-key"]["no-key"]);
    Serial.println();

    p.hashKeys();
    Serial.println("==== HASH ACCESS ====");
    Serial.println(p[su::SH("str2")]);
    Serial.println(p[su::SH("obj")][su::SH("float")]);
    Serial.println();

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

    Serial.println("==== STRINGIFY ====");
    p.stringify(Serial);
    Serial.println();

    Serial.println("==== STRINGIFY ====");
    p[su::SH("obj2")].stringify(Serial);
    Serial.println();

    Serial.println("==== STRINGIFY ====");
    p[su::SH("obj")][su::SH("float2")].stringify(Serial);
    Serial.println();

    Serial.println("==== STRINGIFY ====");
    p[su::SH("int")].stringify(Serial);
    Serial.println();
}

void loop() {
}