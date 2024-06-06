[![latest](https://img.shields.io/github/v/release/GyverLibs/GSON.svg?color=brightgreen)](https://github.com/GyverLibs/GSON/releases/latest/download/GSON.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/GSON.svg)](https://registry.platformio.org/libraries/gyverlibs/GSON)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GSON?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GSON
Парсер и сборщик данных в формате JSON для Arduino
- В 6 раз быстрее и сильно легче ArduinoJSON
- Парсинг JSON с обработкой ошибок
- Линейная сборка JSON-пакета
- Экранирование "опасных" символов
- Работает на базе Text ([StringUtils](https://github.com/GyverLibs/StringUtils))
- Работает с 64 битными числами
- Встроенный механизм хэширования ключей
- *Библиотека не подходит для хранения и изменения данных! Только парсинг и сборка пакетов*

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

### Зависимости
- [StringUtils](https://github.com/GyverLibs/StringUtils)
- [GTL](https://github.com/GyverLibs/GTL)

## Содержание
- [Документация](#docs)
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="docs"></a>

## Документация
### `Text`
Под типом `Text` понимается строка в любом формате:
- `"const char"` - строки
- `char[]` - строки
- `F("f-строки")`
- `String` - строки

### `gson::Parser`
```cpp
// конструктор
gson::Parser;

// методы
bool reserve(uint16_t cap);     // зарезервировать память для ускорения парсинга
uint16_t length();              // получить количество элементов
uint16_t size();                // получить размер документа в оперативной памяти (байт)
void hashKeys();                // хешировать ключи всех элементов (операция необратима)
bool hashed();                  // проверка были ли хешированы ключи
void reset();                   // освободить память
void move(Parser& p);

// установить максимальную глубину вложенности парсинга (умолч. 16)
void setMaxDepth(uint8_t maxdepth);

Entry get(Text key);            // доступ по ключу (главный контейнер - Object)
Entry get(size_t hash);         // доступ по хэшу ключа (главный контейнер - Object)
Entry get(int index);           // доступ по индексу (главный контейнер - Array или Object)
Entry getByIndex(parent_t index);   // получить элемент по индексу в общем массиве парсера

Text key(int idx);              // прочитать ключ по индексу
size_t keyHash(int idx);        // прочитать хэш ключа по индексу
Text value(int idx);            // прочитать значение по индексу
int8_t parent(int idx);         // прочитать родителя по индексу
Type type(int idx);             // получить тип по индексу

const __FlashStringHelper* readType(uint16_t idx);  // прочитать тип по индексу

// парсить. Вернёт true при успешном парсинге
bool parse(const Text& json);
bool parse(const char* json, uint16_t len);

// вывести в Print с форматированием
void stringify(Print& p);

// обработка ошибок
bool hasError();                        // есть ошибка парсинга
Error getError();                       // получить ошибку
uint16_t errorIndex();                  // индекс места ошибки в строке
const __FlashStringHelper* readError(); // прочитать ошибку
```

ParserStream - парсер из Stream потока
```cpp
gson::ParserStream;

// прочитать из потока и сохранить себе
bool parse(Stream* stream, size_t length);

// прочитать из строки и сохранить себе
bool parse(const char* str, size_t length);

// освободить память
void reset();

// получить скачанный json пакет как Text
su::Text getJson();

void move(ParserStream& ps);
```

#### Настройки
Объявляются перед подключением библиотеки
```cpp
// увеличить лимиты на хранение (описаны ниже)
#define GSON_NO_LIMITS
```

#### Лимиты и память
- После парсинга один элемент весит 8 байт
- Максимальное количество элементов: 255 на AVR и 512 на ESP
- Максимальная длина json-строки: 65 536 символов
- Максимальная длина ключа: 32 символа
- Максимальная длина значения: 32768 символов
- Максимальный уровень вложенности элементов 16, настраивается. Парсинг рекурсивный, каждый уровень добавляет несколько байт в оперативку

При объявлении `#define GSON_NO_LIMITS` до подключения библиотеки лимиты повышаются:
- Один элемент весит 12 байт
- Максимальное количество элементов: 65 356 на ESP
- Максимальная длина ключа: 256 символов
- Максимальная длина значения: 65 356 символов

### Тесты
Тестировал на ESP8266, пакет сообщений из телеграм бота - 3600 символов, 147 "элементов". Получал значение самого отдалённого и вложенного элемента, в GSON - через хэш. Результат:

| Либа        | Flash  | SRAM  | FreeHeap | Parse    | Get    |
| ----------- | ------ | ----- | -------- | -------- | ------ |
| ArduinoJson | 297433 | 31628 | 41104    | 10686 us | 299 us |
| GSON        | 279349 | 31400 | 43224    | 1744 us  | 296 us |

Таким образом GSON **в 6 раз быстрее** при парсинге, значения элементов получает с такой же скоростью. Сама библиотека легче на 18 кБ во Flash и 2.1 кБ в Heap. В других тестах (AVR) на получение значения GSON с хэшем работал в среднем в 1.5 раза быстрее.

### `gson::Type`
```cpp
None
Object
Array
String
Int
Float
Bool
```

### `gson::Error`
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
EmptyKey
IndexOverflow
LongPacket
LongKey
EmptyString
```

### `gson::Entry`
Также наследует всё из `Text`, документация [здесь](https://github.com/GyverLibs/StringUtils?tab=readme-ov-file#anytext)

```cpp
Entry get(Text key);        // получить элемент по ключу
bool includes(Text key);    // содержит элемент с указанным ключом

Entry get(size_t hash);     // получить элемент по хэшу ключа
bool includes(size_t hash); // содержит элемент с указанным хэшем ключа

Entry get(int index);       // получить элемент по индексу

bool valid();               // проверка корректности (существования)
uint16_t length();          // получить размер (для объектов и массивов. Для остальных 0)
Text key();                 // получить ключ
size_t keyHash();           // получить хэш ключа
Text value();               // получить значение
void stringify(Print& p);   // вывести в Print с форматированием

Type type();                // получить тип элемента
bool is(gson::Type type);   // сравнить тип элемента
bool isContainer();         // элемент Array или Object
bool isObject();            // элемент Object
bool isArray();             // элемент Array
parent_t index();           // индекс элемента в общем массиве парсера
```

### `gson::string`
```cpp
String s;                   // доступ к строке
void clear();               // очистить строку
bool reserve(uint16_t res); // зарезервировать строку

// делать escape символов при прибавлении через оператор = (умолч. вкл, true)
void escapeDefault(bool esc);

// прибавить gson::string. Будет добавлена запятая
string& add(string& str);

// добавить ключ (строка любого типа)
string& addKey(Text key);

// прибавить текст (строка любого типа) без кавычек
string& addText(Text key, Text txt);
string& addText(Text txt);
string& addTextRaw(Text txt); // без запятой

// прибавить текст (строка любого типа) без кавычек с escape символов
string& addTextEsc(Text key, Text txt);
string& addTextEsc(Text txt);
string& addTextRawEsc(Text txt); // без запятой

// добавить строку (строка любого типа)
string& addString(Text key, Text value);
string& addString(Text value);
string& addStringRaw(Text value);   // без запятой

// добавить строку (строка любого типа) с escape символов
string& addStringEsc(Text key, Text value);
string& addStringEsc(Text value);
string& addStringRawEsc(Text value);   // без запятой

// добавить bool
string& addBool(Text key, const bool& value);
string& addBool(const bool& value);
string& addBoolRaw(const bool& value);   // без запятой

// добавить float
string& addFloat(Text key, const double& value, uint8_t dec = 2);
string& addFloat(const double& value, uint8_t dec = 2);
string& addFloatRaw(const double& value, uint8_t dec = 2);   // без запятой

// добавить int
string& addInt(Text key, const Value& value);
string& addInt(const Value& value);
string& addIntRaw(const Value& value);   // без запятой

string& beginObj(Text key = "");   // начать объект
string& endObj();   // завершить объект

string& beginArr(Text key = "");   // начать массив
string& endArr();   // завершить массив

string& end();      // завершить пакет
```

<a id="usage"></a>

## Использование
### Парсинг
Библиотека **не дублирует строку** в памяти и работает с исходной строкой: запоминает позиции текста, исходную строку не меняет. Отсюда следует, что:
- Строка должна существовать в памяти на всём протяжении работы с json документом
- Если исходная строка - `String` - она категорически не должна изменяться программой до окончания работы с документом
- `ParserStream` "скачивает" строку из стрима в память

Создание документа:
```cpp
gson::Parser p;
```

```cpp
// получили json
char json[] = R"raw({"key":"value","int":12345,"obj":{"float":3.14,"bool":false},"arr":["hello",true]})raw";
String json = "{\"key\":\"value\",\"int\":12345,\"obj\":{\"float\":3.14,\"bool\":false},\"arr\":[\"hello\",true]};";

// парсить
p.parse(json);

// обработка ошибок
if (p.hasError()) {
    Serial.print(p.readError());
    Serial.print(" in ");
    Serial.println(p.errorIndex());
} else Serial.println("done");
```

После парсинга можно вывести весь пакет с типами, ключами, значениями в виде текста и родителем:
```cpp
for (uint16_t i = 0; i < p.length(); i++) {
    // if (p.type(i) == gson::Type::Object || p.type(i) == gson::Type::Array) continue; // пропустить контейнеры
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
```

Значения можно получать в типе `Text`, который может конвертироваться в другие типы и выводиться в порт:
- Ключом может быть строка в любом виде (`"строка"`, `F("строка")`)
- Можно обращаться ко вложенным объектам по ключу, а к массивам по индексу

```cpp
Serial.println(p["key"]);      // value
Serial.println(p[F("int")]);   // 12345
int val = p["int"].toInt16();  // конвертация в указанный тип
val = p["int"];                // авто конвертация
float f = p["obj"]["float"];   // вложенный объект
Serial.println(p["arr"][0]);   // hello
Serial.println(p["arr"][1]);   // true

// проверка типа
p["arr"].type() == gson::Type::Array;

// вывод содержимого массива
for (int i = 0; i < p["arr"].length(); i++) {
    Serial.println(p["arr"][i]);
}

// а лучше - так
gson::Entry arr = p["arr"];
for (int i = 0; i < arr.length(); i++) {
    Serial.println(arr[i]);
}

// Пусть json имеет вид [[123,456],["abc","def"]], тогда ко вложенным массивам можно обратиться:
Serial.println(p[0][0]);  // 123
Serial.println(p[0][1]);  // 456
Serial.println(p[1][0]);  // abc
Serial.println(p[1][1]);  // def
```

Каждый элемент можно вывести в тип `gson::Entry` по имени (из объекта) или индексу (из массива) и использовать отдельно, чтобы не "искать" его заново:
```cpp
gson::Entry e = p["arr"];
Serial.println(e.length());  // длина массива
Serial.println(e[0]);        // hello
Serial.println(e[1]);        // true
```

### Хэширование
GSON нативно поддерживает хэш-строки из StringUtils, работа с хэшами значительно увеличивает скорость доступа к элементам JSON документа по ключу. Строка, переданная в функцию `SH`, вообще **не существует в программе** и не занимает места: хэш считается компилятором на этапе компиляции, вместо него подставляется число. А сравнение чисел выполняется быстрее, чем сравнение строк. Для этого нужно:

1. Хэшировать ключи после парсинга:
```cpp
p.parse(json);
p.hashKeys();
```

> Примечание: хэширование не отменяет доступ по строкам, как было в первых версиях библиотеки! Можно использовать как `p["key"]`, так и `p[su::SH("key")]`

2. Обращаться к элементам по хэшам ключей, используя функцию `su::SH`:
```cpp
using su::SH;

void foo() {
    Serial.println(p[SH("int")]);
    Serial.println(p[SH("obj")][SH("float")]);
    Serial.println(p[SH("array")][0]);
}
```

> Примечание: для доступа по хэшу используется перегрузка `[size_t]`, а для доступа к элементу массива - `[int]`. Поэтому для корректного доступа к элементам массива нужно использовать именно `signed int`, а не unsigned (`uint8` и `uint16`)! Иначе компилятор может вызвать доступ по хэшу вместо обращения к массиву.

```cpp
gson::Entry arr = p["arr"];
for (int i = 0; i < arr.length(); i++) {    // счётчик int!
    Serial.println(arr[i]);
}
```

> Хеширование создаёт в памяти массив размером `колво_элементов * 4`

### Передача парсера
Все динамические данные внутри парсера ведут себя как уникальные, т.е. не дублируются в памяти, также имеется метод `move`. Если нужно создать парсер внутри своего класса, то для корректной работы нужно реализовать move семантику, чтобы объект мог переходить к другим объектам:
```cpp
class MyClass {
    public:
    const char* str;
    Parser parser;

    MyClass(MyClass& p) {
        move(p);
    }
    MyClass& operator=(MyClass& p) {
        move(p);
        return *this;
    }

#if __cplusplus >= 201103L
    MyClass(MyClass&& p) noexcept {
        move(p);
    }
    MyClass& operator=(MyClass&& p) noexcept {
        move(p);
        return *this;
    }
#endif

    void move(MyClass&& p) noexcept {
        parser.move(p.parser);
        str = p.str;
    }
};
```

### Сборка
JSON строка собирается **линейно** в обычную `String`-строку, что очень просто и приятно для микроконтроллера:
```cpp
gson::string gs;                 // создать строку
gs.beginObj();                   // начать объект 1
gs.addString("str1", F("value"));// добавить строковое значение
gs["str2"] = "value2";           // так тоже можно
gs["int"] = 12345;               // целочисленное
gs.beginObj("obj");              // вложенный объект 2
gs.addFloat(F("float"), 3.14);   // float
gs["float2"] = 3.14;             // или так
gs["bool"] = false;              // Bool значение
gs.endObj();                     // завершить объект 2

gs.beginArr("array");            // начать массив
gs.addFloat(3.14);               // в массив - без ключа
gs += "text";                    // добавить значение (в данном случае в массив)
gs += 12345;                     // добавить значение (в данном случае в массив)
gs += true;                      // добавить значение (в данном случае в массив)
gs.endArr();                     // завершить массив

gs.endObj();                     // завершить объект 1

gs.end();                        // ЗАВЕРШИТЬ ПАКЕТ (обязательно вызывается в конце)

Serial.println(gs);              // вывод в порт
Serial.println(gs.s);            // вывод в порт (или так)
```

<a id="versions"></a>

## Версии
- v1.0
- v1.1 - улучшен парсер, добавлено хэширование ключей и обращение по хэш-кодам
- v1.2 - оптимизация под StringUtils 1.3
- v1.3 - оптимизация парсера, ускорение чтения значений из Parser
- v1.4 - оптимизация парсера, ускорение чтения, изначальная строка больше не меняется парсером
- v1.4.1 - поддержка ядра esp8266 v2.x
- v1.4.2 - добавлены Raw методы в string
- v1.4.3 - обновление до актуальной StringUtils, парсинг из su::Text
- v1.4.6 - добавил stringify для Entry
- v1.4.9 - добавлено больше функций addText в gson::string
- v1.5.0 
  - Ускорен парсинг
  - Уменьшен распарсенный вес в оперативке
  - Добавлена семантика move для передачи парсера между объектами как uniq объекта
  - Добавлен парсер из Stream
  - Упразднён Static парсер
  - Мелкие улучшения

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
