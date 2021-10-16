#include <stdio.h>
#include <shell.h>
#include <terminal.h>

//Main for calculator program in 3 languages
int calc_main() {
    switch (language) {
        case GREEK:
            kprintf("Ypologistiki\n----------");
            break;
        case SPANISH:
            kprintf("Calculadora\n----------");
            break;
        case ENGLISH:
            kprintf("Calculator\n----------");
            break;
        default:
            break;
    }
    kprintf("\n1. +");
    kprintf("\n2. -");
    kprintf("\n3. *");
    kprintf("\n4. /\n");
    
    char operator;
    int firstNumber,secondNumber;
//    int calc_choice;
//    double first_number;
//    double second_number;
//    double answer;

    kscanf("%d", &operator);
    
    switch (language) {
        case GREEK:
            kprintf("\nParakalw grapste ton proto arithmo: ");
            break;
        case SPANISH:
            kprintf("\nPor favor escribe el primer numero: ");
            break;
        case ENGLISH:
            kprintf("\nPlease write the first number: ");
            break;
        default:
            break;
    }
    kscanf("%d", &firstNumber);
    
    switch (language) {
        case GREEK:
            kprintf("\nParakalw grapste ton deytero arithmo: ");
            break;
        case SPANISH:
            kprintf("\nPor favor escribe el segundo numero: ");
            break;
        case ENGLISH:
            kprintf("\nPlease write the second number: ");
            break;
        default:
            break;
    }
    kscanf("%d", &secondNumber);
    
    switch(operator)
    {
        case 1:
            kprintf("%d + %d = %d",firstNumber, secondNumber, firstNumber + secondNumber);
            break;
            
        case 2:
            kprintf("%d - %d = %d",firstNumber, secondNumber, firstNumber - secondNumber);
            break;
            
        case 3:
            kprintf("%d * %d = %d",firstNumber, secondNumber, firstNumber * secondNumber);
            break;
            
        case 4:
            kprintf("%d / %d = %d",firstNumber, secondNumber, firstNumber / secondNumber);
            break;
            
            // operator doesn't match any case constant (+, -, *, /)
        default:
            kprintf("Error! operator is not correct");
    }
    
    return 0;
}
