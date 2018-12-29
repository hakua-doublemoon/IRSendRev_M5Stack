/*
 * License and Overview -> readme.md
 */

#include "IRlib.h"

void dtaInit(char* cmd_str, unsigned char* send_p)
{
  /* 
   * LEN = 23
   * START_H: 251  START_L: 122
   * DATA_H: 33  DATA_L: 90
   */
  //send_p[BIT_LEN]        = 23;      // all data that needs to be sent
  send_p[BIT_START_H]    = 251;     // the logic high duration of "Start"
  send_p[BIT_START_L]    =122 + 3;    // the logic low duration of "Start" (with adjust)
  send_p[BIT_DATA_H]     = 33 - 2;    // the logic "long" duration in the communication (with adjust)
  send_p[BIT_DATA_L]     = 90 + 3;    // the logic "short" duration in the communication (with adjust)
  if (strncmp(cmd_str, RC_CMD_HT_24, sizeof(RC_CMD_HT_24)) == 0) {
    /*
     * LEN = 23
     * DATA_LEN = 18
     * DATA: 0x28  0xC6  0x0 0x8 0x8 0x7F  0xD0  0x82  0x81  0x20  0x60  0x0 0x0 0x0 0x0 0x58  0xC0  0xE8  
     * DATA: 40  198 0 8         8 127 208 130         129 32  96  0         0 0 0 88          192 232 
     */
    send_p[BIT_LEN]        = 23;      // all data that needs to be sent
    send_p[BIT_DATA_LEN]   = 18;      // Number of data which will sent. If the number is other, you should increase or reduce send_p[BIT_DATA+x].
    send_p[BIT_DATA+0]     = 40;      // data that will sent
    send_p[BIT_DATA+1]     = 198;
    send_p[BIT_DATA+2]     = 0;
    send_p[BIT_DATA+3]     = 8;
    send_p[BIT_DATA+4]     = 8;       //
    send_p[BIT_DATA+5]     = 127;
    send_p[BIT_DATA+6]     = 208;
    send_p[BIT_DATA+7]     = 130;
    send_p[BIT_DATA+8]     = 129;     //
    send_p[BIT_DATA+9]     = 32;
    send_p[BIT_DATA+10]     = 96;
    send_p[BIT_DATA+11]     = 0;
    send_p[BIT_DATA+12]     = 0;      //
    send_p[BIT_DATA+13]     = 0;
    send_p[BIT_DATA+14]     = 0;
    send_p[BIT_DATA+15]     = 88;
    send_p[BIT_DATA+16]     = 192;    //
    send_p[BIT_DATA+17]     = 232;
    send_p[BIT_DATA+18]     = 0;
    send_p[BIT_DATA+19]     = 0; //88;
  } else if (strncmp(cmd_str, RC_CMD_OFF, sizeof(RC_CMD_OFF)) == 0) {
    /*
     * LEN = 12
     * DATA_LEN = 7
     * DATA: 0x28 0xC6  0x0 0x8 0x8 0x40  0xBF  
     * DATA: 40  198 0 8 8 64  191
     */
    send_p[BIT_LEN]        = 12;
    send_p[BIT_DATA_LEN]   = 7;
    send_p[BIT_DATA+0]     = 40;      // 
    send_p[BIT_DATA+1]     = 198;
    send_p[BIT_DATA+2]     = 0;
    send_p[BIT_DATA+3]     = 8;
    send_p[BIT_DATA+4]     = 8;       //
    send_p[BIT_DATA+5]     = 64;
    send_p[BIT_DATA+6]     = 191;
    send_p[BIT_DATA+7]     = 0;
    send_p[BIT_DATA+8]     = 0;       //
    send_p[BIT_DATA+9]     = 0;
    send_p[BIT_DATA+10]    = 0;
    send_p[BIT_DATA+11]    = 0;
  } else {
    memset(send_p + BIT_DATA_LEN, 0, 20);
  }
}
