#include "global.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#include "app.h"
#include "sys.h"
#include "bmp180.h"
// defines
// ...


// declarations and definations

volatile struct App app;


// functions

void app_init (void)
{
  memset((void *)&app, 0, sizeof(app));
  //timer0
  
  TCCR1B |= (1<<CS11); // Clock select = clk/8;
  //timer1
  TCCR0B |= (1<<CS21); //clock select
  //timer2
  TCCR2B |= (1<<CS22); //clock select
}

void app_incErrCnt (volatile uint8_t *cnt)
{
  if ((*cnt) < 255)
    (*cnt)++;
}

uint8_t app_isHexChar (uint8_t c)
{
  return (c>='0' && c<='9') || (c>='a' && c<='f');
}
//-------------------------------------------------------

void addTimeToTxBuffer(uint32_t timestamp)
{
  if(timestamp>1000000)
    snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%s%d%03d%03d=",(char *)app.uart.txBuffer,(int)(timestamp/1000000),(int)((timestamp/1000)%1000),(int)((timestamp)%1000));
  else if(timestamp>0)
    snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%s%d%03d=",(char *)app.uart.txBuffer,(int)(timestamp/1000),(int)((timestamp)%1000));
  else
    snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%s0=",(char *)app.uart.txBuffer);
}

//--------------------------------------------------------
void addSpeedToTxBuffer()
{
  char shaft[10] = "";
  
  if(app.timer.shaftRpm<1000)
    snprintf(shaft,sizeof(shaft),"%d",(int)app.timer.shaftRpm);
  else if(app.timer.shaftRpm<1000000)
    snprintf(shaft,sizeof(shaft),"%d%03d",(int)(app.timer.shaftRpm/1000),(int)(app.timer.shaftRpm%1000));
  else
    snprintf(shaft,sizeof(shaft),"%d%03d%03d",(int)(app.timer.shaftRpm/1000000),(int)((app.timer.shaftRpm/1000)%1000),(int)(app.timer.shaftRpm%1000));
  
  snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%c%s:",APP_SOT,shaft);
}
//--------------------------------------------------------
void addRpmsToTxBuffer()
{
  char moto[10] = "";
  char shaft[10] = "";
  
  if(app.timer.motorRpm<1000)
    snprintf(moto,sizeof(moto),"%d",(int)app.timer.motorRpm);
  else if(app.timer.motorRpm<1000000)
    snprintf(moto,sizeof(moto),"%d%03d",(int)(app.timer.motorRpm/1000),(int)(app.timer.motorRpm%1000));
  else
    snprintf(moto,sizeof(moto),"%d%03d%03d",(int)(app.timer.motorRpm/1000000),(int)((app.timer.motorRpm/1000)%1000),(int)(app.timer.motorRpm%1000));
  if(app.timer.shaftRpm<1000)
    snprintf(shaft,sizeof(shaft),"%d",(int)app.timer.shaftRpm);
  else if(app.timer.shaftRpm<1000000)
    snprintf(shaft,sizeof(shaft),"%d%03d",(int)(app.timer.shaftRpm/1000),(int)(app.timer.shaftRpm%1000));
  else
    snprintf(shaft,sizeof(shaft),"%d%03d%03d",(int)(app.timer.shaftRpm/1000000),(int)((app.timer.shaftRpm/1000)%1000),(int)(app.timer.shaftRpm%1000));
  
  snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%c%s:%s:",APP_SOT,shaft,moto);
}

void addConditionsToTxBuffer(uint8_t tempH,uint8_t tempL,uint8_t humidity,char pressure[])
{
  snprintf((char *)app.uart.txBuffer,sizeof(app.uart.txBuffer),"%c%d.%d:%d:%s=",APP_SOT,(int)tempH,(int)tempL,(int) humidity,  pressure);
}
//--------------------------------------------------------

void startMeas0()
{
  EICRA |= (1<<ISC01); //INT0 interrupt on rising edge(The rising edge on INT0 generates an interrupt
  TIMSK0 |= (1<<TOIE0); //timer overflow interrupt enabled
  EIMSK |= (1<<INT0); //External Interrupt Request 0 Enable
}

//--------------------------------------------------------

void startMeas1()
{
  TCCR1B &=~ (1<<ICES1);
  TIMSK1 |= (1<<ICIE1) | (1<<TOIE1);
} 

//--------------------------------------------------------
uint32_t getActualTime()
{
  return (TCNT2 + app.timer.ovfCounter2*256)*4;
}

//--------------------------------------------------------
uint16_t calculateCRC(uint16_t initial_crc, char *buffer,uint16_t length) //found on the internet
{
  uint16_t crc;
  uint16_t index = 0;
  crc = initial_crc;
  if (buffer != NULL) 
  {
    for (index = 0; index < length; index++) 
    {
      crc = (uint16_t)((uint16_t)(crc >> 8) | (uint16_t)(crc << 8));
      crc ^= buffer[index];
      crc ^= (uint16_t)(crc & 0xFF) >> 4;
      crc ^= (uint16_t)((uint16_t)(crc << 8) << 4);
      crc ^= (uint16_t)((uint16_t)((crc & 0xFF) << 4) << 1);
    }
  }
  
  return crc;
}
//--------------------------------------------------------
void getEcoSystem()
{
  uint16_t crc = 0;

  int8_t tempH;
  uint8_t tempL;
  uint8_t humidity;
  char pressure[10] = "";
  dht22(&tempH,&tempL,&humidity);
  getBmp180Values(&tempH, &tempL,(char *)pressure, sizeof(pressure));
  addConditionsToTxBuffer(tempH, tempL, humidity,pressure);

/*

  uint8_t i = 0;
  static uint8_t j = 0;
  app.uart.txBuffer[i++] = APP_SOT;

  if(j==0)
  {
    memcpy((char *)app.uart.txBuffer + strlen((char *)app.uart.txBuffer),"26.5:45:1005.5=",strlen("26.5:45:1005.5="));
    j++;
  }
  else
  {
    memcpy((char *)app.uart.txBuffer + strlen((char *)app.uart.txBuffer),"27.2:55:1007.4=",strlen("27.5:55:1007.4="));
    j=0;
  }
*/

  crc = calculateCRC(0xffff,(char *)app.uart.txBuffer,strlen((char *)app.uart.txBuffer));
  printf("%s%04x%c",(char *)app.uart.txBuffer, crc, APP_EOT);
  memset((char *)app.uart.txBuffer,0,strlen((char *)app.uart.txBuffer));
}

//--------------------------------------------------------
void startSpeedMeasure(uint32_t timestamp)
{
  uint16_t crc;
  app.timer.busy = 1;


/*
  static uint32_t simShaft = 20000;

  app.timer.shaftRpm = simShaft--;
  app.timer.busy = 0;
*/

  startMeas1();


  
  while(app.timer.busy>0){}
  addSpeedToTxBuffer();
  addTimeToTxBuffer(timestamp);
  crc = calculateCRC(0xffff,(char *)app.uart.txBuffer,strlen((char *)app.uart.txBuffer));
  printf("%s%04x%c",(char *)app.uart.txBuffer,crc,APP_EOT);
  
  memset((char *)app.uart.txBuffer,0,strlen((char *)app.uart.txBuffer));
}
//--------------------------------------------------------
void startRpmMeasure(uint32_t timestamp)
{
  uint16_t crc;
  app.timer.busy = 2;

/*
  static uint32_t simBike = 10000;
  static uint32_t simShaft = 3000;

  app.timer.shaftRpm = simShaft--;
  app.timer.motorRpm = simBike--;
  app.timer.busy = 0;
*/


  startMeas0();
  startMeas1();
  
  while(app.timer.busy>0){}
  addRpmsToTxBuffer();
  addTimeToTxBuffer(timestamp);
  crc = calculateCRC(0xffff,(char *)app.uart.txBuffer,strlen((char *)app.uart.txBuffer));
  printf("%s%04x%c",(char *)app.uart.txBuffer,crc,APP_EOT);
  
  memset((char *)app.uart.txBuffer,0,strlen((char *)app.uart.txBuffer));
}

//--------------------------------------------------------
void app_main (void)
{
  
  if (sys_clearEvent(APP_EVENT_FRAME_RECEIVED))
  {
    
    if(app.uart.rxBuffer[0] == 'm')
    {
      uint32_t timestamp = getActualTime();
      startRpmMeasure(timestamp);
    }
    else if(app.uart.rxBuffer[0] == 'r')
    {
      uint32_t timestamp = getActualTime();
      startSpeedMeasure(timestamp);
    }
    else if(app.uart.rxBuffer[0] == 's') 
    {
      //set timestamp to 0
      cli();
      TCNT2 = 0;
      app.timer.ovfCounter2 = 0;
      TIMSK2 |= (1<<TOIE2); //timer overflow interrupt enabled
      sei();
      startRpmMeasure(0);
    }
    else if(app.uart.rxBuffer[0] == 'g')
    {
      //set timestamp to 0
      cli();
      TCNT2 = 0;
      app.timer.ovfCounter2 = 0;
      TIMSK2 |= (1<<TOIE2); //timer overflow interrupt enabled
      sei();
      startSpeedMeasure(0);
    }
    else if(app.uart.rxBuffer[0] == 'e')
    {
      getEcoSystem();
    }
    
    cli();
    app.uart.framePending = 0;
    app.uart.recIndex = 0;
    sei();
  }
}

//ISR for timer0 - shaft rpm
ISR(INT0_vect)
{
  static uint8_t edgeDetection0 = 0;
  
  if(edgeDetection0 == 0)
  {
    EICRA |= (1<<ISC00) | (1<<ISC01); //INT0 interrupt on rising edge(The rising edge on INT0 generates an interrupt
    edgeDetection0++;
  }
  else if(edgeDetection0 == 1)
  {
    cli();
    TCNT0 = 0;
    app.timer.ovfCounter0 = 0;
    sei();
    edgeDetection0++;
  }
  else
  {
    cli();
    TIMSK0 &=~ (1<<TOIE0); //timer overflow interrupt enabled
    EIMSK &=~ (1<<INT0); //External Interrupt Request 0 Enable
    app.timer.motorRpm = TCNT0;
    app.timer.motorRpm = (TCNT0+(app.timer.ovfCounter0*256))/2;
    edgeDetection0 = 0;
    app.timer.busy--;
    EICRA &=~ (1<<ISC00); //INT0 interrupt on rising edge(The rising edge on INT0 generates an interrupt 
    sei();
  }
}

ISR(TIMER0_OVF_vect)
{
  app.timer.ovfCounter0++;
}

//ISR for timer1 - bikeRpm
ISR(TIMER1_OVF_vect)
{
  app.timer.ovfCounter1++;
}

ISR(TIMER1_CAPT_vect)
{
  static uint8_t edgeDetection1 = 0;
  
  if(edgeDetection1 == 0)
  {
    TCCR1B |= (1<<ICES1);
    edgeDetection1++;
  }
  else if(edgeDetection1 == 1)
  {
    cli();
    TCNT1 =0;
    app.timer.ovfCounter1 = 0;
    sei();
    edgeDetection1++;
  }
  else
  {
    cli();
    edgeDetection1 = 0;
    app.timer.shaftRpm = (ICR1+app.timer.ovfCounter1*65536)/2;
    TCCR1B &=~ (1<<ICES1);
    TIMSK1 &=~ (1<<ICIE1) | (1<<TOIE1);
    sei();
    app.timer.busy--;
  }
}

//ISR for timer2 - timestamp
ISR(TIMER2_OVF_vect)
{
  app.timer.ovfCounter2++;
}

//--------------------------------------------------------
void app_uart_isr (uint8_t b)
{
  //sys_log(__FILE__, __LINE__, sys_pid(), "uart_isr(0x%02x)", b);
  
  if (app.uart.framePending)
  {
    app_incErrCnt(&app.uart.errCnt_recFrameWhilePending);
    printf( "Error: Received Frame while old frame pending");
    return;
  }
  
  if (app.uart.recIndex == 0 && b != APP_SOT)
    return;

  if(app.uart.recIndex >=1)
  {
    app.uart.rxBuffer[app.uart.recIndex-1] = b;
  }
  
  if (b == APP_EOT) 
  {
    app.uart.framePending = 1;
    sys_setEvent(APP_EVENT_FRAME_RECEIVED);
    //sys_log(__FILE__, __LINE__, sys_pid(), "Frame with %d bytes received", app.uart.recIndex);
  }
  else if (app.uart.recIndex >= APP_UART_BUFFER_SIZE)
  {
    app.uart.recIndex = 0;
    app_incErrCnt(&app.uart.errCnt_recFrameTooLong);
    printf("Error: Received Frame too long");
  }
  
  app.uart.recIndex++;
  //sys_printf("%c", b);
}



