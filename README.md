[![latest](https://img.shields.io/github/v/release/GyverLibs/GSON.svg?color=brightgreen)](https://github.com/GyverLibs/GSON/releases/latest/download/GSON.zip)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD$%E2%82%AC%20%D0%9D%D0%B0%20%D0%BF%D0%B8%D0%B2%D0%BE-%D1%81%20%D1%80%D1%8B%D0%B1%D0%BA%D0%BE%D0%B9-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/GSON?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# GSON
Парсер и сборщик данных в формате JSON для Arduino
- В 7 раз быстрее и сильно легче ArduinoJSON
- Парсинг JSON с обработкой ошибок
- Линейная сборка JSON-пакета
- Экранирование "опасных" символов
- Работает на базе AnyText (StringUtils)
- Работает с 64 битными числами
- *Библиотека не подходит для хранения данных! Только парсинг и сборка с нуля*

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
### `gson::Doc`
```cpp
// конструктор
gson::Doc();
gson::Doc(размер);
gson::DocStatic<размер>();

// методы
uint16_t length();              // получить количество элементов
uint16_t size();                // получить размер документа в оперативной памяти (байт)
void hashKeys();                // хешировать ключи всех элементов (операция необратима)
bool hashed();                  // проверка были ли хешированы ключи

Entry get(AnyText key);         // доступ по ключу (главный контейнер - Object)
Entry get(size_t hash);         // доступ по хэшу ключа (главный контейнер - Object)
Entry get(int index);           // доступ по индексу (главный контейнер - Array или Object)

AnyText key(int idx);           // прочитать ключ по индексу
size_t keyHash(int idx);        // прочитать хэш ключа по индексу
AnyText value(int idx);         // прочитать значение по индексу
int8_t parent(int idx);         // прочитать родителя по индексу
Type type(int idx);             // получить тип по индексу

const __FlashStringHelper* readType(uint16_t idx);  // прочитать тип по индексу

// парсить. Вернёт true при успешном парсинге. Можно указать макс. уровень вложенности
bool parse(String& json, uint8_t maxdepth = 16);
bool parse(const char* json, uint8_t maxdepth = 16);

// вывести в Print с форматированием
void stringify(Print* p);

// обработка ошибок
bool hasError();                        // есть ошибка парсинга
Error getError();                       // получить ошибку
uint16_t errorIndex();                  // индекс места ошибки в строке
const __FlashStringHelper* readError(); // прочитать ошибку
```

#### Лимиты
- После парсинга один элемент весит 6 байт на AVR и 12 байт на ESP
- Максимальное количество элементов (ограничение счётчика) - 255 на AVR и 65535 на ESP
- Максимальный уровень вложенности задаётся в функции `parse()`. Парсинг - рекурсивный, каждый уровень добавляет несколько байт в оперативку

### Тесты
Тестировал на ESP8266, пакет сообщений из телеграм бота - 3500 символов, 8 сообщений, 180 "элементов". Получал значение самого отдалённого и вложенного элемента, в GSON - через хэш. Результат:

| Либа        | Flash  | SRAM  | FreeHeap | Parse   | Hash   | Get    |
|-------------|--------|-------|----------|---------|--------|--------|
| ArduinoJson | 285525 | 28256 | 45648    | 9900 us | -      | 158 us |
| GSON        | 275193 | 28076 | 46664    | 1400 us | 324 us | 156 us |

Таким образом GSON **в 7 раз быстрее** при парсинге, значения элементов получает с такой же скоростью. Сама библиотека легче на 10 кБ во Flash и 1 кБ в Heap. В других тестах (AVR) на получение значения GSON с хэшем работал в среднем в 1.5 раза быстрее.

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
```

### `gson::Entry`
Также наследует всё из `AnyText`, документация [здесь](https://github.com/GyverLibs/StringUtils?tab=readme-ov-file#anytext)

```cpp
Entry get(AnyText key);     // получить элемент по ключу
bool includes(AnyText key); // содержит элемент с указанным ключом

Entry get(size_t hash);     // получить элемент по хэшу ключа
bool includes(size_t hash); // содержит элемент с указанным хэшем ключа

Entry get(int index);       // получить элемент по индексу

bool valid();               // проверка корректности (существования)
uint16_t length();          // получить размер (для объектов и массивов. Для остальных 0)
Type type();                // получить тип элемента
AnyText key();              // получить ключ
size_t keyHash();           // получить хэш ключа
AnyText value();            // получить значение
```

### `gson::string`
```cpp
String s;                   // доступ к строке
void clear();               // очистить строку
bool reserve(uint16_t res); // зарезервировать строку

// прибавить gson::string. Будет добавлена запятая
string& add(string& str);

// добавить ключ (строка любого типа)
string& addKey(AnyText key);

// прибавить текст (строка любого типа)
string& addRaw(AnyText str, bool esc = false);

// добавить строку (строка любого типа)
string& addString(AnyText key, AnyText value);
string& addString(AnyText value);

// добавить bool
string& addBool(AnyText key, const bool& value);
string& addBool(const bool& value);

// добавить float
string& addFloat(AnyText key, const double& value, uint8_t dec = 2);
string& addFloat(const double& value, uint8_t dec = 2);

// добавить int
string& addInt(AnyText key, const AnyValue& value);
string& addInt(const AnyValue& value);

string& beginObj(AnyText key = "");   // начать объект
string& endObj();   // завершить объект

string& beginArr(AnyText key = "");   // начать массив
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
gson::Doc doc;              // динамический документ
gson::Doc doc(10);          // динамический документ, зарезервирован под 10 элементов
gson::DocStatic<10> doc;    // статический документ, зарезервирован под 10 элементов
```

Смысл как у `String`-строки: динамический документ будет увеличиваться в процессе парсинга в динамической памяти МК, если размер документа неизвестен. Можно заранее зарезервировать размер, чтобы парсинг происходил быстрее. Статический - выделяется статически, используя меньше памяти на слабых платформах.

```cpp
// получили json
char json[] = R"raw({"key":"value","int":12345,"obj":{"float":3.14,"bool":false},"arr":["hello",true]})raw";
String json = "{\"key\":\"value\",\"int\":12345,\"obj\":{\"float\":3.14,\"bool\":false},\"arr\":[\"hello\",true]};";

// парсить
doc.parse(json);

// обработка ошибок
if (doc.hasError()) {
    Serial.print(doc.readError());
    Serial.print(" in ");
    Serial.println(doc.errorIndex());
} else Serial.println("done");
```

После парсинга можно вывести весь пакет с типами, ключами, значениями в виде текста и родителем:
```cpp
for (uint16_t i = 0; i < doc.length(); i++) {
    // if (doc.type(i) == gson::Type::Object || doc.type(i) == gson::Type::Array) continue; // пропустить контейнеры
    Serial.print(i);
    Serial.print(". [");
    Serial.print(doc.readType(i));
    Serial.print("] ");
    Serial.print(doc.key(i));
    Serial.print(":");
    Serial.print(doc.value(i));
    Serial.print(" {");
    Serial.print(doc.parent(i));
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

// проверка типа
doc["arr"].type() == gson::Type::Array;

// вывод содержимого массива
for (int i = 0; i < doc["arr"].length(); i++) {
    Serial.println(doc["arr"][i]);
}

// а лучше - так
gson::Entry arr = doc["arr"];
for (int i = 0; i < arr.length(); i++) {
    Serial.println(arr[i]);
}

// Пусть json имеет вид [[123,456],["abc","def"]], тогда ко вложенным массивам можно обратиться:
Serial.println(doc[0][0]);  // 123
Serial.println(doc[0][1]);  // 456
Serial.println(doc[1][0]);  // abc
Serial.println(doc[1][1]);  // def
```

Каждый элемент можно вывести в тип `gson::Entry` по имени (из объекта) или индексу (из массива) и использовать отдельно, чтобы не "искать" его заново:
```cpp
gson::Entry e = doc["arr"];
Serial.println(e.length());  // длина массива
Serial.println(e[0]);        // hello
Serial.println(e[1]);        // true
```

### Хэширование
GSON нативно поддерживает хэш-строки из StringUtils, работа с хэшами значительно увеличивает скорость доступа к элементам JSON документа по ключу. Для этого нужно:

1. Хэшировать ключи. **Данная операция необратима**: ключи в текстовом виде уже нельзя будет прочитать из документа:
```cpp
doc.hashKeys();
```

2. Обращаться к элементам по хэшам ключей, используя функцию `sutil::SH`:
```cpp
using sutil::SH;

void foo() {
    Serial.println(doc[SH("int")]);
    Serial.println(doc[SH("obj")][SH("float")]);
    Serial.println(doc[SH("array")][0]);
}
```

Строка, переданная в `SH`, вообще **не существует в программе** и не занимает места: хэш считается компилятором на этапе компиляции, вместо него подставляется число. А сравнение чисел выполняется быстрее, чем сравнение строк.

Примечания:
- Функция `keyHash()` вернёт:
    - Ранее посчитанный хэш, если была вызвана `hashKeys()`
    - Если не вызвана - посчитает и вернёт хэш
- Функция `key()` вернёт:
    - Валидную строку, если не был посчитан хэш `hashKeys()`
    - Пустую строку, если был посчитан

3. Для расчёта и получения хэша значения можно просто применить `hash()` из `AnyText`
```cpp
switch (doc[SH("str")].hash()) {
    case SH("some text"):
    // ...
}
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