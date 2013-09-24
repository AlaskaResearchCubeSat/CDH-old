#ifndef __CDH_H
#define __CDH_H

extern CTL_EVENT_SET_t cmd_parse_evt;

void cmd_parse(void *p);

//parse events from the bus for the subsystem
void sub_events(void *p);

//structure for status data from subsystems
typedef struct{
  //Flags to determine if subsystem data is valid
  unsigned char flags;
  //time bytes
  unsigned char time0,time1,time2,time3;
  //data from each subsystem
  //TODO: figure out how much data each subsystem has
  char EPS_stat[30],LEDL_stat[16],ACDS_stat[26],COMM_stat[14],IMG_stat[12];
}STAT_PACKET;

//flags for STAT_PACKET
enum{STAT_EPS_VALID=1<<0,STAT_LEDL_VALID=1<<1,STAT_ACDS_VALID=1<<2,STAT_COMM_VALID=1<<3,STAT_IMG_VALID=1<<4};
  
//mask to see if all statuses are valid
#define STAT_ALL_VALID  (STAT_EPS_VALID|STAT_LEDL_VALID|STAT_ACDS_VALID|STAT_COMM_VALID|STAT_IMG_VALID)

//flags for cmd_parse_evt
enum{CMD_PARSE_SPI_CLEAR=1<<2,CMD_PARSE_GET_STAT=1<<4,CMD_PARSE_SEND_STAT=1<<5};

#define CMD_PARSE_ALL (CMD_PARSE_SPI_CLEAR|CMD_PARSE_GET_STAT|CMD_PARSE_SEND_STAT)

extern short beacon_on;

#endif
