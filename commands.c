#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <msp430.h>
#include <ctl.h>
#include <terminal.h>
#include <ARCbus.h>
#include <UCA1_uart.h>
#include <crc.h>
#include <commandLib.h>
#include "CDH.h"
#include <Error.h>
#include "CDH_errors.h"



int printCmd(char **argv,unsigned short argc){
  unsigned char buff[40],*ptr,id;
  unsigned char addr;
  unsigned short len;
  int i,j,k;
  //check number of arguments
  if(argc<2){
    printf("Error : too few arguments.\r\n");
    return 1;
  }
  //get address
  addr=getI2C_addr(argv[1],0,busAddrSym);
  if(addr==0xFF){
    return 1;
  }
  //setup packet 
  ptr=BUS_cmd_init(buff,6);
  //coppy strings into buffer for sending
  for(i=2,k=0;i<=argc && k<sizeof(buff);i++){
    j=0;
    while(argv[i][j]!=0){
      ptr[k++]=argv[i][j++];
    }
    ptr[k++]=' ';
  }
  //get length
  len=k;
  //TESTING: set pin high
  P8OUT|=BIT0;
  //send command
  BUS_cmd_tx(addr,buff,len,0,BUS_I2C_SEND_FOREGROUND);
  //TESTING: set pin low
  P8OUT&=~BIT0;
  return 0;
}

int tstCmd(char **argv,unsigned short argc){
  unsigned char buff[40],*ptr,*end;
  unsigned char addr;
  unsigned short len;
  int i,j,k;
  //check number of arguments
  if(argc<2){
    printf("Error : too few arguments.\r\n");
    return 1;
  }
  if(argc>2){
    printf("Error : too many arguments.\r\n");
    return 1;
  }
  //get address
  addr=getI2C_addr(argv[1],0,busAddrSym);
  len = atoi(argv[2]);
  /*if(len<0){
    printf("Error : bad length");
    return 2;
  }*/
  //setup packet 
  ptr=BUS_cmd_init(buff,7);
  //fill packet with dummy data
  for(i=0;i<len;i++){
    ptr[i]=i;
  }
  //TESTING: set pin high
  P8OUT|=BIT0;
  //send command
  BUS_cmd_tx(addr,buff,len,0,BUS_I2C_SEND_FOREGROUND);
  //TESTING: wait for transaction to fully complete
  while(UCB0STAT&UCBBUSY);
  //TESTING: set pin low
  P8OUT&=~BIT0;
  return 0;
}

int timeCmd(char **argv,unsigned short argc){
  printf("time ticker = %li\r\n",get_ticker_time());
  return 0;
}

int statCmd(char **argv,unsigned short argc){
  unsigned char buff[40],*ptr;
  int i;
  ticker time;
  //setup packet 
  ptr=BUS_cmd_init(buff,CMD_SUB_STAT);
  //get time
  time=get_ticker_time();
  //write time into the array
  ptr[0]=time>>24;
  ptr[1]=time>>16;
  ptr[2]=time>>8;
  ptr[3]=time;
  //send command
  BUS_cmd_tx(BUS_ADDR_GC,buff,4,0,BUS_I2C_SEND_FOREGROUND);
  return 0;
}

char *i2c_stat2str(unsigned char stat){
  switch(stat){
    case BUS_I2C_IDLE:
      return "I2C_IDLE";
    case BUS_I2C_TX:
      return "I2C_TX";
    case BUS_I2C_RX:
      return "I2C_RX";
   /* case BUS_I2C_TXRX:
      return "I2C_TXRX";
    case BUS_I2C_RXTX:
      return "I2C_RXTX";*/
    default:
      return "unknown state";
  }
}

int i2c_statCmd(char **argv,unsigned short argc){
  printf("I2C status = %s\r\n",i2c_stat2str(arcBus_stat.i2c_stat.mode));
  return 0;
}

int beaconCmd(char **argv,unsigned short argc){
  if(argc>1){
    printf("Error : Too many arguments\r\n");
    return -1;
  }
  if(argc==1){
    if(!strcmp(argv[1],"on")){
      beacon_on=1;
    }else if(!strcmp(argv[1],"off")){
      beacon_on=0;
    }else{
      printf("Error : Unknown argument \"%s\"\r\n",argv[1]);
      return -2;
    }
  }
  printf("Beacon : %s\r\n",beacon_on?"on":"off");
  return 0;
}

//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]\r\n\t""get a list of commands or help on a spesific command.",helpCmd},
                     CTL_COMMANDS,ARC_COMMANDS,REPLAY_ERROR_COMMAND,ARC_ASYNC_PROXY_COMMAND,
                     {"print"," addr str1 [[str2] ... ]\r\n\t""Send a string to addr.",printCmd},
                     {"tst"," addr len\r\n\t""Send test data to addr.",tstCmd},
                     {"stat","\r\n\t""Get status from all subsystems.",statCmd},
                     {"I2Cstat","\r\n\t""Print I2C status\r\n",i2c_statCmd},
                     {"beacon","[on|off]\r\n\t""Turn on/off status requests and beacon\r\n",beaconCmd},
                     //end of list
                     {NULL,NULL,NULL}};
