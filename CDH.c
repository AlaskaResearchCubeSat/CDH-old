#include <msp430.h>
#include <ctl_api.h>
#include <stdio.h>
#include <string.h>
#include <ARCbus.h>
#include "CDH.h"


CTL_EVENT_SET_t cmd_parse_evt;

short beacon_on=1;

STAT_PACKET system_stat;

//================[Time Tick interrupt]=========================
void task_tick(void) __ctl_interrupt[TIMERA1_VECTOR]{
  static int sec=0;
  switch(TAIV){
    case TAIV_TACCR1:
    break;
    case TAIV_TACCR2:
    break;
    case TAIV_TAIFG:
      sec+=2;
      if(sec==8){
        //collect status
        ctl_events_set_clear(&cmd_parse_evt,CMD_PARSE_GET_STAT,0);
        //clear status valid flags
        system_stat.flags=0;
      }
      if(sec>10){
        sec=0;
        //send status
        ctl_events_set_clear(&cmd_parse_evt,CMD_PARSE_SEND_STAT,0);
      }
    break;
  }
}

//handle subsystem specific commands
int SUB_parseCmd(unsigned char src,unsigned char cmd,unsigned char *dat,unsigned short len){
  int i;
  switch(cmd){
    case CMD_SUB_POWERUP:
      if(len==0){
        //TODO: keep track of which subsystems have powered up and such
      }else{
        return ERR_PK_LEN;
      }
    case CMD_SPI_CLEAR:
      //set event
      //TODO: keep track of who is using SPI
      ctl_events_set_clear(&cmd_parse_evt,CMD_PARSE_SPI_CLEAR,0);
      return RET_SUCCESS;
    //EPS status message
    case CMD_EPS_STAT:
      //check length
      if(len==sizeof(system_stat.EPS_stat)){
        //copy data into structure
        memcpy(system_stat.EPS_stat,dat,len);
        //set flag for EPS status
        system_stat.flags|=STAT_EPS_VALID;
        return RET_SUCCESS;
      }else{
        return ERR_PK_LEN;
      }
    //LEDL status
    case CMD_LEDL_STAT:
      //check length
      if(len==sizeof(system_stat.LEDL_stat)){
        //copy data into structure
        memcpy(system_stat.LEDL_stat,dat,len);
        //set flag for LEDL status
        system_stat.flags|=STAT_LEDL_VALID;
        return RET_SUCCESS;
      }else{
        return ERR_PK_LEN;
      }
    case CMD_ACDS_STAT:
      //check length
      if(len==sizeof(system_stat.ACDS_stat)){
        //copy data into structure
        memcpy(system_stat.ACDS_stat,dat,len);
        //set flag for ACDS status
        system_stat.flags|=STAT_ACDS_VALID;
        return RET_SUCCESS;
      }else{
        return ERR_PK_LEN;
      }
    case CMD_COMM_STAT:
      //check length
      if(len==sizeof(system_stat.COMM_stat)){
        //copy data into structure
        memcpy(system_stat.COMM_stat,dat,len);
        //set flag for COMM status
        system_stat.flags|=STAT_COMM_VALID;
        return RET_SUCCESS;
      }else{
        return ERR_PK_LEN;
      }
    case CMD_IMG_STAT:
      //check length
      if(len==sizeof(system_stat.IMG_stat)){
        //copy data into structure
        memcpy(system_stat.IMG_stat,dat,len);
        //set flag for IMG status
        system_stat.flags|=STAT_IMG_VALID;
        return RET_SUCCESS;
      }else{
        return ERR_PK_LEN;
      }
  }
  //Return Error
  return ERR_UNKNOWN_CMD;
}

void cmd_parse(void *p) __toplevel{
  unsigned int e;
  unsigned char buff[40],*ptr;
  ticker time;
  int resp,i;
  //init event
  ctl_events_init(&cmd_parse_evt,0);
  for(;;){
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&cmd_parse_evt,CMD_PARSE_ALL,CTL_TIMEOUT_NONE,0);
    if(e&CMD_PARSE_SPI_CLEAR){
      puts("SPI bus Free\r");
      //TODO: keep track of who is using SPI
    }
    if(e&CMD_PARSE_GET_STAT && beacon_on){
      //clear status flags from old status packet
      system_stat.flags=0;
      //print message
      printf("Requesting status\r\n");
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
    }
    if(e&CMD_PARSE_SEND_STAT && beacon_on){
      printf("Sending status\r\n");
      //get time for beacon
      time=get_ticker_time();
      //write time into status
      system_stat.time0=time>>24;
      system_stat.time1=time>>16;
      system_stat.time2=time>>8;
      system_stat.time3=time;
      //send SPI data
      resp=BUS_SPI_txrx(BUS_ADDR_COMM,(unsigned char*)&system_stat,NULL,sizeof(STAT_PACKET));
      ptr=(unsigned char*)&system_stat;
      for(i=0;i<sizeof(STAT_PACKET);i++){
        printf("0x%02X ",ptr[i]);
        if(i%15==14){
          printf("\r\n");
        }
      }
      printf("\r\n");
      if(system_stat.flags&STAT_ALL_VALID==STAT_ALL_VALID){
        printf("All subsystems reported status\r\n");
      }else{
        if(!(system_stat.flags&STAT_EPS_VALID)){
          printf("No Status Info for EPS\r\n");
        }
        if(!(system_stat.flags&STAT_LEDL_VALID)){
          printf("No Status Info for LEDL\r\n");
        }
        if(!(system_stat.flags&STAT_ACDS_VALID)){
          printf("No Status Info for ACDS\r\n");
        }
        if(!(system_stat.flags&STAT_COMM_VALID)){
          printf("No Status Info for COMM\r\n");
        }
        if(!(system_stat.flags&STAT_IMG_VALID)){
          printf("No Status Info for IMG\r\n");
        }
      }
    }
  }
}

//parse events from the bus for the subsystem
void sub_events(void *p) __toplevel{
  unsigned int e,len;
  int i;
  unsigned char buf[10],*ptr;
  for(;;){
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SUB_events,SUB_EV_ALL,CTL_TIMEOUT_NONE,0);
    if(e&SUB_EV_PWR_OFF){
      //print message
      puts("System Powering Down\r");
    }
    if(e&SUB_EV_PWR_ON){
      //print message
      puts("System Powering Up\r");
    }
    if(e&SUB_EV_TIME_CHECK){
      printf("time ticker = %li\r\n",get_ticker_time());
    }
    if(e&SUB_EV_SPI_DAT){
      puts("SPI data recived:\r");
      //get length
      len=arcBus_stat.spi_stat.len;
      //print out data
      for(i=0;i<len;i++){
        //printf("0x%02X ",rx[i]);
        printf("%03i ",arcBus_stat.spi_stat.rx[i]);
      }
      printf("\r\n");
      //free buffer
      BUS_free_buffer_from_event();
    }
    if(e&SUB_EV_SPI_ERR_CRC){
      puts("SPI bad CRC\r");
    }
  }
}
