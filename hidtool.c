/* Name: hidtool.c
 * Project: hid-data example
 * Author: Christian Starkjohann
 * Creation Date: 2008-04-11
 * Tabsize: 4
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 * This Revision: $Id: hidtool.c 723 2009-03-16 19:04:32Z cs $
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "hiddata.h"
#include "usbconfig.h"  /* for device VID, PID, vendor name and product name */

typedef struct _layer{
    unsigned int row0 : 5;
    unsigned int row1 : 5;
    unsigned int row2 : 5;
    unsigned int row3 : 5;
    unsigned int row4 : 5;
    unsigned int index :7;
}layer;

typedef struct _frame{
    layer layers[5];
}frame;




/*** Particles variables */
typedef struct _particle{
    int vx;
    int vy;
    int vz;
    int x;
    int y;
    int z;
    float ttl;
}particle;

#define MAXPARTICLES 20
particle particles[MAXPARTICLES];
int mat[6][6][6];


int emitX = 0;
int emitY = 0;
int emitZ = 0;

int centerX = 0;
int centerY = 0;
int centerZ = 0;

///////
float elapsed = 120;

/* ------------------------------------------------------------------------- */

static char *usbErrorMessage(int errCode)
{
    static char buffer[80];

    switch(errCode){
    case USBOPEN_ERR_ACCESS:      return "Access to device denied";
    case USBOPEN_ERR_NOTFOUND:    return "The specified device was not found";
    case USBOPEN_ERR_IO:          return "Communication error with device";
    default:
        sprintf(buffer, "Unknown USB error %d", errCode);
        return buffer;
    }
    return NULL;    /* not reached */
}

static usbDevice_t  *openDevice(void)
{
    usbDevice_t     *dev = NULL;
    unsigned char   rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
    char            vendorName[] = {USB_CFG_VENDOR_NAME, 0}, productName[] = {USB_CFG_DEVICE_NAME, 0};
    int             vid = rawVid[0] + 256 * rawVid[1];
    int             pid = rawPid[0] + 256 * rawPid[1];
    int             err;

    if((err = usbhidOpenDevice(&dev, vid, vendorName, pid, productName, 0)) != 0){
        fprintf(stderr, "error finding %s: %s\n", productName, usbErrorMessage(err));
        return NULL;
    }
    return dev;
}

/* ------------------------------------------------------------------------- */

static void hexdump(char *buffer, int len)
{
    int     i;
    FILE    *fp = stdout;

    for(i = 0; i < len; i++){
        if(i != 0){
            if(i % 16 == 0){
                fprintf(fp, "\n");
            }else{
                fprintf(fp, " ");
            }
        }
        fprintf(fp, "0x%02x", buffer[i] & 0xff);
    }
    if(i != 0)
        fprintf(fp, "\n");
}

static int  hexread(char *buffer, char *string, int buflen)
{
    char    *s;
    int     pos = 0;

    while((s = strtok(string, ", ")) != NULL && pos < buflen){
        string = NULL;
        buffer[pos++] = (char)strtol(s, NULL, 0);
    }
    return pos;
}

/* ------------------------------------------------------------------------- */

static void usage(char *myName)
{
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  %s read\n", myName);
    fprintf(stderr, "  %s write <listofbytes>\n", myName);
}

int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1)
        continue;
    return 1;
}

void initParticles(){
    
    for (int i=0;i<MAXPARTICLES;i++){
            particles[i].ttl = 0;
            particles[i].vx = 0;
            particles[i].vy = 1;
            particles[i].vz = 1;
        }
}

void processParticles(int k){
    //centerX = k % 5;
    //centerY = k % 5;

    /**** CREATE CUBE ******/
    mat[centerX][centerY][centerZ] = 1;
    mat[centerX+1][centerY][centerZ] = 1;
    mat[centerX][centerY+1][centerZ] = 1;
    mat[centerX+1][centerY+1][centerZ] = 1;
    mat[centerX][centerY][centerZ+1] = 1;
    mat[centerX+1][centerY][centerZ+1] = 1;
    mat[centerX][centerY+1][centerZ+1] = 1;
    mat[centerX+1][centerY+1][centerZ+1] = 1;


    for (int i=0;i<MAXPARTICLES;i++){
        particles[i].ttl = particles[i].ttl-1;

        if (particles[i].ttl > 0){
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].z += particles[i].vz;

            if (    (centerX+particles[i].x < 5) 
                    &&  (centerY+particles[i].y < 5) 
                    &&  (centerZ+particles[i].z < 5)
                    &&  (centerX+particles[i].x >= 0)
                    &&  (centerY+particles[i].y >= 0)
                    &&  (centerZ+particles[i].z >= 0)
                    )
                mat[centerX+particles[i].x][centerY+particles[i].y][centerZ+particles[i].z] = 1;
        } else {
            if (rand()%100 < 30){
                particles[i].ttl = 5;
                particles[i].x = emitX;
                particles[i].y = emitY;
                particles[i].z = emitZ;

                particles[i].vx = rand()%3-1;
                particles[i].vy = rand()%3-1;
                particles[i].vz = rand()%3-1;

                if (    (centerX+particles[i].x < 5) 
                    &&  (centerY+particles[i].y < 5) 
                    &&  (centerZ+particles[i].z < 5)
                    &&  (centerX+particles[i].x >= 0)
                    &&  (centerY+particles[i].y >= 0)
                    &&  (centerZ+particles[i].z >= 0)
                    )
                    mat[centerX+particles[i].x][centerY+particles[i].y][centerZ+particles[i].z] = 1;
            }
        }
    }
}

int main(int argc, char **argv)
{
    usbDevice_t *dev;
    char        buffer[21];    /* room for dummy report ID */
    int         err;

    if(argc < 2){
        usage(argv[0]);
        exit(1);
    }
    if((dev = openDevice()) == NULL)
        exit(1);
    if(strcasecmp(argv[1], "read") == 0){
        int len = sizeof(buffer);
        if((err = usbhidGetReport(dev, 0, buffer, &len)) != 0){
            fprintf(stderr, "error reading data: %s\n", usbErrorMessage(err));
        }else{
            hexdump(buffer + 1, sizeof(buffer) - 1);
        }
    }else if(strcasecmp(argv[1], "write") == 0){
        int i, pos;
        memset(buffer, 0, sizeof(buffer));
        for(pos = 1, i = 2; i < argc && pos < sizeof(buffer); i++){
            pos += hexread(buffer + pos, argv[i], sizeof(buffer) - pos);
        }
        if((err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0)   /* add a dummy report ID */
            fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
    }else if(strcasecmp(argv[1], "loop") == 0){
        

        frame ledframe;

        initParticles();
        

        int k = 0;
        while(++k){

            if (elapsed > 50){

                /****** CLEAN MATRIX ******/
                for(int x=0;x<5;x++)
                    for(int y=0;y<5;y++)
                        for(int z=0;z<5;z++)
                            mat[x][y][z] = 0;
                

                /******* DRAW STUFF *******/
                processParticles(k);
                        
                
                elapsed = 0;
            }

            
            memset(buffer,0,sizeof(buffer));
            memset(&ledframe,0,sizeof(ledframe));
            /// ----- LAYER
            for(int x=0; x< 5 ;x++){
                
            
                for(int j=0;j<5;j++){
                    ledframe.layers[j].index=(1<<j);
                }

                for (int y=0;y<5;y++){
                    for (int z=0;z<5;z++){

                        switch(y){
                            case 0:
                                ledframe.layers[x].row0 |= mat[y][x][z];
                                if (z != 4)
                                    ledframe.layers[x].row0 <<= 1;
                            break;
                            case 1:
                                ledframe.layers[x].row1 |= mat[y][x][z];
                                if (z != 4)
                                    ledframe.layers[x].row1 <<= 1;
                            break;
                            case 2:
                                ledframe.layers[x].row2 |= mat[y][x][z];
                                if (z != 4)
                                    ledframe.layers[x].row2 <<= 1;
                            break;
                            case 3:
                                ledframe.layers[x].row3 |= mat[y][x][z];
                                if (z != 4)
                                    ledframe.layers[x].row3 <<= 1;
                            break;
                            case 4:
                                ledframe.layers[x].row4 |= mat[y][x][z];
                                if (z != 4)
                                    ledframe.layers[x].row4 <<= 1;
                            break;
                        }
                    }
                    
                }
            }
            buffer[0]=0;
            memcpy(buffer+1,&ledframe,sizeof(ledframe));
            if((err = usbhidSetReport(dev, buffer, sizeof(buffer))) != 0)   // add a dummy report ID
                fprintf(stderr, "error writing data: %s\n", usbErrorMessage(err));
                
            msleep(80);
            elapsed += 80;
        }


    }else{
        usage(argv[0]);
        exit(1);
    }
    usbhidCloseDevice(dev);
    return 0;
}

/* ------------------------------------------------------------------------- */
