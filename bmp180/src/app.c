#include "global.h"

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>

#include "app.h"
#include "sys.h"
#include "i2c_master.h"
#define BMP_ADRESS 0xEE
#define BMP_PREAS_OSS3 0xF4
// Defines, nur in app.c sichtbar
// ...

//globale Variablen nur in app.c sichtbar
// ...

//globale Strukturvariable, ueberall sichtbar
volatile struct App app;

void app_init (void)
{
  memset((void *)&app, 0, sizeof(app));
  //init twi frequenzy
}


//--------------------------------------------------------
void startTwi(uint8_t add)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); //Send START condition
  while (!(TWCR &(1<<TWINT))){} // Wait for TWINT Flag set. This indicates that the START condition has been transmitted
  if ((TWSR & 0xF8) !=TW_START) //Check value of TWI Status Register. Mask prescaler bits. If status different from START go to ERROR
    printf("ERROR");
  TWDR = add;
  TWCR = (1<<TWINT) | (1<<TWEN); //send add
  while (!(TWCR &(1<<TWINT))); //
  if ((TWSR & 0xF8) != TW_MT_SLA_ACK);
  printf("ERROR");
}
//--------------------------------------------------------
void writeTwi(uint8_t data)
{
  TWDR = data;
  TWCR = (1<<TWINT) | (1<<TWEN);
  
  while (!(TWCR &(1<<TWINT)));
  if ((TWSR & 0xF8) != TW_MT_DATA_ACK);
    printf("ERROR");
}
//
void stopTwi()
{
  TWCR =(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}
//
void app_main (void)
{
}

//--------------------------------------------------------

void app_task_1ms (void) {}
void app_task_2ms (void) {}
void app_task_4ms (void) {}
void app_task_8ms (void) {}
void app_task_16ms (void) {}
void app_task_32ms (void) {}
void app_task_64ms (void) {}
void app_task_128ms (void) {}



