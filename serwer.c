#include<stdio.h>

#include<stdlib.h>

#include<string.h>

#include<unistd.h>

#include<sys/types.h>

#include<sys/socket.h>

#include<netinet/in.h>

#include<arpa/inet.h>

#include<netdb.h>

#include<unistd.h>

#include<time.h>

#include<stdbool.h>

#include<math.h>

struct Para {
  int jeden;
  int dwa;
};
struct Owoc {
  char x;
  char y;
};
int main() {
  srand(time(NULL));
  struct timeval tescik;
  tescik.tv_sec = 0;
  tescik.tv_usec = 1000;
  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  struct sockaddr_in my_addr;
  memset(my_addr.sin_zero, '0', sizeof my_addr.sin_zero);

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(9034); // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY;

  bind(sockfd, (struct sockaddr * ) & my_addr, sizeof my_addr);

  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  listen(sockfd, 5);

  struct sockaddr_in klientAdr;
  socklen_t addr_size = sizeof(klientAdr);

  char bufor[1024];
  int ileZnak;
  int klient;

  fd_set master; //zbior wszystkich socketow ktore sie z nami polaczyly
  fd_set read_fds; // tymczasowy set
  FD_ZERO( & master);
  FD_ZERO( & read_fds);
  int fdmax = sockfd; // maksymalny numer socket
  bool graRozpoczeta = 0;
  time_t seconds = time(NULL) * 2;
  int iloscZawodnikow = 0;
  // jak cos to sa 2 fd_sety bo przy funckji select() w fd_secie zostaja tylko te sockety ktore zostaly zmodyfikowane tzn. ktos do nich cos napisal
  while (1) {
    if (!graRozpoczeta) {
      if (time(NULL) - seconds > 5) //Gra rozpoczyna sie po 5 sekundach
        graRozpoczeta = 1;
      FD_SET(sockfd, & master);
      read_fds = master; // w read_fds znajduja sie teraz ttlko te sockety w ktorych pojawilo sie cos nowego (klint cos wpisal)
      tescik.tv_sec = 0;
      tescik.tv_usec = 1000;
      int temp = select(fdmax + 1, & read_fds, NULL, NULL, & tescik);
      if (temp <= 0)
        continue;
      for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, & read_fds)) // sprawdza czy w danym zbiorze jest element
        {
          memset(bufor, '\0', sizeof(bufor));

          if (i == sockfd) //jezeli socket byl socketem do odbierania polaczen
          {
            klient = accept(sockfd, (struct sockaddr * ) & klientAdr, & addr_size);
            recv(klient, bufor, sizeof(bufor), 0);
	    //zabezpieczenie przed nieproszonymi połączeniami
            if (strncmp(bufor, "siema", 5) != 0) {
              close(klient); 
            } else {
              printf("\nDolaczyl gracz ziomel o numerze %d\n", klient);
              FD_SET(klient, & master); //nowy gracz 
              if (klient > fdmax)
                fdmax = klient;

              iloscZawodnikow++;
              if (iloscZawodnikow == 2) {
                printf("\nMamy 2 graczy\n");
                seconds = time(NULL);
              }
            }
          } else {
            //nowa wiadomosc
            ileZnak = recv(i, bufor, sizeof(bufor), 0);
            if (!ileZnak) {
              //koniec polaczenia
              FD_CLR(i, & master);
              close(i);
              iloscZawodnikow--;
            }
          }
        }
      }
    }
    if (graRozpoczeta) {
      char bufor[2048];
      printf("Zaczyna sie gra\n");
      fd_set masterGracze = master;
      FD_CLR(sockfd, & masterGracze);
      fd_set Gracze = masterGracze;
      int index = 0;
      int * tablicaGraczy = (int * ) malloc(sizeof(int) * iloscZawodnikow); // robimy tablice, ktora bedzie przechowywala numery socketow. Tablica wykorzystywana TYLKO do losowania 2 graczy w pojedynku
      int liczbaTur = log2((double) iloscZawodnikow); 
      if (pow(2, liczbaTur) != iloscZawodnikow) {
        liczbaTur++;
      }
      printf("Bedzie %d tur\n", liczbaTur);
      while (liczbaTur--) { // logika gry
        struct Owoc * Owoce = (struct Owoc * ) malloc(sizeof(struct Owoc) * iloscZawodnikow / 2);
        struct Para * Pojedynki = (struct Para * ) malloc(sizeof(struct Para) * iloscZawodnikow / 2);
        for (int i = 0; i <= fdmax; i++) {
          if (FD_ISSET(i, & masterGracze)) {
            if (i != sockfd)
              tablicaGraczy[index++] = i;
            //wypelniam tablice wszystkimi obecnymi graczami
          }
        }
        for (int i = 0; i < iloscZawodnikow / 2; i++) {
          Owoce[i].x = rand() % 18 + 33;
          Owoce[i].y = rand() % 18 + 33;
          int randomOne = rand() % iloscZawodnikow; // losowanie pierwszego zawodnika
          while (tablicaGraczy[randomOne] == -1) // -1 oznacza zawodnika ktory juz zostal wybrany do poprzedniego meczu
          {
            randomOne = rand() % iloscZawodnikow;
          }
          int randomTwo = rand() % iloscZawodnikow; // losowanie drugiego zawodnika
          while (tablicaGraczy[randomTwo] == -1 || randomTwo == randomOne) {
            randomTwo = rand() % iloscZawodnikow;
          }
          printf("\nGracz %d bedzie sie mierzyl z graczem %d\n", tablicaGraczy[randomOne], tablicaGraczy[randomTwo]);
          Pojedynki[i].jeden = tablicaGraczy[randomOne];
          Pojedynki[i].dwa = tablicaGraczy[randomTwo]; // rozpoczyna sie pojedynek pomiedzy dwoma graczami. Funkcja zwraca numer socketa gracza przegranego
	// Losowanie poczatkowych pozycji obu graczy
          struct Owoc * PoczPozycje = (struct Owoc * ) malloc(sizeof(struct Owoc));
          for (int i = 0; i < 2; i++) {
            PoczPozycje[i].x = rand() % 18 + 33;
            PoczPozycje[i].y = rand() % 18 + 33;
          }
          while (PoczPozycje[0].x == PoczPozycje[1].x) {
            PoczPozycje[1].x = rand() % 18 + 33;
          }
          char buforTemp[3] = {
            0
          };
          char buforTemp2[3] = {
            0
          };
          sprintf(buforTemp, "%c%c", PoczPozycje[0].x, PoczPozycje[0].y);
          sprintf(buforTemp2, "%c%c", PoczPozycje[1].x, PoczPozycje[1].y);
          free(PoczPozycje);
          send(Pojedynki[i].jeden, buforTemp, sizeof(buforTemp), 0);
          send(Pojedynki[i].dwa, buforTemp2, sizeof(buforTemp2), 0);
          tablicaGraczy[randomOne] = -1;
          tablicaGraczy[randomTwo] = -1; // obu indeksom w tablicy przypisujujemy -1, po to aby ten sam gracz nie walczyl 2 razy w tej samej turze
        }
        int iloscPorazek = 0;
        while ((iloscZawodnikow / 2) != iloscPorazek) { // Tyle porazek powinniśmy otrzymać po wykonaniu jednej tury
          Gracze = masterGracze;
          select(fdmax + 1, & Gracze, NULL, NULL, NULL);
          char buforini[3];
          for (int i = 0; i < iloscZawodnikow / 2; i++) //czytanie wiadomosci i przesylani
          {
            if (FD_ISSET(Pojedynki[i].jeden, & Gracze)) {
              printf("Otrzymano Wiadomosc od socketa %d\n", Pojedynki[i].jeden);
              memset(bufor, 0, sizeof(bufor));
              int ileCharow = recv(Pojedynki[i].jeden, bufor, sizeof(bufor), 0);
              if (bufor[ileCharow - 1] == '!') {
                Owoce[i].x = rand() % 18 + 33;
                Owoce[i].y = rand() % 18 + 33;
              }
              if (ileCharow == 1 || ileCharow == 0) {
                printf("\nGracz %d przegral\n", Pojedynki[i].jeden);
                //wiadomosc o wygranej
                send(Pojedynki[i].dwa, "W", sizeof("W"), 0);
                close(Pojedynki[i].jeden);
                FD_CLR(Pojedynki[i].jeden, & masterGracze);
                iloscPorazek++;
              } else {
                bufor[ileCharow - 1] = 0;
                sprintf(buforini, "%c%c", Owoce[i].x, Owoce[i].y);
                strcat(bufor, buforini); // drugi kord
                printf("\bBUFOR TO %s\n", bufor);
                send(Pojedynki[i].dwa, bufor, ileCharow + 1, 0);
              }
            }
            if (FD_ISSET(Pojedynki[i].dwa, & Gracze)) {
              printf("Otrzymano Wiadomosc od socketa %d\n", Pojedynki[i].dwa);
              memset(bufor, 0, sizeof(bufor));
              int ileCharow = recv(Pojedynki[i].dwa, bufor, sizeof(bufor), 0);
              if (bufor[ileCharow - 1] == '!') {
                Owoce[i].x = rand() % 18 + 33; // generuje pierwszy kord owoca i dopisuje go na koniec bufera
                Owoce[i].y = rand() % 18 + 33;
              }
              if (ileCharow == 1 || ileCharow == 0) {
                printf("\nGracz %d przegral\n", Pojedynki[i].dwa);
		//wiadomosc o wygranej
                send(Pojedynki[i].jeden, "W", sizeof("W"), 0);
                close(Pojedynki[i].dwa);
                FD_CLR(Pojedynki[i].dwa, & masterGracze);
                iloscPorazek++;
              } else {
                bufor[ileCharow - 1] = 0;
                sprintf(buforini, "%c%c", Owoce[i].x, Owoce[i].y);
                strcat(bufor, buforini); // drugi kord
                printf("\bBUFOR TO %s\n", bufor);
                send(Pojedynki[i].jeden, bufor, ileCharow + 1, 0);
              }
            }

          }
        }
        //cleanup
        for (int i = 0; i < iloscZawodnikow; i++) {
          tablicaGraczy[i] = -1;
        }
        iloscZawodnikow = iloscZawodnikow / 2 + iloscZawodnikow % 2; // W taki sposob zmienia sie liczba zawodnikow po jednej turze 
        index = 0;
        free(Pojedynki);
        free(Owoce);
      }
      //koniec turnieju
      for (int i = 0; i <= fdmax; i++) {
        if (FD_ISSET(i, & masterGracze)) {
          //jedyny socket ktory pozostal to zwyciezca
          send(i, "Wygrales", sizeof("Wygrales"), 0);
	  //Rozpoczecie dzialania serwera od poczatku
          graRozpoczeta = 0;
          free(tablicaGraczy);
          iloscZawodnikow = 0;
          liczbaTur = 0;
          seconds = 2 * time(NULL);
          close(i);
          fd_set master; //zbior wszystkich socketow ktore sie z nami polaczyly
          fd_set read_fds; // tymczasowy set
          FD_ZERO( & master);
          FD_ZERO( & read_fds);
          FD_SET(sockfd, & master);
          fdmax = sockfd; // maksymalny numer socketa
          read_fds = master; // w read_fds znajduja sie teraz ttlko te sockety w ktorych pojawilo sie cos nowego (klint cos wpisal)
          break;
        }
      }
    }
  }
}
