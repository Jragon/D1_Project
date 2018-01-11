#include "gui.h"

void _lcd_pset(UG_S16 x, UG_S16 y, UG_COLOR c) { draw_pixel(x, y, c); }

void gui_init() {
  init_lcd();
  UG_Init(&gui, _lcd_pset, 240, 320);
  UG_FontSelect(&FONT_6X8);

  // console setup
  UG_ConsoleSetArea(0, 120, 240, 200);
  UG_ConsoleSetForecolor(C_GREEN_YELLOW);
  UG_ConsoleSetBackcolor(C_BLACK);
  UG_ConsolePutString("Hey!");
}

void drawProperty(Property *prop) {
  char valstr[15];
  snprintf(valstr, 15, "%d", prop->val);

  UG_PutString(prop->x, prop->y, prop->label);
  UG_PutString(prop->x + strwidth(prop->label), prop->y, valstr);
}

uint16_t strwidth(char *str) {
  char ch, chwidth;
  uint16_t width = 0;
  while (*str != 0) {
    ch = *str;
    chwidth = gui.font.widths ? gui.font.widths[ch - gui.font.start_char]
                              : gui.font.char_width;
    width += chwidth + gui.char_h_space;
    str++;
  }

  return width;
}