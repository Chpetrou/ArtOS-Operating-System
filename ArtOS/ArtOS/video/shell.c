#include <shell.h>
#include <rtc.h>
#include <users.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <terminal.h>

struct systemLang {
    char greek[150];
    char spanish[150];
    char english[150];
};

//Adds command to the shell
int add_shell_command(char * cmname, int (*function) (), struct systemLang * description) {
    struct shell_command shcmd;
    
    memset(shcmd.cmname, '\0', sizeof(shcmd.cmname));
    
    strcpy(shcmd.cmname, cmname);
    shcmd.function = function;
    shcmd.description = description;
    
    cmdArray[cmdArrayIndex] = shcmd;
    cmdArrayIndex++;
    
    return 0;
}

//Adds language description to the shell
struct systemLang add_language_desc(char * greek, char * spanish, char * english) {
    struct systemLang cmd;
    
    memset(cmd.greek, '\0', sizeof(cmd.greek));
    memset(cmd.spanish, '\0', sizeof(cmd.spanish));
    memset(cmd.english, '\0', sizeof(cmd.english));
    
    strcpy(cmd.greek, greek);
    strcpy(cmd.spanish, spanish);
    strcpy(cmd.english, english);
    
    return cmd;
}

//Finds a command in the shell
int find_command(char * command_name) {
    for (int i = 0; i < cmdArrayIndex; i++) {
        if (strcmp(command_name, "") == 0 || strcmp(command_name, " ") == 0) return -2;
//        kprintf("\ncmdnm: %s\n", cmdArray[i].cmname);
        if (strcmp(cmdArray[i].cmname, command_name) == 0)
            return i;
    }
    return -1;
}

//Executes a command in the shell
int executeCommand(struct shell_command cmd, char * shellLoc, char * loggedUsername) {
    monInit();
    create_kernel_task(NULL, cmd.function, cmd.cmname, NORMAL_PRIO);
    monInit();
//    kprintf("\nartos:%s %s$ ", shellLoc, loggedUsername);
    return 0;
}

//Shell available commands implementations
int get_time_of_day() {
    date_time();
    return 0;
}

int get_calc() {
    calc_main();
    return 0;
}

int play_snake() {
    snakeMain();
    return 0;
}

int shut_down() {
    //    break;
    monInit();
    switch (language) {
        case GREEK:
            kprintf("Mporeite twra na kleisete th syskeyh sas ");
            break;
        case SPANISH:
            kprintf("Ahora puede apagar su dispositivo de manera segura ");
            break;
        case ENGLISH:
            kprintf("You can now safely turn off your device ");
            break;
        default:
            break;
    }
    waitTicks(100);
    kprintf(".");
    waitTicks(100);
    kprintf(".");
    waitTicks(100);
    kprintf(".");
//    isUserFinished = 1;
    return 0;
}

int help() {
    for (int i = 0; i < cmdArrayIndex; i++) {
        switch (language) {
            case GREEK:
                kprintf("\n%s: %s\n", cmdArray[i].cmname, cmdArray[i].description->greek);
                break;
            case SPANISH:
                kprintf("\n%s: %s\n", cmdArray[i].cmname, cmdArray[i].description->spanish);
                break;
            case ENGLISH:
                kprintf("\n%s: %s\n", cmdArray[i].cmname, cmdArray[i].description->english);
                break;
            default:
                break;
        }
    }
    return 0;
}

//Adds all stock commands
int addAllShellCommands() {
    struct systemLang  cmdn1 = add_language_desc("Dixnei wra kai imerominia", "Muestra hora y fecha", "Shows time and date");
    struct systemLang  cmdn2 = add_language_desc("Anoigei tin ypologistikh", "Abre la calculadora", "Opens the calculator");
    struct systemLang  cmdn3 = add_language_desc("I ekdosi tou ArtOS gia to fidaki", "Edición ArtOS del juego Snake", "ArtOS edition of Snake game");
    struct systemLang  cmdn4 = add_language_desc("Kleinei to ArtOS", "Apaga ArtOS", "Shuts down ArtOS");
    struct systemLang  cmdn5 = add_language_desc("Deixnei tis diathesimes entoles kai perigrafes", "Muestra comandos disponibles y descripciones", "Shows available commands and descriptions");
    struct systemLang * cmd1 = &cmdn1;
    struct systemLang * cmd2 = &cmdn2;
    struct systemLang * cmd3 = &cmdn3;
    struct systemLang * cmd4 = &cmdn4;
    struct systemLang * cmd5 = &cmdn5;
    
    add_shell_command("timedate", get_time_of_day, cmd1);
    add_shell_command("open calculator", get_calc, cmd2);
    add_shell_command("open fidaki", play_snake, cmd3);
    add_shell_command("shutdown", shut_down, cmd4);
    add_shell_command("help", help, cmd5);
    return 0;
}

//Initializes the shell
int shell_main() {
//    char shellUName[15];
    cmdArrayIndex = 0;
    addAllShellCommands();
    char shellLoc[] = "\\";
    int isUserFinished = 0;
    int languageChosen = 0;
    char username[15];
    char password[15];
    user_array_index = 0;
    tempUserStart();
    int passTries = 0;
    switch (language) {
        case GREEK:
            kprintf("Kalwsorisate sto ArtOS\n");
            kprintf("Parakalw dialekste ti glwssa sas: \n");
            kprintf("1. Ellinika\n2. Ispanika\n3. Agglika\n");
            break;
        case SPANISH:
            kprintf("Bienvenido a ArtOS\n");
            kprintf("Por favor elija su idioma: \n");
            kprintf("1. Griego\n2. Espanol\n3. Ingles\n");
            break;
        case ENGLISH:
            kprintf("Welcome to ArtOS\n");
            kprintf("Please choose your language: \n");
            kprintf("1. Greek\n2. Spanish\n3. English\n");
            break;
        default:
            break;
    }
    kscanf("%d", &languageChosen);
    switch (languageChosen) {
        case 1:
            language = GREEK;
            break;
        case 2:
            language = SPANISH;
            break;
        case 3:
            language = ENGLISH;
            break;
        default:
            break;
    }
    
    kprintf("%d", languageChosen);
    
    int userNumber = 0;
    do {
        if (passTries > 0) {
            switch (language) {
                case GREEK:
                    kprintf("\nSignwmh, lathos kwdikos. Parakalw prospathhste ksana.\n");
                    break;
                case SPANISH:
                    kprintf("\nLo siento, contrasena incorrecta. Intentalo de nuevo.\n");
                    break;
                case ENGLISH:
                    kprintf("\nSorry, wrong password. Please try again.\n");
                    break;
                default:
                    break;
            }
        }
        switch (language) {
            case GREEK:
                kprintf("\nSyndetheite\n");
                kprintf("Parakalw grapste to onoma xrhsth sas:\n");
                break;
            case SPANISH:
                kprintf("\nIniciar sesion\n");
                kprintf("Por favor, ingrese su nombre de usuario:\n");
                break;
            case ENGLISH:
                kprintf("\nLog In\n");
                kprintf("Please enter your username:\n");
                break;
            default:
                break;
        }
        kscanf("%s", &username);
//        userNumber = realBufferSize;
//            kprintf("un: %s", username);
    
//            kprintf("%s\n", username);
        switch (language) {
            case GREEK:
                kprintf("\nParakalw grapste ton kwdiko sas:\n");
                break;
            case SPANISH:
                kprintf("\nPor favor, introduzca su contrasena:\n");
                break;
            case ENGLISH:
                kprintf("\nPlease enter your password:\n");
                break;
            default:
                break;
        }
        kscanf("%s", &password);
//            kprintf("ps: %s", password);
//            kprintf("%s\n", password);
        passTries += 1;
    } while (compareUserPass(username, password) == 0);
    char loggedUsername[15];
    strcpy (loggedUsername, username);
    switch (language) {
        case GREEK:
            kprintf("\nKalwsorisate %s !\n", loggedUsername);
            break;
        case SPANISH:
            kprintf("\nBienvenido %s !\n", loggedUsername);
            break;
        case ENGLISH:
            kprintf("\nWelcome %s !\n", loggedUsername);
            break;
        default:
            break;
    }
    while (isUserFinished == 0) {
        char buffCom[15];
        kprintf("\nartos:%s %s$ ", shellLoc, loggedUsername);
        kscanf("%s", &buffCom);
//        kprintf("\ntest: %s\n", ((struct shell_command*) vector_get(&vec, 0))->cmname);
//        break;
        
        if (find_command(buffCom) == -1) {
            switch (language) {
                case GREEK:
                    kprintf("\nSygnwmh ayth h entolh den yparxei !");
                    break;
                case SPANISH:
                    kprintf("\nLo siento, no existe tal comando !");
                    break;
                case ENGLISH:
                    kprintf("\nSorry no such command exists !");
                    break;
                default:
                    break;
            }
        }
        else if (find_command(buffCom) == -2) continue;
        else executeCommand(cmdArray[find_command(buffCom)], shellLoc, loggedUsername);
        
//        kprintf("\n%s", buffCom);
//        if (stringCompare(buffCom, "shutdown", 8)) {
//        break;
//        }
    }
//    shut_down();
    return 0;
}
