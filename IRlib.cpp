/*
 * License and Overview -> readme.md
 */

#include "IRlib.h"

extern irparams_t irparams;
extern unsigned char dta[20];
extern decode_t results;

void ir_write(bool isMark)
{
    if (isMark) {
        if (timer_ic&1) mark(0); else space(0);
    } else {
        space(0);
    }
}

void ir_read()
{
  //TIMER_RESET;

  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF) {
    // Buffer overflow
    if (irparams.rcvstate != STATE_STOP) {
        Serial.print(irparams.rawlen); Serial.println(" : STOP2");
    }
    irparams.rcvstate = STATE_STOP;
  } else {
    irparams.rawbuf[irparams.rawlen] = 0;
  }

  switch(irparams.rcvstate) {
  case STATE_IDLE: // In the middle of a gap
    if (irdata == MARK) {
      //if (irparams.timer < GAP_TICKS) {
      if (irparams.timer < 100) {
        // Not big enough to be a gap.
        //Serial.println("Not big enough to be a gap.");
        irparams.timer = 0;
      }
      else {
        // gap just ended, record duration and start recording transmission
        Serial.print(irparams.timer); Serial.println(" : IDLE -> MARK");
        irparams.rawlen = 0;
        if (irparams.timer >= 0xFF) {
            irparams.timer = 0xFF;
        }
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
    }
    break;
  case STATE_MARK: // timing MARK
    if (irdata == SPACE) {   // MARK ended, record time
      //Serial.print(irparams.timer); Serial.print(":"); Serial.print(irparams.rawlen); Serial.println("/M ");
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_SPACE;
    }
    break;
  case STATE_SPACE: // timing SPACE
    if (irdata == MARK) { // SPACE just ended, record it
      //Serial.print(irparams.timer); Serial.print(" ");
      //Serial.print(irparams.timer); Serial.print(":"); Serial.print(irparams.rawlen); Serial.println("/S ");
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_MARK;
    }
    else { // SPACE
      //if (irparams.timer > GAP_TICKS) {
      if (irparams.timer > 500) {
        // big SPACE, indicates gap between codes
        // Mark current code as ready for processing
        // Switch to STOP
        // Don't reset timer; keep counting space width
        Serial.print(" Too big enough to be a gap. : "); Serial.println(irparams.timer);
        irparams.rcvstate = STATE_STOP;
      } 
    }
    break;
  case STATE_STOP: // waiting, measuring gap
    if (irdata == MARK) { // reset gap timer
        //Serial.println(" : STOP");
        irparams.timer = 0;
    }
    break;
  }
}

unsigned char ir_recv(unsigned char *revData)
{
    int count       = results.rawlen;
    int nshort      = 0;
    int nlong       = 0;
    int count_data  = 0;

    count_data = (count-4)/16;
    //Serial.print(count); Serial.println(" @ count");
    //Serial.print(count_data); Serial.println(" @ count_data");

    for(int i = 0; i<10; i++)           // count nshort
    {
        //Serial.print(results.rawbuf[i]); Serial.print("  ");
        //M5.Lcd.print(results.rawbuf[i]); M5.Lcd.print(" ");
        nshort += results.rawbuf[3+2*i];
    }
    nshort /= 10;
    //Serial.println();
    //M5.Lcd.print();

    int i = 0;
    int j = 0;
    while(1)        // count nlong
    {
        if(results.rawbuf[4+2*i] > (2*nshort))
        {
            nlong += results.rawbuf[4+2*i];
            j++;
        }
        i++;
        if(j==10)break;
        if((4+2*i)>(count-10))break;
    }
    nlong /= j;

    int doubleshort = 2*nshort;
    for(i = 0; i < count_data; i++)
    {
        revData[i+D_DATA] = 0x00;
        for(j = 0; j<8; j++)
        {
            //if(results.rawbuf[4 + 16*i + j*2] > doubleshort) // 1
            if(irparams.rawbuf[4 + 16*i + j*2] > doubleshort) // 1
            {
                revData[i+D_DATA] |= 0x01<< (7-j);
                //Serial.print(irparams.rawbuf[4 + 16*i + j*2]); Serial.print(":"); Serial.print(4 + 16*i + j*2); Serial.print("  ");
                //Serial.print(irparams.rawbuf[4 + 16*i + j*2]); Serial.print("  ");
            }
            else
            {
                revData[i+D_DATA] &= ~(0x01<<(7-j));
                //Serial.print(irparams.rawbuf[4 + 16*i + j*2]); Serial.print(":"); Serial.print(4 + 16*i + j*2); Serial.print("  ");
                //Serial.print(irparams.rawbuf[4 + 16*i + j*2]); Serial.print("  ");
                //Serial.print("0 ");
            }
        }
        Serial.println();
    }
    revData[D_LEN]      = count_data+5;
    revData[D_STARTH]   = results.rawbuf[1];
    revData[D_STARTL]   = results.rawbuf[2];
    revData[D_SHORT]    = nshort;
    revData[D_LONG]     = nlong;
    revData[D_DATALEN]  = count_data;

#if __DEBUG
    Serial.print("\r\n*************************************************************\r\n");
    Serial.print("len\t = ");Serial.println(revData[D_LEN]);
    Serial.print("start_h\t = ");Serial.println(revData[D_STARTH]);
    Serial.print("start_l\t = ");Serial.println(revData[D_STARTL]);
    Serial.print("short\t = ");Serial.println(revData[D_SHORT]);
    Serial.print("long\t = ");Serial.println(revData[D_LONG]);
    Serial.print("data_len = ");Serial.println(revData[D_DATALEN]);
    for(int i = 0; i<revData[D_DATALEN]; i++)
    {
        Serial.print(revData[D_DATA+i]);Serial.print("\t");
    }
    Serial.print("\r\n*************************************************************\r\n");
#endif

    ir_clear(); // Receive the next value
    return revData[D_LEN]+1;
}

void ir_clear()
{
    if (irparams.rawlen != 0) {
        Serial.println(" !clear! ");
    }
#if 0
    Serial.println("----IN----");
    for(int i = 0; i < irparams.rawlen; i++)
    {
        Serial.print(irparams.rawbuf[i]); Serial.print("\t");
        //M5.Lcd.print(irparams.rawbuf[i]); M5.Lcd.print(" ");
        if (i%8 == 0) {
            Serial.println();
            //M5.Lcd.println();
        }
    }
#endif
    irparams.rcvstate = STATE_IDLE;
    irparams.rawlen = 0;
}

void mark(int time) {
  digitalWrite(pinSend, HIGH);
}

/* Leave pin off for time (given in microseconds) */
void space(int time) {
  digitalWrite(pinSend, LOW);
}

//void ir_sendRaw(unsigned int buf[], int len, int hz)
void ir_sendRaw(int len, int hz)
{
  //enableIROut(hz);
#if 0
  for (int i = 0; i < len; i++) {
    if (i & 1) {
      space(pSt[i]);
    } 
    else {
      mark(pSt[i]);
    }
  }
  space(0); // Just to be sure
#else
  sendindex = 0;
  sendmax   = len;
#endif
}


void ir_send(unsigned char *idata, unsigned char ifreq)
{
    int len = idata[0];
    unsigned char start_high    = idata[D_STARTH];
    unsigned char start_low     = idata[D_STARTL];
    unsigned char nshort        = idata[D_SHORT];
    unsigned char nlong         = idata[D_LONG];
    unsigned char datalen       = idata[D_DATALEN];

    //unsigned int *pSt = (unsigned int *)malloc((4+datalen*16)*sizeof(unsigned int));

    if(NULL == pSt)
    {
#if __DEBUG
        Serial.println("not enough place!!\r\n");
#endif
        exit(1);
    }

#if 0
    Serial.println("begin to send ir:\r\n");
    Serial.print("ifreq = ");Serial.println(ifreq);
    Serial.print("len = ");Serial.println(len);
    Serial.print("start_high = ");Serial.println(start_high);
    Serial.print("start_low = ");Serial.println(start_low);
    Serial.print("nshort = ");Serial.println(nshort);
    Serial.print("nlong = ");Serial.println(nlong);
    Serial.print("datalen = ");Serial.println(datalen);
#endif
    unsigned int interval = 1; //1000000/(ifreq*1000);

    pSt[0] = start_high*interval;
    pSt[1] = start_low*interval;

    for(int i = 0; i<datalen; i++)
    {
        for(int j=0; j<8; j++)
        {
            if(idata[D_DATA+i] & 0x01<<(7-j))
            {
                pSt[16*i + 2*j + 2] = nshort*interval;
                pSt[16*i + 2*j + 3] = nlong*interval;
            }
            else
            {
                pSt[16*i + 2*j+2]   = nshort*interval;
                pSt[16*i + 2*j+3]   = nshort*interval;
            }
        }
    }

    pSt[2+datalen*16]   = nshort*interval;
    pSt[2+datalen*16+1] = nshort*interval;

#if 0
    irparams.recvpin = 0;
    Serial.println("----OUT----");
    for(int i = 0; i<4+datalen*16; i++)
    {
        Serial.print(pSt[i]); Serial.print("\t");
        if (i%8 == 7) {
            Serial.println();
        }
    }
    Serial.println();
    delay(100);
    irparams.recvpin = pinRecv;
#endif

    ir_sendRaw(4+datalen*16, ifreq);

    //free(pSt);
    
}
