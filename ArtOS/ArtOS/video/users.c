#include <users.h>
#include <string.h>
#include <terminal.h>
#include <stdio.h>

//Adds new user to the shell
int addNewUser(char * username, char * password, char * firstName, char * lastName, char * country) {
    struct users newUser;
//    kprintf("%s", newUser.username);
    memset(newUser.username, '\0', sizeof(newUser.username));
    memset(newUser.password, '\0', sizeof(newUser.password));
    memset(newUser.firstName, '\0', sizeof(newUser.firstName));
    memset(newUser.lastName, '\0', sizeof(newUser.lastName));
    memset(newUser.country, '\0', sizeof(newUser.country));
    
    strcpy(newUser.username, username);
    strcpy(newUser.password, password);
    strcpy(newUser.firstName, firstName);
    strcpy(newUser.lastName, lastName);
    strcpy(newUser.country, country);
    user_array[user_array_index] = newUser;
    user_array_index += 1;
    
    return 0;
}

//Adds all available users
int tempUserStart() {
    addNewUser("root", "toor", "root", "root", "earth");
    addNewUser("chpetrou", "CyEsGr", "Christos", "Petrou", "Cyprus");
    addNewUser("AirPit", "Eks123Pet", "Petros", "Christodoulou", "Greece");
    
    return 0;
}

//Searches for a user
int searchForUser(char * username) {
    for (int i = 0; i < user_array_index; i++) {
//        kprintf("\nusps: %d\n", strlen(username));
//         kprintf("\nusr: %s", user_array[0].username);
        if (strcmp(user_array[i].username, username) == 0) {
            return i;
        }
    }
    
    return -1;
}

//compares users details for authentication
int compareUserPass(char * username, char * password) {
    int isUserFound = searchForUser(username);
//     kprintf("usr: %s , pas: %s", username, password);
    if (isUserFound != -1) {
        if (strcmp(user_array[isUserFound].username, username) == 0 && strcmp(user_array[isUserFound].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}
