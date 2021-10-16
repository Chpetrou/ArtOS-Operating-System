#ifndef __USERS_H_
#define __USERS_H_

//Users struct
struct users {
    char username[15];
    char password[15];
    char firstName[10];
    char lastName[10];
    char country[20];
};

struct users user_array[5];
int user_array_index;

//User management functions
int addNewUser(char * username, char * password, char * firstName, char * lastName, char * country);
int tempUserStart();
int searchForUser(char * username);
int compareUserPass(char * username, char * password);

#endif
