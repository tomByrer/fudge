#ifndef ARCH_SCREEN_H
#define ARCH_SCREEN_H

#define SCREEN_ADDRESS          0xB8000
#define SCREEN_CHARACTER_WIDTH  80
#define SCREEN_CHARACTER_HEIGHT 25
#define SCREEN_COLOR_BLACK      0x00
#define SCREEN_COLOR_BLUE       0x01
#define SCREEN_COLOR_BLUE2      0x09
#define SCREEN_COLOR_BROWN      0x06
#define SCREEN_COLOR_BROWN2     0x0e
#define SCREEN_COLOR_CYAN       0x03
#define SCREEN_COLOR_CYAN2      0x0b
#define SCREEN_COLOR_GRAY       0x08
#define SCREEN_COLOR_GRAY2      0x07
#define SCREEN_COLOR_GREEN      0x02
#define SCREEN_COLOR_GREEN2     0x0a
#define SCREEN_COLOR_MAGENTA    0x05
#define SCREEN_COLOR_MAGENTA2   0x0d
#define SCREEN_COLOR_RED        0x04
#define SCREEN_COLOR_RED2       0x0c
#define SCREEN_COLOR_WHITE      0x0f

extern void screen_clear();
extern void screen_cursor_move();
extern void screen_scroll();
extern void screen_init();

#endif

