#include <dht.h>
#include <SdFat.h>

SdFat SD;
SdFile dataFile;
dht DHT;

const int DHT_PIN = 2;
const int SD_PIN1 = 4;
const int SD_PIN2 = 10;
const int LED_PIN = 13;

int counter = 0;
int led_switch = 0;


void setup() {
    Serial.begin(115200);
    Serial.print("DHT22 Monitor (");
    Serial.print(DHT_LIB_VERSION);
    Serial.println(")");

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
}

void loop() {
    // LED Activate
    digitalWrite(LED_PIN, HIGH);

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

    // LED Activate
    digitalWrite(LED_PIN, LOW);

    counter++;
    delay(600000);  // Delay 10 mins
}
