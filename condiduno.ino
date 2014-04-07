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

SdFat SD;
SdFile dataFile;
dht DHT;

const int DHT_PIN = 2;
const int SD_PIN1 = 4;
const int SD_PIN2 = 10;
const int LED_PIN = 13;
const byte S7ADDR = 0x71;

// Global Variables
int counter = 0;
int ticker = 0;
int led_state = LOW;
float humi;
float temp;
char temp_string[10];
char tempString[6];
char humiString[6];
boolean rotate = false;


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
    Wire.beginTransmission(S7ADDR);
    for (int i=0; i<5; i++) {
        if (toSend[i] != '.') {
            Wire.write(toSend[i]);
        }
    }
    Wire.endTransmission();
}


/**
 * updateConditions
 *  Read and write current temperature conditions
 */
void updateConditions() {
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

    humi = DHT.humidity;
    temp = DHT.temperature;

    // Display Data
    Serial.print("Counter: ");
    Serial.print(counter, 1);
    Serial.print(" Temperature: ");
    Serial.print(temp, 1);
    Serial.print(" Humidity: ");
    Serial.println(humi, 1);

    // Write Date to SD
    String dataString = "";

    dtostrf(temp, 1, 2, tempString);
    dtostrf(humi, 1, 2, humiString);

    // Write to SD
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

void setup() {
    Serial.begin(115200);
    Serial.println("DHT22 Monitor (2.0)");

    // Initialize Seven Segment Display
    Wire.begin();
    clearDisplay();
    setBrightness(127);
    setDecimals(0b000010);  // Set decimal on position 1 on

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
        Serial.print("Found last couter ");
        Serial.println(lastCounter);
        counter = lastCounter.toInt() + 1;
    }

    // Initiale temperature/humidity read/write
    updateConditions();
}

void loop() {
    if (ticker == 600) {
        // Read DHT Data
        updateConditions();

        // Reset ticker
        ticker = 0;

        // Update counter
        counter++;
    }

    // Every 5 seconds rotate sevenseg details
    if ((ticker % 50) == 0) {
        if (rotate == true) {
            sendString(tempString);
            rotate = false;
        } else {
            sendString(humiString);
            rotate = true;
        }
    }

    ticker++;
    delay(100);  // Delay 100 miliseconds
}
