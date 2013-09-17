#include <msp430.h>
#include <ctl_api.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <ARCbus.h>
#include <UCA1_uart.h>
#include <terminal.h>
#include <Error.h>
#include "CDH.h"

CTL_TASK_t tasks[3];

//stacks for tasks
unsigned stack1[1+256+1];          
unsigned stack2[1+512+1];
unsigned stack3[1+64+1];   



//set printf and friends to send chars out UCA1 uart
int __putchar(int c){
  //don't print if async connection is open
  if(!async_isOpen()){
    return UCA1_TxChar(c);
  }else{
    return EOF;
  }
}

int main(void){
  const TERM_SPEC uart_term={"ARC CDH board test program",UCA1_Getc};
  //DO this first
  ARC_setup(); 
  
  //setup subsystem specific peripherals
  
  //setup UCA1 UART
  UCA1_init_UART();
  
  //TESTING: set log level to report everything by default
  set_error_level(0);
    
  //setup P7 for LED's
  P7OUT=0x00;
  P7DIR=0xFF;
    
  P7OUT|=BIT7|BIT6;

  //setup P8 for output
  P8OUT=0x00;
  P8DIR=0xFF;
  P8SEL=0x00;

  //setup bus interface
  initARCbus(BUS_ADDR_CDH);

  //enable 2 sec interrupt
  TACTL|=TAIE;

  //initialize stacks
  memset(stack1,0xcd,sizeof(stack1));  // write known values into the stack
  stack1[0]=stack1[sizeof(stack1)/sizeof(stack1[0])-1]=0xfeed; // put marker values at the words before/after the stack
  
  memset(stack2,0xcd,sizeof(stack2));  // write known values into the stack
  stack2[0]=stack2[sizeof(stack2)/sizeof(stack2[0])-1]=0xfeed; // put marker values at the words before/after the stack
    
  memset(stack3,0xcd,sizeof(stack3));  // write known values into the stack
  stack3[0]=stack3[sizeof(stack3)/sizeof(stack3[0])-1]=0xfeed; // put marker values at the words before/after the stack

  //create tasks
  ctl_task_run(&tasks[0],BUS_PRI_LOW,cmd_parse,NULL,"cmd_parse",sizeof(stack1)/sizeof(stack1[0])-2,stack1+1,0);
  ctl_task_run(&tasks[1],BUS_PRI_NORMAL,terminal,(void*)&uart_term,"terminal",sizeof(stack2)/sizeof(stack2[0])-2,stack2+1,0);
  ctl_task_run(&tasks[2],BUS_PRI_HIGH,sub_events,NULL,"sub_events",sizeof(stack3)/sizeof(stack3[0])-2,stack3+1,0);
  
  mainLoop();

}

