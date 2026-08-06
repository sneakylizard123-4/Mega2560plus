---
title: "Mega 2560+"
author: sneak
description: "Upgraded Arduino Mega 2560 with an onboard sensor suite and ESP32-S3 co-processor for WiFi and Bluetooth"
created_at: "2026-08-03T00:00:00Z"
---



# August 3: project start

started the mega 2560+ project. wanted to make a beefed-up arduino mega that can sense the room around it and talk to the network without needing external modules. the idea is a mega you can just drop into projects that already knows the temp, co2, air quality, and motion, and has wifi/bluetooth built in.

main mcu is the atmega2560-16a obviously, since its the same chip the real mega uses and i know the ecosystem. added an esp32-s3 as a co-processor for wifi/bt and sensor reading, it handles the wireless stuff so the mega can focus on user code.

set up the kicad project as hierarchical sheets so each block is manageable - power, mega2560, esp32, sensors, peripherals, headers, translator, usbuart. dumped all the datasheets in and started placing the big parts.
![root schematic](images/schematics/01-Root.png)

**Total time spent: 5 hours**

# August 3: wiring up the mega sheet

focused on the mega2560 sheet. the atmega2560 has a ton of pins and i wanted to get the core right first - decoupling on every power pin, the 16mhz crystal with load caps, reset circuit.

this sheet has the fm24c64c fram and w25q128jvs spi flash hooked up, plus the micro sd slot. grabbed an rtc too (ds3231m) so the board can keep time and date over i2c.

was a long slog of wiring. the pin mapping on the mega is 100 pins and i kept cross-referencing the schematic against the real mega pinout so headers stay drop-in compatible with existing mega shields.
![mega2560 sheet](images/schematics/04-MEGA2560.png)

**Total time spent: 4 hours**

# August 3: sensors and esp32

did the sensors sheet - bme680 for temp/humidity/pressure/air quality, scd41 for actual co2 readings, lsm6dsl imu for motion. all on the same i2c bus with different addresses, which is nice and tidy.

the esp32-s3 sheet took the longest. it talks to the mega over uart, and needs its own flash. had to think through the power - esp32 is 3.3v, mega is 5v, so i added an sn74lvc1t45 level translator for the uart lines so they can actually talk without magic smoke.

also threw in the ws2812b-2020 leds, tiny little addressable leds that will look cool as status indicators.
![esp32 sheet](images/schematics/03-ESP32.png)
![sensors](images/schematics/06-Sensors.png)

**Total time spent: 5 hours**

# August 4: finished schematic, started pcb

schematic is done and passing erc. usbuart sheet with the ft231xs and usb-c connector, esd protection on the data lines. power sheet with the tps62163 buck for the main rail and tlv75733 ldo for 3.3v, barrel jack plus a battery option with the polyfuse and mbr0520 so you cant plug things in backwards and destroy everything.

started the pcb layout. this board is going to be huge - its a mega after all, keeping the same form factor so it drops into mega shields. got all the footprints assigned and started placing the big stuff. the mega pin headers take up most of the board, the fun parts go in the middle.
![usbuart sheet](images/schematics/02-UsbUart.png)
![power](images/schematics/05-Power.png)

**Total time spent: 6 hours**

# August 5: 3d models and finishing the pcb

tracked down proper step models for the scd41 co2 sensor and the ws2812b leds so the 3d view isnt just a pile of boxes. the scd41 one came straight from sensirion, the ws2812b i grabbed from somewhere sketchy but it looks right.

routed the whole board. the interesting bit was keeping the antenna area of the esp32 clear and getting the switching regulator traces clean. ran drc and it passed first try which shocked me. the board is a chunky 100x53mm which is basically a mega, i love it.
![pcb layout](images/pcb-render-top.png)

**Total time spent: 7 hours**

# August 6: spinning render

exported a spinning render of the board to see it from all angles. satisfying seeing the whole thing come together. next up is bom export and picking lcsc parts, and probably some easter eggs on the silkscreen.
![3d render](images/3d-render.png)

**Total time spent: 1 hour**
