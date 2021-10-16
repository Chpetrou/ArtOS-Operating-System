#include <terminal.h>
#include <kb.h>

//Moves the cursor of the terminal
static void move_cursor()
{
    uint16_t cursorLocation = cursorY * 80 + cursorX;
    outportb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outportb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outportb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outportb(0x3D5, cursorLocation);      // Send the low cursor byte.
}

//Move the cursor to a scpecific location
void setCursorToSpecificLocation(COORD coordinates)
{
    cursorY = coordinates.Y;
    cursorX = coordinates.X;
}

//Scrolls the terminal when needed
void scroll()
{
    if(cursorY >= 25)
    {
        int i;
        for (i = 0*80; i < 24*80; i++)
        {
            TBuffer[i] = TBuffer[i+80];
        }

        for (i = 24*80; i < 25*80; i++)
        {
            TBuffer[i] = vgaEntry(' ', TColor);
        }
        cursorY = 24;
    }
}

//Initializes the terminal
void monInit(void) {
	TRow = 0;
	TColumn = 0;
	TColor = vgaEntryColor(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	TBuffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			TBuffer[index] = vgaEntry(' ', TColor);
		}
	}
    
    cursorX = 0;
    cursorY = 0;
    move_cursor();
}

//Changes the color of text
void monSetColor(uint8_t color) {
	TColor = color;
}

//Functions for writing to the terminal
void monPutEntryAt(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
    if (c == '\n') {
        if (TRow == VGA_HEIGHT-1) scroll();
        else TRow++;
        //c = '\0';
        TColumn = 0;
    }
	else TBuffer[index] = vgaEntry(c, color);
}
 
void monPutchar(char c) {
	monPutEntryAt(c, TColor, TColumn, TRow);
	if (++TColumn == VGA_WIDTH) {
		TColumn = 0;
		if (++TRow == VGA_HEIGHT) {
            scroll();
			TRow = 24;
        }
	}
}
 
void monPut(char c)
{
    // Handle a backspace
    if (c == 0x08)
    {
        if (cursorX == 0) {
            cursorX = 80;
            cursorY--;
        }
        TColumn = cursorX-1;
        TRow = cursorY;
        monPutchar(' ');
        cursorX--;
    }

    // Handle a tab
    else if (c == 0x09)
    {
        cursorX = (cursorX+8) & ~(8-1);
    }
    
    // Handle carriage return
    else if (c == '\r')
    {
        cursorX = 0;
    }

    // Handle newline
    else if (c == '\n')
    {
        cursorX = 0;
        cursorY++;
    }
    // Handle other printable character.
    else if(c >= ' ')
    {
        if (capsLock == 1 && (c >= 0x61 && c <= 0x7A)) {
            TColumn = cursorX;
            TRow = cursorY;
            monPutchar(c-32);
            cursorX++;
        }
        else {
            TColumn = cursorX;
            TRow = cursorY;
            monPutchar(c);
            cursorX++;
        }
    }

    if (cursorX >= 80)
    {
        cursorX = 0;
        cursorY ++;
    }

    scroll();
    move_cursor();

}

void monWrite(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
//		monPutchar(data[i]);
        monPut(data[i]);
}
 
void monWString(const char* data) {
	monWrite(data, strlen(data));
}


void monWDec(uint32_t n)
{

    if (n == 0)
    {
        monPut('0');
        return;
    }

    int32_t acc = n;
    char c[32];
    int i = 0;
    while (acc > 0)
    {
        c[i] = '0' + acc%10;
        acc /= 10;
        i++;
    }
    c[i] = 0;

    char c2[32];
    c2[i--] = 0;
    int j = 0;
    while(i >= 0)
    {
        c2[i--] = c[j++];
    }
    monWString(c2);

}

void monWHex(uint32_t n)
{
    int32_t tmp;

    monWString("0x");

    char noZeroes = 1;

    int i;
    for (i = 28; i > 0; i -= 4)
    {
        tmp = (n >> i) & 0xF;
        if (tmp == 0 && noZeroes != 0)
        {
            continue;
        }
    
        if (tmp >= 0xA)
        {
            noZeroes = 0;
            monPut (tmp-0xA+'a' );
        }
        else
        {
            noZeroes = 0;
            monPut( tmp+'0' );
        }
    }
  
    tmp = n & 0xF;
    if (tmp >= 0xA)
    {
        monPut (tmp-0xA+'a');
    }
    else
    {
        monPut (tmp+'0');
    }

}

void clearBuffer(char* buffer) {
    memset(buffer,0,strlen(buffer));
}

//Arrow keys pointers

void upKeyn(){
    if (cursorY == 0) cursorY = 24;
    else cursorY--;
}
void downKeyn(){
    if (cursorY == 24) cursorY = 0;
    else cursorY++;
}
void rightKeyn(){
    if (cursorX == 79) cursorX = 0;
    if (shiftKey == 1) cursorX = (cursorX+8) & ~(8-1);
    else cursorX++;
}
void leftKeyn(){
    if (cursorX == 0) cursorX = 79;
    if (shiftKey == 1) cursorX = (cursorX-8) & ~(8-1);
    else cursorX--;
}

//Input from keyboard
void getKbCode() {
    if (scancode & 0x80)
    {
        //Key released
        switch (scancode) {
            case 0xaa :
                shiftKey = 0;
                break;
            case 0xb6 :
                shiftKey = 0;
                break;
            case 0xb8 :
                //                monPutchar('Alt');
                break;
            case 0x9d :
                //                monPutchar('Ctrl');
                break;
            case 0xdb :
                //                monPutchar('CmdL');
                break;
            case 0xdc :
                //                monPutchar('CmdR');
                break;
        }
    }
    else
    {
        //A key was just pressed.
        switch (scancode) {
            case 0x2a :
                shiftKey = 1; //shiftL
                break;
            case 0x36 : //shiftR
                shiftKey = 1;
                break;
            case 0x3a : //capsLock
                if (capsLock == 0) capsLock = 1;
                else if (capsLock == 1) capsLock = 0;
                break;
            case 0x48 : //upKey
                (*upKey)();
//                upKeyPressed();
//                kprintf("gia");
//                isNextCharReady = 1;
                break;
            case 0x50 : //downKey
                (*downKey)();
//                downKeyPressed();
//                isNextCharReady = 1;
                break;
            case 0x4b : //leftKey
                (*leftKey)();
//                leftKeyPressed();
//                isNextCharReady = 1;
                break;
            case 0x4d : //rightKey
                (*rightKey)();
//                rightKeyPressed();
//                isNextCharReady = 1;
                break;
            case 0x38 :
                //                monPutchar('Alt');
                break;
            case 0x1d :
                //                monPutchar('Ctrl');
                break;
            case 0x5b :
                //                monPutchar('CmdL');
                break;
            case 0x5c :
                //                monPutchar('CmdR');
                break;
            default:
                isNextCharReady = 1;
//                monPut('c');
                break;
        }
//        monPut('\0');
        scroll();
        move_cursor();
    }
//    task_finish_current ();
}

//Getchar function
char getChar() {
    isButtonPressed = 0;
    isNextCharReady = 0;
//    isNotCharMove
//    while (isNextCharReady == 0 && isNotCharMove == 0);
    while (isNextCharReady == 0){
        if (isButtonPressed == 1) {
            getKbCode();
//            monPut('\0');
            isButtonPressed = 0;
        }
    }
//    nextButtonAvailable = 0;
    if (shiftKey == 1) return kbdusS[scancode];
    else return kbdus[scancode];
}

//Useful buffer functions
void printBuffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) {
        if (buffer[i] != '\0') monPut(buffer[i]);
    }
}

int bufferCount(char *buffer, int size) {
    int counter = 0;
    for (int i = 0; i < size; i++) {
        if (buffer[i] != '\0') counter++;
    }
    return counter;
}

int isString(char *buffer, int size) {
    int flag = 0;
    for (int i = 0; i < size; i++) {
        if (buffer[i] == '\0') flag = 1;
    }
    return flag;
}

int addToBuffer(char* buffer, int size, char character) {
    if (bufferNum < size){
        buffer[bufferNum] = character;
        bufferNum++;
        buffer[size-1] = '\0';
        return 1;
    }
    return 0;
}

int removeFromBuffer(char* buffer, int size) {
    if (bufferNum < size){
        buffer[bufferNum] = '\0';
        bufferNum--;
        return 1;
    }
    else return 0;
}

int stringCompare(char* one, char* two, int oneSize) {
    int flag = 1;
    for (int i = 0; i < oneSize; i++) {
        if (one[i] != two[i]) {
            flag = 0;
        }
    }
    return flag;
}

int bufferCopy(char* dest, char* src, int destSize, int srcSize) {
    if (destSize < srcSize) {
        return 0;
    }
    for (int i = 0; i < destSize; i++) {
        dest[i] = src[i];
    }
    return 1;
}

