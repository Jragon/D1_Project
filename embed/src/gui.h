#ifndef GUI_H
#define GUI_H

#include "UGUI/ugui.h"
#include "lcd.h"

void gui_init();
uint16_t strwidth(char *str);
void console_put_ch(char ch);
void console_put_number(uint16_t number);
void console_put_float(float number);

UG_GUI gui;

#endif