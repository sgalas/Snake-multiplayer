#include<windows.h>
#include<stdio.h>
#include<vector>
#include<time.h>
#include<winsock.h>
#include<fstream>
#include<iostream>
#include<direct.h>
#define IP "46.41.151.118"
#define PORT 9034
using namespace std;
int kierunek=-1,owocX,owocY;
char plansza[20][20];
bool terazZjadl=0,przegrales=0;
char bufor[1024];
sockaddr_in server;
SOCKET s;
struct Segment
{
    int x;
    int y;
    Segment(int xx,int yy)
    {
        x=xx;
        y=yy;
    }
};
std::vector<Segment> Snake;
std::vector<Segment> Enemy;
void encode()
{
    bufor[0]=char(Snake.size()+32);
    for(unsigned int i=1;i<Snake.size()+1;i++)
    {
        bufor[(2*i)-1]=char(Snake[i-1].x+32);
        bufor[2*i]=char(Snake[i-1].y+32);
    }
    bufor[(2*Snake.size())+1]=char((int)terazZjadl+32);
}
bool nowaRunda = 0;
void decode()
{
    if(bufor[0]=='W')
    {
        printf("Wygrales.");
        Sleep(10000);
        nowaRunda = 1;
        return;
    }
    if( (int)Enemy.size() < bufor[0]-32) Enemy.push_back(Segment(0,0));
    int ile = 1+(2*Enemy.size())+2;
    for(int i=0;i<bufor[0]-32;i++)
    {
        Enemy[i].x = bufor[(2*(i+1))-1]-32;
        Enemy[i].y = bufor[2*(i+1)]-32;
    }
    owocX = bufor[ile-2]-32;
    owocY = bufor[ile-1]-32;
}
int enemyLastX = -1,enemyLastY = -1;
void placeEnemyAndOwoc()
{
    if(enemyLastX>0 && enemyLastY)
    {
        plansza[enemyLastY][enemyLastX]=' ';
    }

    for(unsigned int i=0;i<Enemy.size();i++)
    {
        plansza[Enemy[i].y][Enemy[i].x]='e';
    }
    plansza[owocY][owocX] = 'o';
    enemyLastX = Enemy[Enemy.size()-1].x;
    enemyLastY = Enemy[Enemy.size()-1].y;
}
void PoruszSieIAktualizujPlanszeSprawdzKolizje()
{
    if(!terazZjadl) plansza[Snake[ Snake.size()-1 ].y][Snake[ Snake.size()-1 ].x]=' ';
    for(int i=Snake.size()-1-terazZjadl;i>0;i--)
    {
        Snake[i].x = Snake[i-1].x;
        Snake[i].y = Snake[i-1].y;
    }
    if(kierunek==0)
    {
        Snake[0].x-=1;
    }
    else if(kierunek==1)
    {
        Snake[0].y-=1;
    }
    else if(kierunek==2)
    {
        Snake[0].x+=1;
    }
    else if(kierunek==3)
    {
        Snake[0].y+=1;
    }



    //jesli walnoles w sciane
    if(Snake[0].x==0||Snake[0].x==19||Snake[0].y==0||Snake[0].y==19)
    {
        przegrales = 1;
    }
    //jesli wszedles w siebie
    for(unsigned int i=1;i<Snake.size();i++)
    {
        if(Snake[0].x==Snake[i].x && Snake[0].y==Snake[i].y)
        {
            przegrales = 1;
            break;
        }
    }
    //jesli wszedles w przeciwnika
    for(unsigned int i=0;i<Enemy.size();i++)
    {
        if(Snake[0].x==Enemy[i].x && Snake[0].y==Enemy[i].y)
        {
            przegrales = 1;
            break;
        }
    }
    if(Enemy.size()>14)
    {
        przegrales = 1;
    }
    if(przegrales)
    {
        system("cls");
        plansza[Snake[0].y][Snake[0].x]='s';
        for(int i=0;i<20;i++)
        {
            for(int j=0;j<40;j++)
            {
                if(!(j%2))
                    printf("%c",plansza[i][j/2]);
                else
                    printf(" ");
            }
            printf("\n");
        }
        printf("Dlugosc: %i\n",Snake.size());
        printf("%s","Przegrales.\n");
        memset(bufor,'\0',1024);
        bufor[0]='p';
        send(s,bufor,1,0);
        Sleep(10000);
        exit(0);
    }



    plansza[Snake[0].y][Snake[0].x]='s';
}
HANDLE h;
void init()
{
    h = GetStdHandle(STD_OUTPUT_HANDLE);

    WSADATA wsa;
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return;
	}
	printf("Initialised.\n");

	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}
	printf("Socket created.\n");

	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return;
	}
	puts("Connected.\n");

	send(s,"siema",6,0);
}
void RESET()
{
    kierunek = -1;
    nowaRunda = 0;
    Snake.clear();
    Enemy.clear();
    memset(bufor,'\0',1024);
}
void animejszyn()
{
    string screen[17];
    int animationSleepTime = 25;
    for(int i=0;i<17;i++)
    {
        screen[i]="";
    }
    fstream text_file;
    fstream snake_file;
    text_file.open("text_ascii.txt");
    snake_file.open("snake_ascii.txt");

    string s;
    //SNEJK WJEZDZA
    for(int i=53;i>=0;i--)
    {
        for(int j=0;j<17;j++)
        {
            getline(snake_file,s);
            screen[j]=s[i]+screen[j];
            cout<<screen[j]<<endl;
        }
        Sleep(animationSleepTime);
        system("cls");
        snake_file.seekg(0,snake_file.beg);
    }
    //SPACJE PO SNEJKU
    for(int i=0;i<25;i++)
    {
        for(int j=0;j<17;j++)
        {
            screen[j]=' '+screen[j];
            cout<<screen[j]<<endl;
        }
        Sleep(animationSleepTime);
        system("cls");
    }
    //NAPIS WJEZDZA
    for(int i=0;i<51;i++)
    {
        for(int j=0;j<17;j++)
        {
            getline(text_file,s);
            screen[j].insert(25+i,string(1,s[i]));
            cout<<screen[j]<<endl;
        }
        Sleep(animationSleepTime);
        system("cls");
        text_file.seekg(0,snake_file.beg);
    }
    //SPACJE PO NAPISIE
    for(int i=0;i<50;i++)
    {
        for(int j=0;j<17;j++)
        {
            screen[j].insert(25+51," ");
            cout<<screen[j]<<endl;
        }
        Sleep(animationSleepTime);
        system("cls");
    }
    text_file.close();
    snake_file.close();
    return;
}
void playmusic()
{
    char pathbuff[FILENAME_MAX];
    _getcwd(pathbuff,FILENAME_MAX);
    string path(pathbuff);

    string filename;
    filename="Better.wav";

    string pathfull=(path+"\\"+filename);
    LPCSTR Lpathfull=pathfull.c_str();

    PlaySound(Lpathfull,NULL,SND_FILENAME|SND_ASYNC);
    return;
}
int main()
{
    playmusic();
    animejszyn();
    init();
AGAIN:

    recv(s,bufor,1024,0);
    if(bufor=="Wygrales")
    {
        printf("Gratulacje! Wygrales caly turniej!");
        Sleep(10000);
        exit(0);
    }
    Snake.push_back(Segment(bufor[0]-32,bufor[1]-32));
    Enemy.push_back(Segment(1,1));
    for(int i=0;i<20;i++)
    {
        for(int j=0;j<20;j++)
        {
            if(i==bufor[1]-32&&j==bufor[0]-32)
                plansza[i][j]='s';
            else if(i==0||i==19)
                plansza[i][j]='_';
            else if(j==0||j==19)
                plansza[i][j]='|';
            else
                plansza[i][j]=' ';
        }
    }



    while(1)
    {
        if(GetAsyncKeyState(VK_LEFT) && kierunek!=2)
           kierunek=0;
        else if(GetAsyncKeyState(VK_UP) && kierunek!=3)
           kierunek=1;
        else if(GetAsyncKeyState(VK_RIGHT) && kierunek!=0)
           kierunek=2;
        else if(GetAsyncKeyState(VK_DOWN) && kierunek!=1)
           kierunek=3;



        if(Snake[0].x==owocX && Snake[0].y==owocY)
        {
            Snake.push_back(Segment( Snake[Snake.size()-1].x, Snake[Snake.size()-1].y ));
            terazZjadl=1;
        }
        PoruszSieIAktualizujPlanszeSprawdzKolizje();



        memset(bufor,'\0',sizeof(bufor));
        encode();
        send(s,bufor,1+(2*Snake.size())+1,0);
        recv(s,bufor,1024,0);

        Sleep(100);
        system("cls");

        decode();
        if(nowaRunda)
        {
            RESET();
            goto AGAIN;
        }
        placeEnemyAndOwoc();



        for(int i=0;i<20;i++)
        {
            for(int j=0;j<40;j++)
            {
                if(!(j%2))
                {
                    if(plansza[i][j/2]=='s')
                    {
                        SetConsoleTextAttribute(h,10);
                        printf("%c",plansza[i][j/2]);
                        SetConsoleTextAttribute(h,7);
                    }
                    else if(plansza[i][j/2]=='e')
                    {
                        SetConsoleTextAttribute(h,12);
                        printf("%c",plansza[i][j/2]);
                        SetConsoleTextAttribute(h,7);
                    }
                    else
                    {
                        printf("%c",plansza[i][j/2]);
                    }
                }
                else
                {
                    printf(" ");
                }
            }
            printf("\n");
        }
        printf("Dlugosc: %i\n",Snake.size());



        terazZjadl=0;
    }
}
