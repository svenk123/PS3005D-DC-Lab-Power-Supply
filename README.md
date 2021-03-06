# PS3005D-DC-Lab-Power-Supply
This is a command line tool to control a PS3005D compabtile bench top lab power supplies.

These programmable power supplies have the following specifications

| Feature | Description |
|-----|--------|
| Channels | 1
| Voltage range | 0..30 V
| Current range | 0..5 A
| Features | over-voltage protection, over-current protection, manual on/off
| Connectivity | USB/serial, RS232

Supported devices:

* Velleman [PS3005D](https://www.velleman.eu/products/view?id=409798) 
* Velleman [LABPS3005D](https://www.velleman.eu/products/view?id=417862)

and compatible devices (not tested):

* Korad KA3005P
* RND [320-KA3005P](https://www.distrelec.de/en/bench-top-power-supply-30-programmable-rnd-lab-rnd-320-ka3005p/p/30061864)
* Tenma [72-2540](http://uk.farnell.com/tenma/72-2540/power-supply-1ch-30v-5a-prog/dp/2445412)

## Usage

Usage ps3005d_powersupply [options...]

| parameter | description
| ------------ | ------------------|
| -t <timeout>      | socket response timeout in seconds
| -b <baudrate>     | 9600|19200 (default=19200)
| -d <device>       | tty device file (default: /dev/ttyACM0 on Linux, /dev/cu.usbserial on Mac OSX)
 | -q              | query status
| -u <voltage>      | set or get voltage (0.0..30.0)
| -i <current>      | set or get current (0.0..5.0)
| -x 0\|1          | turn on (1)/off (0) overvoltage protection
| -y 0\|1          | turn on (1)/off (0) overcurrent protection
| -U              | get output voltage
| -I              | get output current
| -V              | get device information
| -v              | turn on verbosity
| -o 0\|1          | turn on (1)/off (0) output
| -r bank         | recall memory
| -s bank         | save memory
| -m mode        | set tracking mode (0=independent, 1=series, 2=parallel)

## Platforms
This tool can be compile on linux or Mac OS X.
Linux is the default platform. Uncomment the following line in the Makefile for compilation on Mac OS X.

Makefile:
```
OS ?= MACOSX
```

## Installation
Compile the ps3005d_powersupply executable and install it.

```bash
make
make install
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://choosealicense.com/licenses/mit/)
    
## Support
This project was developed for the public domain and as such it is unsupported. I will listen to problems and suggestions for improvement. I wish you my best and hope that everything works out.
