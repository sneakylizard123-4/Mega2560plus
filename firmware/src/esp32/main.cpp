#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MEGA_RX 5
#define MEGA_TX 4
#define I2C_SCL 6
#define I2C_SDA 7
#define IMU_CS 15
#define IMU_SCK 18
#define IMU_MOSI 17
#define IMU_MISO 16
#define FLASH_CS 10
#define FLASH_SCK 13
#define FLASH_MOSI 11
#define FLASH_MISO 12
#define SD_CS 9
#define SD_DET 8
#define RTC_INT 38
#define NPX_PIN 39
#define NPX_COUNT 8

#define BME680_ADDR 0x76
#define SCD41_ADDR 0x62
#define DS3231_ADDR 0x68
#define LSM6DSL_ADDR 0x6A
#define FLASH_JEDEC_ID 0xEF4018

#define I2C_FREQ 400000
#define SENSOR_POLL_MS 5000

Adafruit_NeoPixel npx(NPX_COUNT, NPX_PIN, NEO_GRB + NEO_KHZ800);
SPIClass imuSpi(HSPI);

static uint8_t bcdToDec(uint8_t b)
{
    return ((b >> 4) * 10) + (b & 0x0F);
}

static bool scanAddress(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

static bool readRegI2C(uint8_t addr, uint8_t reg, uint8_t* out, size_t len)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0)
        return false;
    Wire.requestFrom((uint8_t)addr, (uint8_t)len);
    for (size_t i = 0; i < len && Wire.available(); i++)
        out[i] = Wire.read();
    return true;
}

static bool readBME680Id()
{
    uint8_t id = 0;
    return readRegI2C(BME680_ADDR, 0xD0, &id, 1) && id == 0x61;
}

static void readSCD41Serial(uint32_t* serial)
{
    Wire.beginTransmission(SCD41_ADDR);
    Wire.write(0x36);
    Wire.write(0x80);
    Wire.endTransmission();
    delay(10);
    uint8_t raw[9] = { 0 };
    Wire.requestFrom((uint8_t)SCD41_ADDR, (uint8_t)9);
    for (int i = 0; i < 9 && Wire.available(); i++)
        raw[i] = Wire.read();
    *serial = ((uint32_t)raw[0] << 16) | ((uint32_t)raw[1] << 8) | raw[2];
}

static uint32_t readDS3231Time()
{
    uint8_t regs[7] = { 0 };
    if (!readRegI2C(DS3231_ADDR, 0x00, regs, 7))
        return 0;
    return ((uint32_t)bcdToDec(regs[6]) << 24) | ((uint32_t)bcdToDec(regs[5]) << 16) |
           ((uint32_t)bcdToDec(regs[4]) << 8) | bcdToDec(regs[3]);
}

static bool readIMUWhoAmI()
{
    imuSpi.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);
    imuSpi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(IMU_CS, LOW);
    imuSpi.transfer(0x80 | 0x0F);
    uint8_t who = imuSpi.transfer(0x00);
    digitalWrite(IMU_CS, HIGH);
    imuSpi.endTransaction();
    return who == LSM6DSL_ADDR;
}

static bool readFlashJedec(uint8_t* jedec)
{
    SPI.begin(FLASH_SCK, FLASH_MISO, FLASH_MOSI, FLASH_CS);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(FLASH_CS, LOW);
    SPI.transfer(0x9F);
    for (int i = 0; i < 3; i++)
        jedec[i] = SPI.transfer(0x00);
    digitalWrite(FLASH_CS, HIGH);
    SPI.endTransaction();
    return jedec[0] == 0xEF && jedec[1] == 0x40 && jedec[2] == 0x18;
}

static void printSensorReport()
{
    uint32_t now = millis();
    uint32_t serial = 0;
    readSCD41Serial(&serial);

    Serial.println();
    Serial.println("--- Mega2560+ sensor report ---");
    Serial.print("I2C bus: ");
    Serial.print(scanAddress(BME680_ADDR) ? "BME680 " : "");
    Serial.print(scanAddress(SCD41_ADDR) ? "SCD41 " : "");
    Serial.print(scanAddress(DS3231_ADDR) ? "DS3231 " : "");
    Serial.println();
    Serial.print("BME680 id  : ");
    Serial.println(readBME680Id() ? "0x61 OK" : "missing");
    Serial.print("SCD41 ser  : ");
    Serial.println(serial, HEX);
    uint32_t dt = readDS3231Time();
    if (dt)
    {
        Serial.print("RTC (sec/date/mon/yr): ");
        Serial.println(dt, DEC);
    }
    Serial.print("LSM6DSL whoami: ");
    Serial.println(readIMUWhoAmI() ? "0x6A OK" : "missing");
    uint8_t jedec[3] = { 0 };
    bool flash = readFlashJedec(jedec);
    Serial.print("W25Q128 JEDEC: ");
    if (flash)
    {
        Serial.print("0x");
        for (int i = 0; i < 3; i++)
        {
            if (jedec[i] < 0x10)
                Serial.print("0");
            Serial.print(jedec[i], HEX);
        }
        Serial.println(" OK");
    }
    else
    {
        Serial.println("missing");
    }
    Serial.printf("uptime %us\n", now / 1000);
    Serial.println("-----------------------------");
}

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, MEGA_RX, MEGA_TX);
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);

    pinMode(IMU_CS, OUTPUT);
    digitalWrite(IMU_CS, HIGH);
    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(SD_DET, INPUT_PULLUP);
    pinMode(RTC_INT, INPUT);

    npx.begin();
    npx.show();

    Serial.println("Mega2560+ ESP32 co-processor ready");
    printSensorReport();
}

void loop()
{
    static uint32_t lastPoll = 0;
    uint32_t now = millis();

    while (Serial1.available())
        Serial.write(Serial1.read());
    while (Serial.available())
        Serial1.write(Serial.read());

    if (now - lastPoll >= SENSOR_POLL_MS)
    {
        lastPoll = now;
        printSensorReport();
    }

    uint16_t hue = (now >> 2) % 360;
    for (int i = 0; i < NPX_COUNT; i++)
        npx.setPixelColor(i, npx.ColorHSV(hue + i * 45));
    npx.show();
}
