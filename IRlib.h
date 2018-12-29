/*
 * License and Overview -> readme.md
 */

#include <M5Stack.h>
#include <driver/ledc.h>

// information for the interrupt handler
//#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF (400) // Length of raw duration buffer
//#define RAWBUF 252 // Length of raw duration buffer
typedef struct {
    uint8_t recvpin;              // pin for IR data from detector
    uint8_t rcvstate;             // state machine
    unsigned int timer;           // state timer, counts 50uS ticks. -> maybe 26
    uint32_t rawlen;               // counter of entries in rawbuf
    //unsigned int rawbuf[RAWBUF];  // raw data
    unsigned char* rawbuf;  // raw data
} irparams_t;

// Results returned from the decoder
typedef struct {
    //public:
    unsigned char *rawbuf; // Raw intervals in .5 us ticks
    int rawlen;            // Number of records in rawbuf.
} decode_t;

#define pinRecv (22)             // 22 = MPU9250/GROVE SCL ;  
#define pinSend (2)

// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

#define MARK  0
#define SPACE 1

#define ERR 0
#define DECODED 1

// len, start_H, start_L, nshort, nlong, data_len, data[data_len]....
#define D_LEN       0
#define D_STARTH    1
#define D_STARTL    2
#define D_SHORT     3
#define D_LONG      4
#define D_DATALEN   5
#define D_DATA      6

#define BIT_LEN         D_LEN //0
#define BIT_START_H     D_STARTH //1
#define BIT_START_L     D_STARTL //2
#define BIT_DATA_H      D_SHORT //3
#define BIT_DATA_L      D_LONG  //4
#define BIT_DATA_LEN    D_DATALEN //5
#define BIT_DATA        D_DATA //6

#define TIMER_ENABLE_PWM(P)      () //(ledcWrite(P,255)) //(analogWrite(P,1))
#define TIMER_DISABLE_PWM(P)     ()  //(analogWrite(P,0))
//#define TIMER_DISABLE_PWM    (TCCR1A &= ~(_BV(COM1A1)))

#define RC_CMD_HT_24   ("RC_CMD_HT_24")
#define RC_CMD_OFF     ("RC_CMD_OFF")

extern volatile unsigned int* pSt;
//extern volatile unsigned char pSt[RAWBUF];
extern volatile unsigned int sendmax;
extern volatile unsigned int sendindex;
extern volatile unsigned int timer_ic;

extern void ir_read();
extern unsigned char ir_recv(unsigned char *revData);
extern void ir_clear();
extern void ir_send(unsigned char *idata, unsigned char ifreq);
extern void mark(int time);
extern void space(int time);
extern void ir_write(bool isMark);
extern void dtaInit(char* cmd_str, unsigned char* send_p);
