//
//  ps3005d_powersupply.c
//  PS3005D-DC-Lab-Power-Supply
//
//  Created by Sven Kreiensen on 28.04.13.
//  Copyright © 2013 Sven Kreiensen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/param.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>

#include "ps3005d_powersupply.h"

//
// waits until descriptor becomes readable and timeout after seconds
//
static int waitfor(int socket_handle, int timeout) {
    fd_set rfds;
    struct timeval tv;
    int ret;

    FD_ZERO(&rfds);
    FD_SET(socket_handle, &rfds);

    /* Define timeout */
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    ret = select(socket_handle+1, &rfds, NULL, NULL, &tv);
    
    return ret;
}
    
//
// tries hard to read from descriptor bytes into a buffer or returns after a timeout
// Returns the number of read bytes
//
static int tryread (char *buf, int len, int timeout) {
    int i = 0;
    unsigned char tmp_buf = '\0';
    int count=0;
    
    while (i<len) {
        if (!waitfor(fd, timeout)) {
            fprintf(stderr, "Error: Timeout.\n");
            return -1;
        }

        count=read(fd, &tmp_buf, 1);
        if (count < 0) {
            fprintf(stderr, "Error: read() failed.\n");
            return -1;
        }

        //printf("r: %c\n", tmp_buf);
    
        /* Read byte */
        buf[i]=tmp_buf;
        i++;
    
        // Do nothing and go inside the loop
    }

    buf[i]='\0';

    if (debug)
        printf("read: %s\n", buf);

    return i;
}

//
// Send command to the lab power supply
//
static int send_command(char *cmd) {
    if (debug) {
        printf("s: %s\n", cmd);
    }

    if (write(fd, cmd, strlen(cmd)) < 0) {
        fprintf(stderr, "ERROR: write() failed\n");
        return -1;
    }

    return 0;
}

//
// Called on application termination
//
static void termination_handler(int signum) {
    if (fd)
        close(fd);
}

//
// Open a serial port device
// Returns the device handle
//
static int open_serial_port(char *serialport, int baudrate) {
    int sockfd;
    struct termios options;
   
    //if ((sockfd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
    if ((sockfd = open(serialport, O_RDWR)) < 0) {
        fprintf(stderr, "ERROR: Failed to open device %s\n", serialport);
        return -1;
    }

    fcntl(sockfd, F_SETFL, 0);

    // get the current options
    tcgetattr(sockfd, &options);

    switch(baudrate) {
        case 9600:
            options.c_cflag    = B9600 | CS8 | CLOCAL | CREAD;
            break;
        case 19200:
            options.c_cflag    = B19200 | CS8 | CLOCAL | CREAD;
            break;
    }

    // input modes - clear indicated ones giving: no break, no CR to NL,
    // no parity check, no strip char, no start/stop output (sic) control
    options.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // local modes - clear giving: echoing off, canonical off (no erase with
    // backspace, ^U,...),  no extended functions, no signal chars (^Z,^C)
    options.c_lflag     &= ~(ICANON | ECHO | IEXTEN | ISIG);
    
    // output modes - clear giving: no post processing such as NL to CR+NL
    options.c_oflag     &= ~OPOST;

    // set raw input, 1 second timeout
    options.c_cc[VMIN]  = 1;
    options.c_cc[VTIME] = 0;

    // set the options
    //tcsetattr(sockfd, TCSAFLUSH, &options);

    return sockfd;
}

//
// Print a command line help
//
void usage() {
    fprintf(stderr, "%s v%s\nCopyright (c) 2013 Sven Kreiensen\n\n", PROG_NAME, PROG_VERSION);
    fprintf(stderr, "Usage %s [options...]\n\n", PROG_NAME);
    fprintf(stderr, "   -t timeout      socket response timeout\n");
    fprintf(stderr, "   -b baudrate     9600|19200 (default=%s)\n", DEFAULT_BAUD_RATE);
    fprintf(stderr, "   -d device       tty device file (default: %s)\n", DEFAULT_SERIAL_DEVICE);
    fprintf(stderr, "    -q              query status\n");
    fprintf(stderr, "   -u voltage      set or get voltage (0.0..30.0)\n");
    fprintf(stderr, "   -i current      set or get current (0.0..5.0)\n");
    fprintf(stderr, "   -x 0|1          turn on/off overvoltage protection\n");
    fprintf(stderr, "   -y 0|1          turn on/off overcurrent protection\n");
    fprintf(stderr, "   -U              get output voltage\n");
    fprintf(stderr, "   -I              get output current\n");
    fprintf(stderr, "   -V              get device information\n");
    fprintf(stderr, "   -v              turn on verbosity\n");
    fprintf(stderr, "   -o 0|1          turn on/off output\n");
    fprintf(stderr, "   -r bank         recall memory\n");
    fprintf(stderr, "   -s bank         save memory\n");
    fprintf(stderr, "   -m mode         set tracking mode (0=independent, 1=series, 2=parallel)\n");
}

//
// Main function
//
int main(int argc, char **argv) {
    int c;
    int ret = 0;
    int timeout = DEFAULT_RESPONSE_TIMEOUT; // seconds
    int baudrate = DEFAULT_BAUD_RATE;
    char serialport[255];
    strcpy(serialport, DEFAULT_SERIAL_DEVICE);
    int command = CMD_NO_CMD;
    float voltage = 0.0f;
    float current = 0.0f;
    int overcurrentprotection = 0;
    int overvoltageprotection = 0;
    int output_on = 0;
    int bank = 0;
    int tracking_mode = 0;
    
    // Print command line help when called without command line parameters
    if (argc == 1) {
        usage();
        exit(1);
    }

    // Parse command line parameters
    while ((c=getopt(argc, argv, "t:d:b:qhu:i:x:y:UIvVo:r:s:m:")) != -1)
        switch(c) {
            case 't':
                timeout=atoi(optarg);
                break;
            case 'd':
                strcpy(serialport, optarg);
                break;
            case 'b':
                baudrate=atoi(optarg);
                break;
            case 'q':
                command=CMD_GETSTATUS;
                break;
            case 'h':
                usage();
                exit(1);
                break;
            case 'u':
                if (optarg) {
                    command=CMD_SETVOLTAGE;
                    voltage=atof(optarg);
                } else {
                    command=CMD_GETVOLTAGE;
                }
                break;
            case 'i':
                if (optarg) {
                    command=CMD_SETCURRENT;
                    current=atof(optarg);
                } else {
                    command=CMD_GETCURRENT;
                }
                break;
            case 'x':
                command=CMD_SETOVERVOLTAGE;
                overvoltageprotection=atoi(optarg);
                break;
            case 'y':
                command=CMD_SETOVERCURRENT;
                overcurrentprotection=atoi(optarg);
                break;
            case 'U':
                command=CMD_GETOUTPUTVOLTAGE;
                break;
            case 'I':
                command=CMD_GETOUTPUTCURRENT;
                break;
            case 'v':
                debug=1;
                break;
            case 'V':
                command=CMD_GETVERSION;
                break;
            case 'o':
                command=CMD_SETOUTPUT;
                output_on=atoi(optarg);
                break;
            case 'r':
                command=CMD_RECALLMEMORY;
                bank=atoi(optarg);
                break;
            case 's':
                command=CMD_SAVEMEMORY;
                bank=atoi(optarg);
                break;
            case 'm':
                command=CMD_TRACKINGMODE;
                tracking_mode=atoi(optarg);
                if (tracking_mode < 0 || tracking_mode > 2) {
                    fprintf(stderr, "ERROR: unknown tracking mode\n");
                    exit (1);
                }
                break;
        }

    // Install signal handlers
    signal(SIGPIPE, SIG_IGN); // ignore broken pipes
    signal(SIGTERM, termination_handler);

    fd=open_serial_port(serialport, baudrate);
    if (fd < 0)
        return 1;

    // Send command
    char cmd[20];
    memset(cmd, 0, sizeof(cmd));
    switch(command) {
        case CMD_GETSTATUS:
            if (send_command(GETSTATUS) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SETVOLTAGE:
            sprintf(cmd, "%s%.2f", SETVOLTAGE, voltage);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_GETVOLTAGE:
            if (send_command(GETVOLTAGE) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SETCURRENT:
            sprintf(cmd, "%s%.3f", SETCURRENT, current);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_GETCURRENT:
            if (send_command(GETCURRENT) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SETOVERVOLTAGE:
            sprintf(cmd, "%s%d", SETOVERVOLTAGEPROTECTION, overvoltageprotection == 1 ? 1 : 0);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SETOVERCURRENT:
            sprintf(cmd, "%s%d", SETOVERCURRENTPROTECTION, overcurrentprotection == 1 ? 1 : 0);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_GETOUTPUTVOLTAGE:
            if (send_command(GETOUTPUTVOLTAGE) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_GETOUTPUTCURRENT:
            if (send_command(GETOUTPUTCURRENT) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_GETVERSION:
            if (send_command(GETVERSION) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SETOUTPUT:
            sprintf(cmd, "%s%d", SETOUTPUT, output_on == 1 ? 1 : 0);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_RECALLMEMORY:
            sprintf(cmd, "%s%d", RECALLMEM, bank);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_SAVEMEMORY:
            sprintf(cmd, "%s%d", SAVEMEM, bank);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        case CMD_TRACKINGMODE:
            sprintf(cmd, "%s%d", TRACKINGMODE, tracking_mode);
            if (send_command(cmd) < 0) {
                ret=1;
                goto ende;
            }
            break;
        default:
            goto ende;
            break;
    }

    // Parse response
    char response[1024];
    int len=0;
    //float outputvoltage = 0.0f;
    //float outputcurrent = 0.0f;

    switch (command) {
        case CMD_GETSTATUS:
            // STATUS?
            // Description?Returns the POWER SUPPLY status.
            // Contents 8 bits in the following format
            // Bit Item Description
            // 0 CH1 0=CC mode, 1=CV mode
            // 1 CH2 0=CC mode, 1=CV mode
            // 2, 3 Tracking 00=Independent, 01=Tracking series,11=Tracking parallel
            // 4 Beep 0=Off, 1=On
            // 5 Lock 0=Lock, 1=Unlock
            // 6 Output 0=Off, 1=On
            // 7 N/A N/A
            //
            // "Q" is all i get. Protocol documentation error?
            len=tryread(response, 1, timeout);
            if (len < 1) {
                fprintf(stderr, "no response\n");
                ret=1;
                goto ende;
            }

#define CH1_CVMODE    0x01
#define CH2_CVMODE    0x02
#define    TRACKING_SERIES    0x04
#define TRACKING_PARALLEL    0x0c
#define BEEP_ON    0x10
#define    UNLOCK    0x20
#define    OUTPUT_ON    0x40

            printf("Channel 1: %s mode\n", response[0] & CH1_CVMODE ? "CV": "CC");
            printf("Channel 2: %s mode\n", response[0] & CH2_CVMODE ? "CV": "CC");

            printf("Tracking:");
            if (response[0] & TRACKING_SERIES) {
                if (response[0] & TRACKING_PARALLEL) {
                    printf("parallel\n");
                } else {
                    printf("series\n");
                }
            } else {
                printf("Independent\n");
            }

            printf("Beep: %s\n", response[0] & BEEP_ON ? "on" : "off");
            printf("Unlock: %s\n", response[0] & UNLOCK ? "on" : "off");
            printf("Output: %s\n", response[0] & OUTPUT_ON ? "on" : "off");
            break;
        case CMD_GETVOLTAGE:
        case CMD_GETCURRENT:
        case CMD_GETOUTPUTVOLTAGE:
        case CMD_GETOUTPUTCURRENT:
            len=tryread(response, 5, timeout);
            if (len > 0) {
                printf("%s\n", response);
            }
        
            break;
        case CMD_GETVERSION:
            len=tryread(response, 19, timeout);
            if (len > 0) {
                printf("%s\n", response);
            }
        
            break;
    }

ende:
    if (fd)
        close(fd);

    return ret;
}
