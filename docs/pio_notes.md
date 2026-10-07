# Platform IO CLI notes 

## Check if it compiles

`pio run`

## Upload

`pio run -t upload`

## TIP: reset esp32

EN pushbutton = reset/restart the ESP32

## Serial monitor

```sh
pio device monitor
```

Please note this is unlikely to suffice. In my computer the default setting
when doing the monitor is `--- Terminal on COM3 | 9600 8-N-1`

Change the baud rate from 9600 to 115200:

```sh
pio device monitor -b 115200
```

## Selecting the device

```
pio device list
```

Then use `--upload-port` for `pio run`.

e.g.

```sh
pio run -t upload --upload-port COM3 # Windows
pio run -t upload --upload-port /dev/ttyUSB0 # Unix
pio run -t upload --upload-port /dev/ttyACM0 # Unix
```

And `-p` for `pio monitor`.

```sh
pio device monitor -b 115200 -p COM3 # Windows
pio device monitor -b 115200 -p /dev/ttyUSB0 # Unix
pio device monitor -b 115200 -p /dev/ttyACM0 # Unix
```