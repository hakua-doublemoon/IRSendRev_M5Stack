/*
 * License and Overview -> readme.md
 */

#include "IRlib.h"

//#include <M5Stack.h>
//#include <Wire.h>     //The DHT12 uses I2C comunication.

#define __RECV__
#define __SEND__

//unsigned bool initialize_flag = false;

volatile irparams_t irparams;
decode_t results;
unsigned char dta[40];
unsigned char dtaSend[40];
unsigned volatile int* pSt = NULL;
//unsigned volatile char pSt[RAWBUF];
volatile unsigned int timer_ic = 0;

volatile unsigned int sendmax   = 0;
volatile unsigned int sendindex = 0;
volatile int pwdcnt = 0;

void onTimer_read()
{
    uint8_t irdata = 1;
    if (irparams.recvpin == 0) {
        return;
    }

    irdata = digitalRead(irparams.recvpin);
    ir_read();
}

void onTimer_send()
{
    //if (irparams.recvpin == 0) {
    //    return;
    //}
    if (sendmax > sendindex) {
        if (sendindex & 1) {
            ir_write(false);
        }
        else {
            // 0, 2, ....
            ir_write(true);
        }
        if (pwdcnt > 0) {
            pwdcnt--;
            if (pwdcnt == 0) {
               timer_ic &= 0xFE;
               sendindex++;
               // 2nd or later
               if (sendmax != sendindex) {
                   //if (sendindex%8 == 7) {
                   //   Serial.println();
                   //}
                   pwdcnt = pSt[sendindex];
                   //Serial.print(pwdcnt); Serial.print(":"); Serial.print(sendindex); Serial.print(" ");
               } else {
                  //Serial.println("Send - final");
                  ir_write(false);
               }
            }
        } else {
            // 1st
            //Serial.println("Send - 1st");
            pwdcnt = pSt[sendindex];
        }
    } else {
        ir_write(false);
    }
}

void IRAM_ATTR onTimer()
{
    timer_ic += 1;
#ifdef __RECV__
    onTimer_read();
#endif

#ifdef __SEND__
    onTimer_send();
#endif
}

void setup()
{
    M5.begin();
    //Wire.begin();
    irparams.recvpin = pinRecv;

#ifdef __RECV__
    irparams.rawbuf = (unsigned char*)malloc((RAWBUF)*sizeof(unsigned char));
    for (int i = 0; i < 256; i+=1) {
        irparams.rawbuf[i] = 0;
    }
    pinMode(irparams.recvpin, INPUT);
    irparams.rcvstate = STATE_IDLE;
#endif

#ifdef __SEND__
    pSt = (unsigned int *)malloc((RAWBUF)*sizeof(unsigned int));
    pinMode(pinSend, OUTPUT);
    ir_write(false);
#endif

    delay(1000);

    // 64kHz
    hw_timer_t *timer = timerBegin(0, 21, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 50, true);
    timerAlarmEnable(timer);

    Serial.println("Setup - Finish");
}

int ir_decode(decode_t *results)
{
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;
  if (irparams.rcvstate != STATE_STOP) {
      //Serial.print("iligal state: ");
      //Serial.println(irparams.rcvstate);
      return ERR;
  }
  // Throw away and start over
  ir_clear();
  return 1;
}

int isDta()
{
    if(ir_decode(&results))
    {
        int count       = results.rawlen;
        if(count < 20 || (count -4)%8 != 0)
        {         
            ir_clear();        // Receive the next value
            Serial.print("iligal count: ");
            Serial.println(count);
            return 0;
        }
        int count_data  = (count-4) / 16;
        return (unsigned char)(count_data+6);
    }
    else 
    {
        return 0;
    }
}

void loop_read()
{
    if(isDta())                  // get IR data
    {
        ir_recv(dta);               // receive data to dta

        Serial.println("+------------------------------------------------------+");
        Serial.print("LEN = ");
        Serial.println(dta[BIT_LEN]);
        Serial.print("START_H: ");
        Serial.print(dta[BIT_START_H]);
        Serial.print("\tSTART_L: ");
        Serial.println(dta[BIT_START_L]);
        
        Serial.print("DATA_H: ");
        Serial.print(dta[BIT_DATA_H]);
        Serial.print("\tDATA_L: ");
        Serial.println(dta[BIT_DATA_L]);
        
        Serial.print("\r\nDATA_LEN = ");
        Serial.println(dta[BIT_DATA_LEN]);
        
        Serial.print("DATA: ");
        for(int i=0; i<dta[BIT_DATA_LEN]; i++)
        {
            Serial.print("0x");
            Serial.print(dta[i+BIT_DATA], HEX);
            Serial.print("\t");
            M5.Lcd.print(dta[i+BIT_DATA], HEX);
            M5.Lcd.print(" ");
        }
        Serial.println();
        M5.Lcd.println();
    
        Serial.print("DATA: ");
        for(int i=0; i<dta[BIT_DATA_LEN]; i++)
        {
            Serial.print(dta[i+BIT_DATA], DEC);
            Serial.print("\t");
        }
        Serial.println();
        Serial.println("+------------------------------------------------------+\r\n\r\n");
    }
}

bool first_flag = true;

void loop()
{
    delay(500);
    //irparams.recvpin = 0;
#ifdef __RECV__
    loop_read();
#endif
#ifdef __SEND__
    if(M5.BtnA.read()) {
        Serial.println(" #=> A");
        M5.Lcd.println("Send A");
        dtaInit(RC_CMD_OFF, dtaSend);
        ir_send(dtaSend, 38);
        delay(500);
    } else if (M5.BtnB.read()) {
        Serial.println(" #=> B");
        M5.Lcd.println("Send B");
        dtaInit(RC_CMD_HT_24, dtaSend);
        ir_send(dtaSend, 38);
        delay(500);       
    } else {
        if (first_flag) { // for test
            first_flag = false;
            //dtaInit(RC_CMD_OFF, dtaSend);
            //ir_send(dtaSend, 38);
        }
        // nothing
    }
    M5.update();
#endif
    irparams.recvpin = pinRecv;
}
