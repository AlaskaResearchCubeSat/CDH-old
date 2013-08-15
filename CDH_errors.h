#ifndef __CDH_ERRORS_H
  #define __CDH_ERRORS_H
  
  //error sources for BUS test program
  enum{CDH_ERR_SRC_CMD=ERR_SRC_SUBSYSTEM};
    
  //command errors
  enum{CMD_ERR_RESET};
    
  //subsystem errors
  enum{SUB_ERR_SPI_CRC};
#endif
  