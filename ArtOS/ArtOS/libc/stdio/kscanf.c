#include <sysdef.h>
#include <terminal.h>
#include <stdio.h>

//Core input function
char *mgets(char *s)
{
    char *ptr = s;
    int rd;
    
    if ((rd == '\n')) {
        *ptr = '\0';
        return s;
    }
    else {
        *ptr = rd;
    }
    
    /* now read the rest until NEWLINE/EOF */
    while (((rd = getChar()) != '\n')) {
        if (rd == '\b') {
            *ptr-- = ' ';
        } else {
            *ptr++ = rd;
        }
        monPut(rd);
    }
    
    *ptr = '\0';
    return s;
}

//Turns input to int
int read_int(uint64_t* var_addr) {
    
    uint64_t temp;
    char buff[5];
    
    mgets(buff);
    
    temp = atoi(buff);
    if(temp == -1)
        return temp;
    *var_addr = temp;
    
    return 0;
}

//Turns input to string
int read_str(uint64_t* var_addr) {
    
    mgets((char *) var_addr);
    
    return 0;
}

//The kscanf function
int kscanf(const char * format, ...) {
    
    va_list ap;
    va_start(ap,format);
    
    void * dest_addr = va_arg(ap,void *);
    
    char* str = format;
    int ret_val = 0;
    
    while(*str != '\0') {
        if((*str != '%') || (*(str+1) == '\0')) {
        }
        else
        {
            switch(*(str+1)) {
                case 'd':
                    ret_val = read_int((uint64_t *)dest_addr);
                    if(ret_val == -1) {
                        kprintf("scanf: error in input!!");
                        return ret_val;
                    }
                    break;
                    
                case 's':
                    ret_val = read_str((uint64_t *)dest_addr);
                    if(ret_val == -1) {
                        kprintf("scanf: error in input!!");
                        return ret_val;
                    }
                    break;
            }
            str+=2;
        }
    }
    
    return ret_val;
}
