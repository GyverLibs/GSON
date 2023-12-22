[![latest](https://img.shields.io/github/v/release/GyverLibs/GSON.svg?color=brightgreen)](https://github.com/GyverLibs/GSON/releases/latest/download/GSON.zip)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD$%E2%82%AC%20%D0%9D%D0%B0%20%D0%BF%D0%B8%D0%B2%D0%BE-%D1%81%20%D1%80%D1%8B%D0%B1%D0%BA%D0%BE%D0%B9-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GSON?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GSON
Парсер и сборщик данных в формате JSON для Arduino
- В два раза быстрее и сильно легче ArduinoJSON
- Парсинг JSON с обработкой ошибок
- Линейная сборка JSON-пакета
- Экранирование "опасных" символов
- Работает на базе AnyText (StringUtils)

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

### Зависимости
- Библиотека [StringUtils](https://github.com/GyverLibs/StringUtils)

## Содержание
- [Документация](#docs)
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="docs"></a>

## Документация
### `GSON::Doc`
```cpp
// конструктор
GSON::Doc();
GSON::Doc(размер);
GSON::DocStatic<размер>();

// методы
uint16_t length();              // получить количество записей Entry
uint16_t size();                // получить размер Doc в оперативной памяти (байт)
Entry get(const AnyText& key);  // доступ по ключу (главный контейнер - Object)
Entry get(int index);           // доступ по индексу (главный контейнер - Array или Object)
AnyText key(uint16_t idx);      // прочитать ключ по индексу
AnyText value(uint16_t idx);    // прочитать значение по индексу
GSON::Type type();              // получить тип по индексу
const __FlashStringHelper* readType(uint16_t idx);  // прочитать тип по индексу

// парсить. Вернёт true при успешном парсинге
bool parse(String& json);
bool parse(const char* json);

template <uint16_t max_depth = 16>
bool parseT(String& json);
template <uint16_t max_depth = 16>
bool parseT(const char* json);

// вывести в Print с форматированием
void stringify(Print* p);

// обработка ошибок
bool hasError();                        // есть ошибка парсинга
Error getError();                       // получить ошибку
uint16_t errorIndex();                  // индекс места ошибки в строке
const __FlashStringHelper* readError(); // прочитать ошибку
```

### `GSON::Type`
```cpp
None
Object
Array
String
Int
Float
Bool
```

### `GSON::Error`
```cpp
None
Alloc
TooDeep
NoParent
NotContainer
UnexComma
UnexColon
UnexToken
UnexQuotes
UnexOpen
UnexClose
UnknownToken
BrokenToken
BrokenString
BrokenContainer
```

### `GSON::Entry`
Также наследует всё из `AnyText`, документация [здесь](https://github.com/GyverLibs/StringUtils?tab=readme-ov-file#anytext)

```cpp
Entry get(const AnyText& key);  // получить элемент по ключу
Entry get(int index);           // получить элемент по индексу
bool valid();                   // проверка корректности (существования)
AnyText key();                  // получить ключ
AnyText value();                // получить значение
uint16_t length();              // получить размер (для объектов и массивов. Для остальных 0)
Type type();                    // получить тип
```

### `GSON::string`
```cpp
String s;                   // доступ к строке
void clear();               // очистить строку
bool reserve(uint16_t res); // зарезервировать строку

// прибавить GSON::string. Будет добавлена запятая
string& add(const string& str);

// добавить ключ (строка любого типа)
string& addKey(const AnyText& key);

// прибавить текст (строка любого типа)
string& addRaw(const AnyText& str, bool esc = false);

// добавить строку (строка любого типа)
string& addStr(const AnyText& key, const AnyText& value);
string& addStr(const AnyText& value);

// добавить bool
string& addBool(const AnyText& key, const bool& value);
string& addBool(const bool& value);

// добавить float
string& addFloat(const AnyText& key, const double& value, uint8_t dec = 2);
string& addFloat(const double& value, uint8_t dec = 2);

// добавить int
string& addInt(const AnyText& key, const AnyValue& value);
string& addInt(const AnyValue& value);

string& beginObj(const AnyText& key = "");   // начать объект
string& endObj();   // завершить объект

string& beginArr(const AnyText& key = "");   // начать массив
string& endArr();   // завершить массив

string& end();      // завершить пакет
```

<a id="usage"></a>

## Использование
### Парсинг
Особенности:
- Библиотека **не дублирует JSON строку** в памяти, а работает с исходной строкой, сохраняя указатели на значения
- Парсинг **изменяет исходную строку**: некоторые символы заменяются на `'\0'`

Отсюда следует, что:
- Строка должна существовать в памяти на всём протяжении работы с json документом
- Если исходная строка - `String` - она категорически не должна изменяться программой до окончания работы с документом

Создание документа:
```cpp
GSON::Doc doc;              // динамический документ
GSON::Doc doc(10);          // динамический документ, зарезервирован под 10 элементов
GSON::DocStatic<10> doc;    // статический документ, зарезервирован под 10 элементов
```

Смысл как у `String`-строки: динамический документ будет увеличиваться в процессе парсинга в динамической памяти МК, если размер документа неизвестен. Можно заранее зарезервировать размер, чтобы парсинг происходил быстрее. Статический - выделяется статически, используя меньше памяти на слабых платформах.

```cpp
// получили json
char json[] = R"raw({"key":"value","int":12345,"obj":{"float":3.14,"bool":false},"arr":["hello",true]})raw";
String json = "{\"key\":\"value\",\"int\":12345,\"obj\":{\"float\":3.14,\"bool\":false},\"arr\":[\"hello\",true]};";

// парсить
doc.parse(json);
```

Парсер имеет **фиксированный** уровень вложенности объектов и массивов, при его переполнении будет ошибка парсинга `TooDeep`. По умолчанию значение равно 16 (*один уровень вложенности занимает один байт памяти*). Можно изменить его, используя шаблонную функцию `parseT<размер>`. 

```cpp
// вывести весь пакет с типами, ключами, значениями в виде текста и родителем
for (int i = 0; i < doc.entries.length(); i++) {
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
```

> Вес одного "элемента" любого типа (строка, число, объект, массив) составляет 6 байт на AVR и 10 байт на ESP. Таким образом после парсинга примера выше в памяти дополнительно появятся 9 элементов (54 байта).

Значения можно получать в типе `AnyText`, который может конвертироваться в другие типы и выводиться в порт:
- Ключом может быть строка в любом виде (`"строка"`, `F("строка")`)
- Можно обращаться ко вложенным объектам по ключу, а к массивам по индексу

```cpp
Serial.println(doc["key"]);      // value
Serial.println(doc[F("int")]);   // 12345
int val = doc["int"].toInt16();  // конвертация в указанный тип
val = doc["int"];                // авто конвертация
float f = doc["obj"]["float"];   // вложенный объект
Serial.println(doc["arr"][0]);   // hello
Serial.println(doc["arr"][1]);   // true

// Пусть json имеет вид [[123,456],["abc","def"]], тогда ко вложенным массивам можно обратиться:
Serial.println(doc[0][0]);  // 123
Serial.println(doc[0][1]);  // 456
Serial.println(doc[1][0]);  // abc
Serial.println(doc[1][1]);  // def
```

Каждый элемент можно вывести в тип `GSON::Entry` по имени (из объекта) или индексу (из массива) и использовать отдельно, чтобы не "искать" его заново:
```cpp
GSON::Entry e = doc["arr"];
Serial.println(e.length());  // длина массива
Serial.println(e[0]);        // hello
Serial.println(e[1]);        // true
```

### Сборка
JSON строка собирается **линейно** в обычную `String`-строку, что очень просто и приятно для микроконтроллера:
```cpp
GSON::string gs;                // создать строку
gs.beginObj();                  // начать объект 1
gs.addStr("str1", F("value"));  // добавить строковое значение
gs["str2"] = "value2";          // так тоже можно
gs["int"] = 12345;              // целочисленное
gs.beginObj("obj");             // вложенный объект 2
gs.addFloat(F("float"), 3.14);  // float
gs["float2"] = 3.14;            // или так
gs["bool"] = false;             // Bool значение
gs.endObj();                    // завершить объект 2
gs.endObj();                    // завершить объект 1

gs.end();           // ЗАВЕРШИТЬ ПАКЕТ (обязательно вызывается в конце)

Serial.println(gs); // вывод в порт
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>

## Установка
- Библиотеку можно найти по названию **GSON** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/GSON/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)

### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>
## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код