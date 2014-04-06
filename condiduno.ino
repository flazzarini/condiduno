/*
    Condiduno

    Analyses temperature/humidity saves it to an SD Card and gives the user the
    possibility the output the readings to a seven segment display by pressing
    a button.


    Wires

    Adruino  ----------------------- 7-Segemnt I2C
    5V       ----------------------- VCC
    GND      ----------------------- GND
    A4 (SDA) ----------------------- SDA
    A5 (SCL) ----------------------- SCL

    Arduino  ----------------------- DHT22 Sensor
    5V       -----------\/---------- LPin1
    PIN2     -----------/\---------- LPin2
    GND      ----------------------- LPin4
*/

#include <dht.h>
#include <SdFat.h>
#include <Wire.h>
#include <Ethernet.h>

SdFat SD;
SdFile dataFile;
dht DHT;

const int DHT_PIN = 2;
const int SD_PIN1 = 4;
const int SD_PIN2 = 10;
const int LED_PIN = 13;
const byte S7ADDR = 0x71;
const int textBuffSize = 9;

/* Global Variables */
int counter = 0;
int ticker = 120;       // 120 * 5sec = 10min
int led_state = LOW;
char temp_string[10];
char textBuff[textBuffSize];
int charsReceived = 0;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server = EthernetServer(4949);
EthernetClient client;

/**
 * clearDisplay
 *  Send clear display command via I2C
 */
void clearDisplay() {
    Wire.beginTransmission(S7ADDR);
    Wire.write(0x76); // Clear display command
    Wire.endTransmission();
}


/**
 * setBrightness
 *  Set brightness of the seven segment display
 *  (0-127-255) (dimmest-brightest)
 */
void setBrightness(byte value)
{
  Wire.beginTransmission(S7ADDR);
  Wire.write(0x7A);  // Set brightness command byte
  Wire.write(value);  // brightness data byte
  Wire.endTransmission();
}


/**
 * setDecimals
 *  Activates the decimal place on the Seven Segment Display
 *   Digits go from left to right
 *  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
 */
void setDecimals(byte decimals) {
    Wire.beginTransmission(S7ADDR);
    Wire.write(0x77);
    Wire.write(decimals);
    Wire.endTransmission();
}


/**
 * sendString
 *  Sends a 4 character string to the 7Segment Display
 */
void sendString(String toSend) {
    setDecimals(0b000010);  // Set decimal on position 1 on
    Wire.beginTransmission(S7ADDR);
    for (int i=0; i<5; i++) {
        if (toSend[i] != '.') {
            Wire.write(toSend[i]);
        }
    }
    Wire.endTransmission();
}


/**
 * blinkLed
 *  Activates LED for a brief short time
 */
void blinkLed() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}


void setup() {
    Serial.begin(115200);
    Serial.print("DHT22 Monitor (");
    Serial.print(DHT_LIB_VERSION);
    Serial.println(")");

    // Initialize Seven Segment Display
    Wire.begin();
    clearDisplay();
    setBrightness(127);

    // Initialize LED on PIN 13
    pinMode(LED_PIN, OUTPUT);

    // Initialize SD Card
    pinMode(SD_PIN2, OUTPUT);
    if (!SD.begin(SD_PIN1, SPI_HALF_SPEED)) {
        Serial.println("SD Card not present, please reset!");
        SD.initErrorHalt();
    }

    // Retrieve last counter
    ifstream sdin("temphumi.txt");
    int bufferLength = 20;
    char buffer[bufferLength];
    String lastCounter;

    while (sdin.getline(buffer, bufferLength, '\n') || sdin.gcount()) {
        String counterString(buffer);
        lastCounter = counterString.substring(0, counterString.indexOf(","));
    }

    if (lastCounter) {
        counter = lastCounter.toInt() + 1;
    }

    // Ethernet
    Ethernet.begin(mac);
    server.begin();
    Serial.print("Server running at ");
    Serial.print(Ethernet.localIP());
    Serial.println(" on port 4949");
}

void loop() {
    // Read DHT Data
    int chk = DHT.read22(DHT_PIN);
    switch (chk)
    {
        case DHTLIB_OK:
            break;
        case DHTLIB_ERROR_CHECKSUM:
            Serial.println("Checksum error");
            break;
        case DHTLIB_ERROR_TIMEOUT:
            Serial.println("Time out error");
            break;
        default:
            Serial.println("Unknown error");
            break;
    }

    float humi = DHT.humidity;
    float temp = DHT.temperature;

    // Display Data
    Serial.print("Counter: ");
    Serial.print(counter, 1);
    Serial.print(" Temperature: ");
    Serial.print(temp, 1);
    Serial.print(" Humidity: ");
    Serial.println(humi, 1);

    // Write Date to SD
    String dataString = "";

    char tempString[6];
    char humiString[6];
    dtostrf(temp, 1, 2, tempString);
    dtostrf(humi, 1, 2, humiString);

    // Send String to Sevensegment
    if ((ticker % 2) == 0) {
        sendString(tempString);
    } else {
        sendString(humiString);
    }

    EthernetClient client = server.available();
    if (client && client.connected()) {
        blinkLed();
    }

    // Write to SD
    if (ticker == 120) {
        dataString += String(counter);
        dataString += ", ";
        dataString += String(tempString);
        dataString += ", ";
        dataString += String(humiString);
        Serial.print("Writing to SD Card: ");
        Serial.println(dataString);

        dataFile.open("temphumi.txt", O_RDWR | O_CREAT | O_AT_END);
        dataFile.println(dataString);
        dataFile.close();
    }

    counter++;
    ticker++;
    delay(5000);  // Delay 5 second
}
