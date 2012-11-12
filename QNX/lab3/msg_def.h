#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_

#include <sys/iomsg.h>

#define SIZE 256
#define LOWCASE_MSG_TYPE _IO_MAX + 1

typedef struct  {
   int  type;        // typ komunikatu
   int  from;       // nr procesu który wys³a³ komunikat
   int  count;        // ile by³o malych liter
   char text[SIZE]; // tekst komunikatu
 } mss_t;

typedef struct {
         int  type;            // Typ komunikatu
         int  from;              // Numer procesu
         char text[SIZE];    // Tekst komunikatu
 } kom_t;

#define ATTACH_POINT "edu.uj.rts"

#define OPENR 1 // Otwarcie pliku do odczytu
#define OPENW 2 // Otwarcie pliku do zapisu
#define READ 3 // Odczyt fragmentu pliku
#define WRITE 4 // Zapis fragmentu pliku
#define CLOSE 5 // Zamkniecie pliku
#define STOP 10 // Zatrzymanie serwera

typedef struct  {
    int  type;       // typ zlecenia
    int  bytes_count;       // liczba bajtow
    int  fh;        // uchwyt pliku
    char buffer[SIZE]; // bufor
} mms;

#endif //_MSG_DEF_H_
