# Mega 2560+ Firmware

PlatformIO firmware for the Mega 2560+ board. The board has two MCUs, each with its own environment:

| MCU | Env | Role |
|-----|-----|------|
| ATmega2560-16A | `mega2560` | Main MCU, drop-in Mega compatible. USB-UART via FT231XS, FM24C64C FRAM on I2C. |
| ESP32-S3-WROOM-1U-N16R8 | `esp32s3` | Co-processor. WiFi/BT, sensor suite (I2C), IMU + SPI flash (SPI). |

## Build

```bash
cd firmware
pio run -e mega2560
pio run -e esp32s3
```

Outputs: `.pio/build/mega2560/firmware.hex` and `.pio/build/esp32s3/firmware.bin`.

## Flash

```bash
# Mega 2560 (over USB via FT231XS, like any Mega)
pio run -e mega2560 -t upload

# ESP32-S3 (over native USB in download mode: hold IO0/BTN_BOOT, plug in)
pio run -e esp32s3 -t upload
```

## MCU Link

The two MCUs talk over UART through two SN74LVC1T45 level translators:

| Signal | Mega | ESP32-S3 |
|--------|------|----------|
| TX | Serial3 TX (D14) | UART1 RX (GPIO5) |
| RX | Serial3 RX (D15) | UART1 TX (GPIO4) |

Baud: 115200. The link is a raw byte bridge right now — Mega's `Serial` (USB) is relayed to the ESP32 `Serial` (USB CDC) and back. A structured command protocol can be layered on later.

## ESP32-S3 Pin Map

### I2C1 (sensor bus, 3.3V, 400kHz)

| Pin | Signal |
|-----|--------|
| GPIO6 | SCL |
| GPIO7 | SDA |

| Device | Address |
|--------|---------|
| SCD41 (CO2) | 0x62 |
| BME680 | 0x76 |
| DS3231M RTC | 0x68 |

### SPI3 (IMU, 1MHz)

| Pin | Signal |
|-----|--------|
| GPIO18 | SCK |
| GPIO17 | MOSI |
| GPIO16 | MISO |
| GPIO15 | CS (LSM6DSL) |

### SPI (flash, 1MHz)

| Pin | Signal |
|-----|--------|
| GPIO13 | SCK |
| GPIO11 | MOSI |
| GPIO12 | MISO |
| GPIO10 | CS (W25Q128) |

### Other

| Pin | Signal |
|-----|--------|
| GPIO39 | WS2812B NeoPixel data (8 LEDs) |
| GPIO2 | User LED |
| GPIO1 | microSD activity LED |
| GPIO8 | microSD detect |
| GPIO9 | microSD CS |
| GPIO14 / GPIO21 | IMU INT1 / INT2 |
| GPIO38 | RTC interrupt / SQW |
| GPIO0 | BOOT |

## Mega 2560 Pin Map

| Signal | Pin |
|--------|-----|
| USB-UART | Serial0 (D0/D1) via FT231XS |
| ESP32 link | Serial3 (D14/D15) |
| FRAM (FM24C64C) | I2C, address 0x50 (D20/D21) |
