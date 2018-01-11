#ifndef GUI_H
#define GUI_H

#include "UGUI/ugui.h"
#include "lcd.h"


typedef struct {
  uint16_t x, y;
  char *label;
  uint8_t val;
} Property;

void gui_init();
uint16_t strwidth(char *str);
void draw_property(Property *prop);
void draw_plabel(Property *prop);
void draw_pval(Property *prop);
void console_put_ch(char ch);
void console_put_number(uint16_t number);

UG_GUI gui;

#endif