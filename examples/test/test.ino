#include <Arduino.h>
#include <GSON.h>

void setup() {
    Serial.begin(115200);
    GSON::string gs;                // создать строку
    gs.beginObj();                  // начать объект
    gs.addStr("str1", F("value"));  // добавить строковое значение
    gs["str2"] = "value2";          // так тоже можно
    gs["int"] = (int32_t)12345;     // целочисленное
    gs.beginObj("obj");             // вложенный объект
    gs.addFloat(F("float"), 3.14);  // float
    gs["float2"] = 3.14;            // или так
    gs["bool"] = false;             // Bool значение
    gs.endObj();                    // завершить объект

    gs.beginArr("array");
    gs.addFloat(3.14);
    gs = "text";
    gs = 12345;
    gs = true;
    gs.endArr();
    
    gs.endObj();  // завершить объект
    gs.end();     // завершить пакет

    Serial.println(gs);

    GSON::Doc doc(10);
    doc.parse(gs);

    Serial.println(doc[0]);
    Serial.println(doc["str2"]);
    Serial.println(doc[F("int")]);
    Serial.println(doc["obj"]["float"]);
    Serial.println(doc["array"][0]);
    Serial.println(doc["array"][1]);
    Serial.println(doc["array"][2]);
    Serial.println();
    Serial.println();

    for (uint16_t i = 0; i < doc.entries.length(); i++) {
        // if (doc.entries[i].type == GSON::Type::Object || doc.entries[i].type == GSON::Type::Array) continue; // пропустить контейнеры
        Serial.print(i);
        Serial.print(". [");
        Serial.print(doc.readType(i));
        Serial.print("] ");
        Serial.print(doc.entries[i].key);
        Serial.print(":");
        Serial.print(doc.entries[i].value);
        Serial.print(" {");
        Serial.print(doc.entries[i].parent);
        Serial.println("}");
    }
}

void loop() {
}