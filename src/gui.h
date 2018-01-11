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
void drawProperty(Property *prop);

UG_GUI gui;

#endif