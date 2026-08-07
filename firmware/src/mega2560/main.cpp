#include <Arduino.h>
#include <Wire.h>

#define FRAM_ADDR 0x50
#define FRAM_SIZE 8192
#define ESP_SERIAL Serial3

static void framWrite(uint16_t addr, const uint8_t* data, size_t len)
{
    Wire.beginTransmission(FRAM_ADDR);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.write(data, len);
    Wire.endTransmission();
}

static void framRead(uint16_t addr, uint8_t* data, size_t len)
{
    Wire.beginTransmission(FRAM_ADDR);
    Wire.write((uint8_t)(addr >> 8));
    Wire.write((uint8_t)(addr & 0xFF));
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)FRAM_ADDR, (uint8_t)len);
    for (size_t i = 0; i < len && Wire.available(); i++)
        data[i] = Wire.read();
}

void setup()
{
    Serial.begin(115200);
    ESP_SERIAL.begin(115200);
    Wire.begin();
    Wire.setClock(400000);

    const uint8_t magic[] = { 'M', '2', '5', '6', '0' };
    uint8_t check[sizeof(magic)] = { 0 };
    framWrite(0x0000, magic, sizeof(magic));
    framRead(0x0000, check, sizeof(check));

    Serial.println("Mega2560+ ready");
    Serial.print("FRAM check: ");
    Serial.println(memcmp(magic, check, sizeof(magic)) == 0 ? "OK" : "FAIL");
}

void loop()
{
    while (Serial.available())
        ESP_SERIAL.write(Serial.read());
    while (ESP_SERIAL.available())
        Serial.write(ESP_SERIAL.read());
}
