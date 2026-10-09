# ESP32 pins

This is for the 30-pin esp32 devkitv1

## Use

GPIO 4, 13, 14, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33

## Never use

Please ignore the below, not very relevant

Never use GPIO 6–11. 
(already guaranteed by devboard, this is actually for the WROOM chip itsel)

## Probably don't use

Avoid GPIO 1 and 3 if you're using PlatformIO's serial monitor. 
They are the ESP32's default UART0:

The ESP32 samples several GPIOs during reset to decide how it should boot. These are:

GPIO 0
GPIO 2
GPIO 5
GPIO 12
GPIO 15

You can use them for normal GPIO functionality, but an external circuit connected to one of
them can prevent the ESP32 from booting if it forces the wrong level at reset.

So, avoid them.

## Input only

GPIO 34
GPIO 35
GPIO 36 / VP
GPIO 39 / VN

## ADC

ADC1:
GPIO32
GPIO33
GPIO34
GPIO35
GPIO36
GPIO39

ADC2:
GPIO0
GPIO2
GPIO4
GPIO12
GPIO13
GPIO14
GPIO15
GPIO25
GPIO26
GPIO27

ESP32's Wi-Fi subsystem conflicts with ADC2, so if you need analog measurements while Wi-Fi is active, use GPIO32–39 rather than ADC2 pins.

## Summary

```text
GOOD GENERAL GPIO
4  13 14 16 17 18 19 21 22 23 25 26 27 32 33

INPUT ONLY
34 35 36 39

SERIAL / BOOT
0 1 2 3 5 12 15

INTERNAL FLASH — DO NOT TOUCH
6 7 8 9 10 11
```

## "Pin to pin"

```text
                 USB
                  ▲
          ┌───────────────────┐
     30 VIN │                 │ 3V3   1
     29 GND │                 │ GND   2
     28 GPIO13│               │ GPIO15 3
     27 GPIO12│               │ GPIO2  4
     26 GPIO14│               │ GPIO4  5
     25 GPIO27│               │ GPIO16 6
     24 GPIO26│               │ GPIO17 7
     23 GPIO25│               │ GPIO5  8
     22 GPIO33│               │ GPIO18 9
     21 GPIO32│               │ GPIO19 10
     20 GPIO35│               │ GPIO21 11
     19 GPIO34│               │ GPIO3  12
     18 GPIO39│               │ GPIO1  13
     17 GPIO36│               │ GPIO22 14
     16 EN    │               │ GPIO23 15
          └───────────────────┘
                  ▼
```                
