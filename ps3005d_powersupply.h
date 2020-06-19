//
//  ps3005d_powersupply.h
//  PS3005D-DC-Lab-Power-Supply
//
//  Created by Sven Kreiensen on 28.04.13.
//  Copyright © 2013 Sven Kreiensen. All rights reserved.
//

#ifndef ps3005d_powersupply_h
#define ps3005d_powersupply_h

#define PROG_NAME "ps3005d_powersupply"
#define PROG_VERSION "0.1"

//
// PS3005D lab power supply command set
//
#define SETOVERCURRENTPROTECTION    "OCP"
#define SETOVERVOLTAGEPROTECTION    "OVP"

#define SETVOLTAGE    "VSET1:"
#define SETCURRENT    "ISET1:"
#define GETVOLTAGE    "VSET1?"
#define GETCURRENT      "ISET1?"
#define GETOUTPUTVOLTAGE    "VOUT1?"
#define GETOUTPUTCURRENT    "IOUT1?"

#define GETSTATUS    "STATUS?"
#define GETVERSION    "*IDN?"
#define SETOUTPUT    "OUT"
#define RECALLMEM    "RCL"
#define SAVEMEM        "SAV"
#define TRACKINGMODE    "TRACK"

//
// Default settings
//
#if defined(MACOSX)
#define DEFAULT_SERIAL_DEVICE   "/dev/cu.usbserial"
#else
#define DEFAULT_SERIAL_DEVICE   "/dev/ttyACM0"
#endif

#define DEFAULT_BAUD_RATE   9600
#define DEFAULT_RESPONSE_TIMEOUT    2   // 2 seconds

// Gobal Debug flag
int debug = 0;

char buf[BUFSIZ];
int fd;

// Enum for the command ids
enum {
    CMD_NO_CMD=-1,
    CMD_SETVOLTAGE=0,
    CMD_GETVOLTAGE=1,
    CMD_SETCURRENT=2,
    CMD_GETCURRENT=3,
    CMD_SETOVERVOLTAGE=4,
    CMD_SETOVERCURRENT=5,
    CMD_GETOUTPUTVOLTAGE=6,
    CMD_GETOUTPUTCURRENT=7,
    CMD_GETSTATUS=8,
    CMD_GETVERSION=9,
    CMD_SETOUTPUT=10,
    CMD_RECALLMEMORY=11,
    CMD_SAVEMEMORY=12,
    CMD_TRACKINGMODE=13
};

#endif /* ps3005d_powersupply_h */
