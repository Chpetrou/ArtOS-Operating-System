#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <sysdef.h>
#include <string.h>
#include <lowlevel/io.h>
#include <stdio.h>

//Languages
#define GREEK 1
#define SPANISH 2
#define ENGLISH 3

typedef struct COORD {
    int X;
    int Y;
} COORD, *PCOORD;

//Colors
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

int language;

int artos_start();

char buffer[30];
int bufferNum;
volatile int isScanEnabled;

//char kbchar;
unsigned char scancode;
volatile int isNextCharReady;
volatile int isButtonPressed;

//Arrow key pointers
void upKeyn();
void downKeyn();
void rightKeyn();
void leftKeyn();

void (*upKey)();
void (*downKey)();
void (*leftKey)();
void (*rightKey)();

void upKeyPressed();
void downKeyPressed();
void leftKeyPressed();
void rightKeyPressed();

int nextButtonAvailable;

int snakeMain();

//Terminal functions
void setCursorToSpecificLocation(COORD coordinates);

void printBuffer(char *buffer, int size);
void clearBuffer(char* cbuffer);
int bufferCopy(char* dest, char* src, int destSize, int srcSize);
int addToBuffer(char* buffer, int size, char character);
int bufferCount(char *buffer, int size);
int isString(char *buffer, int size);
int stringCompare(char* one, char* two, int oneSize);
char getChar();

uint8_t cursorX;
uint8_t cursorY;
int shiftKey, capsLock, altKey, ctrlKey, cmdKey;
 
static inline uint8_t vgaEntryColor(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vgaEntry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t TRow;
size_t TColumn;
uint8_t TColor;
uint16_t* TBuffer;
 
void monInit(void);
 
void monSetColor(uint8_t color);
 
void monPutEntryAt(char c, uint8_t color, size_t x, size_t y);
 
void monPutchar(char c);
 
void monWrite(const char* data, size_t size);
 
void monWString(const char* data);

void monPut(char c);

void monWDec(uint32_t n);

void monWHex(uint32_t n);

char getChar();

#endif // TERMINAL_H
