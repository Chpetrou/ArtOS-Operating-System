#include <stdio.h>
#include <time.h>
#include <time_date.h>
#include <stdlib.h>
#include <terminal.h>
#include <rand.h>

//ArtOS edition of Snake Game (Fidaki) implementation

#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

int length;
int bend_no;
int len;
char key;
//void record();
void load();
int life;
void Delay(long double);
void Move();
void Food();
int Score();
int Print();
void gotoxy(int x, int y);
void gotoxy(int x,int y);
void Bend();
void Boarder();
void Down();
void Left();
void Up();
void Right();
void ExitGame();
int Scoreonly();

void upKeyPressed() {
    key = UP;
}
void downKeyPressed() {
    key = DOWN;
}
void leftKeyPressed() {
    key = LEFT;
}
void rightKeyPressed() {
    key = RIGHT;
}

struct coordinate{
    int x;
    int y;
    int direction;
};

typedef struct coordinate coordinate;

coordinate head, bend[500],food,body[30];

//Start the fidaki
int snakeMain()
{
    upKey = &upKeyPressed;
    downKey = &downKeyPressed;
    leftKey = &leftKeyPressed;
    rightKey = &rightKeyPressed;
    
    char key;
    
    if (Print() == 0) return 0;
    
    load();
    
    length=5;
    
    head.x=25;
    
    head.y=20;
    
    head.direction=RIGHT;
    
    Boarder();
    
    Food(); //to generate food coordinates initially
    
    life=3; //number of extra lives
    
    bend[0]=head;
    
    Move();   //initialing initial bend coordinate
    
    return 0;
    
}

//Move the fidaki
void Move()
{
    int a,i;
    
    do{
        
        Food();
        
        len=0;
        
        for(i=0;i<30;i++)
        {
            
            body[i].x=0;
            
            body[i].y=0;
            
            if(i==length)
                
                break;
            
        }
        
        Score();
        waitTicks(20);
        if (isNextCharReady == 1 && isButtonPressed == 0) isButtonPressed = 1;
        else isButtonPressed = 0;
        
        //        nextButtonAvailable = 0;
        Boarder();
        
        //        kprintf("hd: %d", head.direction);
        
        if(head.direction==RIGHT)
            
            Right();
        
        else if(head.direction==LEFT)
            
            Left();
        
        else if(head.direction==DOWN)
            
            Down();
        
        else if(head.direction==UP)
            
            Up();
        
        ExitGame();
        
    }while(isButtonPressed == 0);
    
    a=getChar();
    
    if(a==27)
        
    {
        
        monInit();
        
        return;
        
    }
    switch (getChar()) {
        case 'w':
            key = UP;
            break;
        case 's':
            key = DOWN;
            break;
        case 'a':
            key = LEFT;
            break;
        case 'd':
            key = RIGHT;
            break;
            
        default:
            break;
    }
    
    if((key==RIGHT&&head.direction!=LEFT&&head.direction!=RIGHT)||(key==LEFT&&head.direction!=RIGHT&&head.direction!=LEFT)||(key==UP&&head.direction!=DOWN&&head.direction!=UP)||(key==DOWN&&head.direction!=UP&&head.direction!=DOWN))
        
    {
        
        bend_no++;
        
        bend[bend_no]=head;
        
        head.direction=key;
        
        if(key==UP)
            
            head.y--;
        
        if(key==DOWN)
            
            head.y++;
        
        if(key==RIGHT)
            
            head.x++;
        
        if(key==LEFT)
            
            head.x--;
        
        Move();
        
    }
    
    else if(key==27)
        
    {
        
        monInit();
        
        return;
        
    }
    
    else
        
    {
        
        kprintf("\a");
        
        Move();
        
    }
}

//Go to specific place in terminal
void gotoxy(int x, int y)
{
    
    COORD coord;
    
    //    if (x > 79) {
    //         coord.X = 79;
    //    } else {
    coord.X = x;
    //    }
    //    if (y > 24) {
    //        coord.Y = 24;
    //    } else {
    coord.Y = y;
    //    }
    
    setCursorToSpecificLocation(coord);
    
}

//Loads the fidaki
void load(){
    int row,col,r,c,q;
    gotoxy(36,14);
    kprintf("loading...");
    gotoxy(30,15);
    for(r=1;r<=20;r++){
        for(q=0;q<=1000000000;q++);//to display the character slowly
        kprintf("#");
    }
    getChar();
}

//Make fidaki go down
void Down()
{
    int i;
    for(i=0;i<=(head.y-bend[bend_no].y)&&len<length;i++)
    {
        gotoxy(head.x,head.y-i);
        {
            if(len==0)
                kprintf("v");
            else
                kprintf("o");
        }
        body[len].x=head.x;
        body[len].y=head.y-i;
        len++;
    }
    Bend();
    if(isButtonPressed != 1)
        head.y++;
}

//Delay for movement of fidaki
void Delay(long double k)
{
    Score();
    long double i;
    for(i=0;i<=(10000000);i++);
}

//Exit the fidaki game
void ExitGame()
{
    int i,check=0;
    for(i=4;i<length;i++)   //starts with 4 because it needs minimum 4 element to touch its own body
    {
        if(body[0].x==body[i].x&&body[0].y==body[i].y)
        {
            check++;    //check's value increases as the coordinates of head is equal to any other body coordinate
        }
        if(i==length||check!=0)
            break;
    }
    if(head.x<=0||head.x>=79||head.y<=0||head.y>=24||check!=0)
    {
        life--;
        if(life>=0)
        {
            head.x=25;
            head.y=20;
            bend_no=0;
            head.direction=RIGHT;
            Move();
        }
        else
        {
            monInit();
            kprintf("All lives completed\nBetter Luck Next Time!!!\nPress any key to quit the game\n");
            //            record();
            return 0;
        }
    }
}

//Show food in "random" places
void Food()
{
    if(head.x==food.x&&head.y==food.y)
    {
        length++;
        time_t a;
        a=(time_t) get_clock_tick();//time(0);
        srand(a);
        food.x=rand()%78;
        if(food.x<=0)
            food.x+=11;
        food.y=rand()%23;
        if(food.y<=0)
            
            food.y+=11;
    }
    else if(food.x==0)/*to create food for the first time coz global variable are initialized with 0*/
    {
        food.x=rand()%78;
        if(food.x<=0)
            food.x+=11;
        food.y=rand()%23;
        if(food.y<=0)
            food.y+=11;
    }
    //    getChar();
}

//Make fidaki go left
void Left()
{
    int i;
    for(i=0;i<=(bend[bend_no].x-head.x)&&len<length;i++)
    {
        gotoxy((head.x+i),head.y);
        {
            if(len==0)
                kprintf("<");
            else
                kprintf("o");
        }
        body[len].x=head.x+i;
        body[len].y=head.y;
        len++;
    }
    Bend();
    if(isButtonPressed != 1)
        head.x--;
    
}

//Make fidaki go right
void Right()
{
    int i;
    for(i=0;i<=(head.x-bend[bend_no].x)&&len<length;i++)
    {
        //gotoxy((head.x-i),head.y);
        body[len].x=head.x-i;
        body[len].y=head.y;
        gotoxy(body[len].x,body[len].y);
        {
            if(len==0)
                kprintf(">");
            else
                kprintf("o");
        }
        /*body[len].x=head.x-i;
         body[len].y=head.y;*/
        len++;
    }
    Bend();
    if(isButtonPressed != 1)
        head.x++;
}

//Make fidaki go up
void Up()
{
    int i;
    for(i=0;i<=(bend[bend_no].y-head.y)&&len<length;i++)
    {
        gotoxy(head.x,head.y+i);
        {
            if(len==0)
                kprintf("^");
            else
                kprintf("o");
        }
        body[len].x=head.x;
        body[len].y=head.y+i;
        len++;
    }
    Bend();
    if(isButtonPressed != 1)
        head.y--;
}

////Make fidaki bend its body
void Bend()
{
    int i,j,diff;
    for(i=bend_no;i>=0&&len<length;i--)
    {
        if(bend[i].x==bend[i-1].x)
        {
            diff=bend[i].y-bend[i-1].y;
            if(diff<0)
                for(j=1;j<=(-diff);j++)
                {
                    body[len].x=bend[i].x;
                    body[len].y=bend[i].y+j;
                    gotoxy(body[len].x,body[len].y);
                    kprintf("o");
                    len++;
                    if(len==length)
                        break;
                }
            else if(diff>0)
                for(j=1;j<=diff;j++)
                {
                    /*gotoxy(bend[i].x,(bend[i].y-j));
                     kprintf("*");*/
                    body[len].x=bend[i].x;
                    body[len].y=bend[i].y-j;
                    gotoxy(body[len].x,body[len].y);
                    kprintf("o");
                    len++;
                    if(len==length)
                        break;
                }
        }
        else if(bend[i].y==bend[i-1].y)
        {
            diff=bend[i].x-bend[i-1].x;
            if(diff<0)
                for(j=1;j<=(-diff)&&len<length;j++)
                {
                    /*gotoxy((bend[i].x+j),bend[i].y);
                     kprintf("*");*/
                    body[len].x=bend[i].x+j;
                    body[len].y=bend[i].y;
                    gotoxy(body[len].x,body[len].y);
                    kprintf("o");
                    len++;
                    if(len==length)
                        break;
                }
            else if(diff>0)
                for(j=1;j<=diff&&len<length;j++)
                {
                    /*gotoxy((bend[i].x-j),bend[i].y);
                     kprintf("*");*/
                    body[len].x=bend[i].x-j;
                    body[len].y=bend[i].y;
                    gotoxy(body[len].x,body[len].y);
                    kprintf("o");
                    len++;
                    if(len==length)
                        break;
                }
        }
    }
}

//Borders of the fidaki game
void Boarder()
{
    monInit();
    //    kprintf("geia");
    int i;
    gotoxy(food.x,food.y);   /*displaying food*/
    kprintf("X");
    for(i=0;i<79;i++)
    {
        gotoxy(i,0);
        kprintf("#");
        gotoxy(i,24);
        kprintf("#");
    }
    for(i=0;i<24;i++)
    {
        gotoxy(0,i);
        kprintf("#");
        gotoxy(79,i);
        kprintf("#");
    }
    //    getChar();
}

//Initial information about the game
int Print()
{
    //gotoxy(10,12);
    switch (language) {
        case GREEK:
            kprintf("\tKalwsorisate sto Fidaki.(patiste opiodipote pliktro gia na proxorisete)\n");
            break;
        case SPANISH:
            kprintf("\tBienvenido al juego Fidaki. (Presiona cualquier tecla para continuar)\n");
            break;
        case ENGLISH:
            kprintf("\tWelcome to the Fidaki game.(press any key to continue)\n");
            break;
        default:
            break;
    }
    getChar();
    monInit();
    switch (language) {
        case GREEK:
            kprintf("\tOdigeies Paixnidiou:\n");
            kprintf("\n-> Xrisimopoiiste to W-A-S-D gia na metakineite to fidaki.");
            kprintf("\n\n-> Tha vlepete fagito se diafora simia tis othonis pou tha prepei na fate. Kathe fora pou trwte ena, to mikos tou fidiou tha megalwnei kata 1 simeio opos kai i vathmologia sas.");
            kprintf("\n\n-> Ksekinate me treis zwes. I zoi sas meionete kata ena kathe fora pou aggizete ton toixo i to soma tou fidiou.Here you are provided with three lives. Your life will decrease as you hit the wall or snake's body.\n\n->An thelete na stamatisete patiste to pliktro esc.\n");
            kprintf("\n\n Patiste opiodipote pliktro gia na arxisete to paixnidi...");
            break;
        case SPANISH:
            kprintf("\tInstrucciones del juego:\n");
            kprintf("\n-> Use las teclas W-A-S-D para mover la serpiente");
            kprintf("\n\n-> Vera alimentos en las diversas coordenadas de la pantalla que debe comer. Cada vez que comas un alimento, la longitud de la serpiente aumentara en uno elemento y tambien en el puntaje.");
            kprintf("\n\n-> Comienzas con tres vidas. Tu vida disminuirá al golpear la pared o el cuerpo de la serpiente.\n\n-> Si quieres salir, presiona la tecla esc.\n");
            kprintf("\n\n Presiona cualquier tecla para jugar el juego ...");
            break;
        case ENGLISH:
            kprintf("\tGame instructions:\n");
            kprintf("\n-> Use W-A-S-D keys to move the snake.");
            kprintf("\n\n-> You will see foods at the several coordinates of the screen which you have to eat. Every time you eat a food the length of the snake will be increased by one element and also the score.");
            kprintf("\n\n-> You start with three lives. Your life will decrease as you hit the wall or snake's body.\n\n-> If you want to exit press esc key.\n");
            kprintf("\n\nPress any key to play game...");
            break;
        default:
            break;
    }
    if(getChar()==27)
        return 0;
    return 1;
}

//Shows the score of the game
int Score()
{
    int score;
    gotoxy(20,8);
    score=length-5;
    switch (language) {
        case GREEK:
            kprintf("VATHMOLOGIA : %d",(length-5));
            break;
        case SPANISH:
            kprintf("PUNTUACION : %d",(length-5));
            break;
        case ENGLISH:
            kprintf("SCORE : %d",(length-5));
            break;
        default:
            break;
    }
    score=length-5;
    gotoxy(50,8);
    switch (language) {
        case GREEK:
            kprintf("Zwi : %d",life);
            break;
        case SPANISH:
            kprintf("Vida : %d",life);
            break;
        case ENGLISH:
            kprintf("Life : %d",life);
            break;
        default:
            break;
    }
    return score;
}

//Shows only the score in the screen
int Scoreonly()
{
    int score=Score();
    monInit();
    return score;
}
