#include "global.h"

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "app.h"
#include "sys.h"

// Defines, nur in app.c sichtbar
// ...

//globale Variablen nur in app.c sichtbar
// ...

//globale Strukturvariable, ueberall sichtbar
volatile struct App app;


void app_init (void)
{
  memset((void *)&app, 0, sizeof(app));
}


//--------------------------------------------------------

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



