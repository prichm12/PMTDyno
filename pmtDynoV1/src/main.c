//***********************************************************************
// Vorlage PRT Level 2
// ----------------------------------------------------------------------
// Beinhaltet:
// UART-Support, Timer, Task-System
// ----------------------------------------------------------------------
// Author: SX 
// Version: 
// Vorlage Version: 19.2.2015 (SX)
//***********************************************************************

#include "global.h"

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "sys.h"
#include "app.h"

// Defines, nur in main.c sichtbar
// ...

// globale Variable, nur in main.c sichtbar
// ...

// Konstante, die im Flash abgelegt werden
const char MAIN_WELCOME[] = "\n\rProgramm ?? ";
const char MAIN_DATE[] = __DATE__;
const char MAIN_TIME[] = __TIME__;


int main (void)
{
  sys_init();
  app_init();
  printf("%s %s %s\n\r", MAIN_WELCOME, MAIN_DATE, MAIN_TIME);
  sys_newline();

  // Interrupt-System jetzt einschalten
  sei();

  while (1)
  {
    sys_main();
    app_main();
  }
  return 0;
}
