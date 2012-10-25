#define F_CPU 16000000UL  // 16 MHz
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#define TASK_SIZE 4
#define INVALID_VALUE 0xFFFFFFFF
#define TRIES_COUNT 2

/*
 Port A - LCD
 Port B - 7-dmio segmentowy
 Port C - klawiatura
 Port D - Sterowanie kolumnami siedmiosegmentowego
 */


int columns[] = {224, 208, 176, 112}; //Liczby odpowiadaja adresowaniu odpowiedniego wyswietlacza

int timeToChangeColumn = 0; //liczba ktora sluzy do przelaczenia wyswietlacza w przerwaniu po 5ms

int column = 0; //aktualna kolumna do wyswietlenia

int digits[] = {192, 249, 164, 176, 153, 146, 130, 248, 128, 144}; //kody do siedmiosegmentowego odpowiadajace kolejnym cyfrom

int digitsROWS[] = {0x70,0xB0,0xD0,0xE0}; //wartosci na PINC odpowiadajace wcisnietemu konretnemu wierszowi

int digitsCOLUMNS[] = {0x07,0x0B,0x0D,0x0E}; //wartosci na PINC odpowiadajace wcisnietemu konretnej kolumnie

int buttonRow = 0; //ktory wiersz jest wcisniety
int buttonColumn = 0; //ktora kolumna jest wcisnieta

uint8_t Displays[4]; //kazda z czterech cyfr w odpowiednim indeksie

uint32_t time = 0; // ilosc sekund
int seed = 0; //seed do randoma

int previousButton = -1; // okresla wcisniety guzik
int LCDposition = 0; //aktualna pozycja na wyswietlaczu


uint8_t isStarted = 0; // okresla czy gra sie zaczela
uint8_t player = 1; //okresla ktory gracz aktualnie gra
uint8_t position = 0; //okresla ktora pozycja liczby zostala juz wpisana
uint8_t tries = 0; //okresla ile wynikow zostalo prawidlowo podanych
uint8_t failedTries = 0; //okresla ile wynikow zostalo nieprawidlowo podanych
char result[5]; //tablica z wpisywanym wynikiem
uint32_t expectedResult; //prawidlowy wynik danego dzialania
uint8_t isEnded = 0; //okresla czy gra jest zakonczona
uint32_t playerOneTime = 0;
uint32_t playerTwoTime = 0;

typedef struct
{
	void (*foo)();
	uint32_t timeout;
	uint32_t timetogo;
	uint32_t ready;
} TASK;

TASK TASK_TABLE[TASK_SIZE];

ISR(TIMER0_COMP_vect)
{
	if (timeToChangeColumn < 4) {
		timeToChangeColumn++;
	}
	else {
		changeColumn(); //zmien kolumne co 5ms
		timeToChangeColumn = 0;
	}
	
	int i;
	for (i =0; i < TASK_SIZE; i++)
	{
        if (1 == TASK_TABLE[i].timetogo)
		{
			if (TASK_TABLE[i].timeout != INVALID_VALUE)
			{
				TASK_TABLE[i].ready++;
				TASK_TABLE[i].timetogo = TASK_TABLE[i].timeout;
			}
		}
		else
		{
			TASK_TABLE[i].timetogo--;
		}
	}
}

int schedule()
{
	while (1)
	{
		uint8_t i;
		for(i = 0; i < TASK_SIZE; ++i) {
			if(TASK_TABLE[i].ready != 0x00) {
				TASK_TABLE[i].foo();
				TASK_TABLE[i].ready--;
				break;
			}
		}
	}
	return 1234;
}

void delay(){ //opoznienie zeby czytalo z portow
	int i;
	for (i=0; i <=15; i++);
}

void addTask(void (*f)(),uint32_t timeout, uint32_t isPeriodic, uint8_t priority)
{
	if(priority<TASK_SIZE){
        TASK_TABLE[priority].foo=f;
        TASK_TABLE[priority].timetogo=timeout-1;
        TASK_TABLE[priority].ready=0;
        TASK_TABLE[priority].timeout=(isPeriodic)?timeout:INVALID_VALUE;
	}
}

//ISR(TIMER1_OVF_vect, ISR_ALIASOF(TIMER0_OVF_vect));

void setupTimer()
{
	OCR0 = 248; //wartosc do porownywania
	TCCR0 |= (1 << WGM00) | (1 << WGM01) | (1 << CS00) | (1 << CS01); //tryb CTC/prescaler 64/waveform generation mode
	TIMSK |= (1 << OCIE0); //compare interrupt
	sei(); //globalna flaga przerwan
}

EMPTY_INTERRUPT(TIMER2_OVF_vect);

ISR(BADISR_vect)
{
	cli();
	sei();
	//Catch all?
}

void setupPORTA() {
	DDRA = 0xFF;//PORTA caly na wyjscie
}

void setupPORTB() {
	DDRB = 0xFF; //PORTB caly na wyjscie
}

void setPORTB(int column) {
	PORTB = column; //ustawienie odpowiedniego wyswietlacza
}

void setupPORTD(){
	DDRD = (15 << 4);
}

void setPORTD(int digit) {
	PORTD = digit; //ustawienie liczby/cyfry, bo w danej chwili ustawiamy tylko jeden wyswietlacz
}

void updateDisplay(uint32_t val) { //wyluskanie odpowiednich cyfr z liczby sekund
	Displays[0] = (val/1000) % 10;
	Displays[1] = (val/100) % 10;
	Displays[2] = (val/10) % 10;
	Displays[3] = val % 10;
}

void incTime() { //zwiekszenie liczby sekund
	if(isStarted) {
		updateDisplay(++time);
	}
	
}

void incSeed() { //inkrementowanie seeda do randoma
	++seed;
}

void changeColumn() {
	PORTD = (PORTD & 0x0F) | (columns[column] & 0xF0);
	setPORTB(digits[Displays[column]]); //ustawienie konkretnej cyfry na odpowiednim wyswietlaczu
	column = (++column) % 4; //przejscie do nastepnej kolumny
}

void wait() { //petla opozniajaca
	int i;
	for (i = 0; i < 10000; i++) {
        
	}
}

void setupCOLUMNS(){ // ustawia mniej znaczace bity portu PORTC na czytanie i wlacza na nich rezystory podciagajace
	DDRC = 0xF0;
	PORTC = 0x0F;
}

void setupROWS() { // ustawia bardziej znaczace bity portu PORTC na czytanie i wlacza na nich rezystory podciagajace
	DDRC = 0x0F;
	PORTC = 0xF0;
}

void read() {
    delay();
    setupROWS(); //najpierw sprawdzamy wiersze
    delay();
    int i = 0;
    while(i < 4) { //sprawdza ktory wiersz jest wcisniety
        if( PINC == digitsROWS[i]) {
            break;
        }
        i++;
    }
    if(i != 4) { //jesli ktorys wiersz jest wcisniety
        buttonRow = i;
    }
    delay();
    setupCOLUMNS(); //sprawdzamy kolumny
    delay();
    i = 0;
    while(i < 4) { //sprawdza ktora kolumna jest wcisnieta
        if( PINC == digitsCOLUMNS[i]) {
            break;
        }
        i++;
    }
    if(i != 4) { //jezeli ktoras kolumna jest wcisnieta
        buttonColumn = i+1;
    }
    if(buttonColumn != 0) { //jezeli cos jest wcisniete
        if(previousButton == -1) { //jezeli guzik nie jest wciskany caly czas
            previousButton = buttonRow*4 + buttonColumn; //zapamietaj co jest wcisniete
            //przeciecie wcisnietego wiersza i kolumny daje wcisniety klawisz
        }
    }
    else {
        if(previousButton != -1) {
            
            if(isStarted) {
                handle(previousButton); //gra jest wystartowana, wpisujemy liczbe, cofamy albo zatwierdzamy
            } else if(isEnded) {
                initGame(); //gra jest skonczona, wlaczamy od nowa
            } else {
                startGame(); //startujemy gre
            }
            previousButton = -1; //ustawiamy, ze klawisz zostal puszczony
        }
    }
    buttonRow = 0; //wyzerowanie wiersza na kolejny odczyt
    buttonColumn = 0; //wyzerowanie kolumny na kolejny odczyt
}

void handle(uint8_t t) {
	if(t < 11 && t > 0) { //wcisniety ktorys z pierwszych dziesieciu klawiszy
		if(position < 4){ //jezeli jest miejsce
			sendData(t+47); //wypisujemy cyfre
			result[position] = t+47; //ustawiamy cyfre na danej pozycji
			position++; //zwiekszamy pozycje na ktorej wpisujemy nastepna cyfre
			LCDposition++;
		}
	} else if(t == 11) { //klawisz cofania
		if(position > 0) { //jezeli juz cos wpisalismy
			clearOneCharacter(); //usuwamy z wyswietlacza ostatni znak
			position--; //zmniejszamy pozycje do wpisywania
			LCDposition--;
			result[position] = 0; //ucinamy stringa na ostatnim miejscu
		}
	} else if(t == 12) { //klawisz zatwierdzania
		checkResult(); //sprawdz czy wynik jest ok
		if(tries < TRIES_COUNT) { //jezeli nie odpowiedzielismy na zadana liczbe pytan dobrze
			playOneTry(); //gramy nastepna runde
		} else {
			if(player == 1) { //jezeli gracz pierwszy skonczyl
				playerOneTime = time; //zapisujemy jego czas
				player = 2; //zmieniamy gracza
				isStarted = 0; //stopujemy gre
				initGame(); //inicjalizujemy gre
                
			} else {
				playerTwoTime = time; //zapisujemy czas drugiego gracz
				endGame(); //konczymy gre
			}
		}
	}
}

uint32_t calculateDistance(uint32_t t1, uint32_t t2) { //wylicza roznice dwoch liczb
	if(t1 > t2) {
		return t1 - t2;
	} else {
		return t2 - t1;
	}
}

void endGame() { //konczenie gry
	isStarted = 0; //zatrzymujemy gre
	isEnded = 1; //konczymy
	player = 1; //zmieniamy gracza do nastepnej rozgrywki na 1
	updateDisplay(9999);
	if (playerOneTime < playerTwoTime) { //wygral gracz pierwszy
		USART_Transmit_String("Wygrywa Gracz 1! Gratulacje!");
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
	} else if(playerOneTime > playerTwoTime) {
		USART_Transmit_String("Wygrywa Gracz 2! Gratulacje!");
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
	} else {
		USART_Transmit_String("Remis! Gratulacje!");
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
	}
    
	USART_Transmit_String("Czas gracza pierwszego: ");
	char tmp[5];
	int i= 0;
	for(i = 0; i < 5; i++) {
		tmp[i] = 0;
	}
	itoa(playerOneTime, tmp, 10);
	USART_Transmit_String(tmp);
	USART_Transmit(10); //nowa linia
	USART_Transmit(13); //powrot karetki
	USART_Transmit_String("Czas gracza drugiego: ");
	for(i = 0; i < 5; i++) {
		tmp[i] = 0;
	}
	itoa(playerTwoTime, tmp, 10);
	USART_Transmit_String(tmp);
	USART_Transmit(10); //nowa linia
	USART_Transmit(13); //powrot karetki
	USART_Transmit_String("Roznica: ");
	for(i = 0; i < 5; i++) {
		tmp[i] = 0;
	}
	itoa(calculateDistance(playerOneTime, playerTwoTime), tmp, 10);
	USART_Transmit_String(tmp);
	USART_Transmit(10); //nowa linia
	USART_Transmit(13); //powrot karetki
    
	playerOneTime = 0; //zerujemy czasy obu graczy
	playerTwoTime = 0;
	clearAndSetZeroPosition(); //czyscimy wyswietlacz i wracamy na poczatek
	sendString("Jeszcze raz?");
	setPosition(0x28); //40 to pozycja pierwsza na dole
	sendString("Wcisnij klawisz");
}

void checkResult() { //sprawdzanie czy to co wpisalismy jest prawidlowe
	uint32_t res = atoi(result);
	if(tries == 0 && failedTries == 0) { //jezeli zaczynamy
		if(player == 1) {
			USART_Transmit(10); //nowa linia
			USART_Transmit(13); //powrot karetki
			USART_Transmit_String("Gracz 1:");
			USART_Transmit(10); //nowa linia
			USART_Transmit(13); //powrot karetki
		} else {
			USART_Transmit(10); //nowa linia
			USART_Transmit(13); //powrot karetki
			USART_Transmit_String("Gracz 2:");
			USART_Transmit(10); //nowa linia
			USART_Transmit(13); //powrot karetki
		}
	}
	if(expectedResult == res) { //zgadlismy
		tries++; //zwiekszamy ilosc prob na ktore dobrze odpowiedzielismy
		USART_Transmit_String("Dobrze!");
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
		USART_Transmit_String("Twoj aktualny czas: ");
		char tmp[5];
		int i= 0;
		for(i = 0; i < 5; i++) {
			tmp[i] = 0;
		}
		itoa(time, tmp, 10);
		USART_Transmit_String(tmp); //wyswietla aktualny czas
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
	} else {
		failedTries++;
		USART_Transmit_String("Zle!");
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
		USART_Transmit_String("Twoj aktualny czas: ");
		char tmp[5];
		int i= 0;
		for(i = 0; i < 5; i++) {
			tmp[i] = 0;
		}
		itoa(time, tmp, 10);
		USART_Transmit_String(tmp);
		USART_Transmit(10); //nowa linia
		USART_Transmit(13); //powrot karetki
		printGoodResult(expectedResult);
		TASK_TABLE[3].timeout = 3000; //uaktywniamy task czyszczenia
		TASK_TABLE[3].timetogo = 3000;
	}
}

void clearBottomLine() {
    setPosition(0x28); //dol wyswietlacza
    int i = 17;
    while(i > 0) {
        sendString(" "); //17 spacji
        i--;
    }
    setPosition(LCDposition); //powrot do aktualnego miejsca
}

void printGoodResult(uint32_t result) {
    setPosition(0x28); //40 to pozycja pierwsza na dole
    sendString("Wynik: ");
    char tmp[5];
    int i= 0;
    for(i = 0; i < 5; i++) {
        tmp[i] = 0;
    }
    itoa(expectedResult, tmp, 10);
    sendString(tmp);
}

void initGame() { //inicjalizacja gry
	time = 0;
	tries = 0;
	failedTries = 0;
	isEnded = 0;
	updateDisplay(time);
	clearAndSetZeroPosition();
	if(player == 1) {
		sendString("Gracz 1");
	} else {
		sendString("Gracz 2");
	}
	setPosition(0x28); //40 to pozycja pierwsza na dole
	sendString("Wcisnij klawisz");
}

void startGame() {
	srand(seed); //ustawiamy seed randoma
	isStarted = 1; //startujemy gre
	clearAndSetZeroPosition();
	playOneTry(); //gramy pierwsza runde
}

void playOneTry(int player) { //gra jedna ture
	position = 0; //ustawiamy na poczatek do wpisywania odpowiedzi
	clearTopAndSetZeroPosition(); //czyscimy wyswietlacz
	int i = 0;
	for(i = 0; i < 5; i++) {
		result[i] = 0;
	}
	randAndDisplay(); //losujemy dzialanie, liczby i wyswietlamy je
}

void clearOneCharacter() {
	sendInstruction(0x04); //zmieniamy kierunek wpisywania na odwrotny
	sendData(32); //wpisujemy spacje
	sendInstruction(0x06); //zmieniamy na pierwotny kierunek
}

void clearTopAndSetZeroPosition() {
	setPosition(0);
	int i = 17;
	while(i > 0) {
		sendString(" ");
		i--;
	}
	setPosition(0);
}

void randAndDisplay() {
	uint8_t operation = rand() % 4;
	if( operation == 0) {
		uint8_t r1 = rand() % 254;
		uint8_t r2 = rand() % 254;
		sendData(r1/100 + 48); //+48 z wylosowanego inta zamienia cyfre na jej kod ascii
		sendData(r1 / 10 % 10 + 48); //zastosowany algorytm dzielenia i modulo 10 wyluskuje kolejne cyfry
		sendData(r1 % 10 + 48);
		sendData(43);
		sendData(r2/100 + 48);
		sendData(r2 / 10 % 10 + 48);
		sendData(r2 % 10 + 48);
		sendData(61);
		expectedResult = r1 + r2;
		LCDposition = 8;
	} else if(operation == 1) {
		uint8_t r1 = rand() % 254;
		uint8_t r2 = rand() % 254;
		if(r1 < r2) { //zeby nie bylo ujemnego wyniku
			uint8_t tmp = r1;
			r1 = r2;
			r2 = tmp;
		}
		sendData(r1/100 + 48);
		sendData(r1 / 10 % 10 + 48);
		sendData(r1 % 10 + 48);
		sendData(45);
		sendData(r2/100 + 48);
		sendData(r2 / 10 % 10 + 48);
		sendData(r2 % 10 + 48);
		sendData(61);
		expectedResult = r1 - r2;
		LCDposition = 8;
	} else if(operation == 2) {
		uint8_t r1 = rand() % 100;
		uint8_t r2 = rand() % 10;
		sendData(r1/10 + 48);
		sendData(r1 % 10 + 48);
		sendData(42);
		sendData(r2 + 48);
		sendData(61);
		expectedResult = r1 * r2;
		LCDposition = 5;
	} else if(operation == 3) {
		uint8_t r1 = rand() % 254;
		uint8_t r2 = rand() % 10;
		while(r2 == 0) { //nie dzielimy przez 0
			r2 = rand() % 10;
		}
		sendData(r1/100 + 48);
		sendData(r1 / 10 % 10 + 48);
		sendData(r1 % 10 + 48);
		sendData(47);
		sendData(r2 + 48);
		sendData(61);
		expectedResult = r1 / r2;
		LCDposition = 6;;
	}
}


void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN) | (1 << TXEN);
	/* Set frame format: 8data, 1stop bit */
	UCSRC = (1<<URSEL)|(0<USBS)|(3<<UCSZ0);
}

unsigned char USART_Receive( void )
{
	/* Sprawdzenie czy cos zostalo otrzymane */
	if ( (UCSRA & (1<<RXC)) ) {
		//updateDisplay(UDR); //wyswietlenie na wyswietlaczu kodu wcisnietego klawisza
		return UDR;
	} else {
		return 0;
	}
}

void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) )
        ;
	/* Put data into buffer, sends the data */
	UDR = data;
}

void USART_Transmit_String(char* str) {
	int i = 0;
	while(str[i] != '\0') {//wysyla kolejne litery napisu
		USART_Transmit(str[i++]);
	}
}

void sendFourBytes(uint8_t val) {
	PORTA |= (1<<4); //Ustawia E na 1
	PORTA = (PORTA & 0xF0) | (val & 0x0F); //Wysyla 4 mlodsze bity val na Port A
	PORTA &= ~(1<<4); //Zeruje E
}

void sendEightBytes(uint8_t val) {
	sendFourBytes(val>>4); //wysyla 4 starsze bity val do portu A
	sendFourBytes(val); // wysyla 4 mlodsze bity val do portu A
	_delay_us(50);
}

void sendInstruction(uint8_t val) {
	PORTA &= ~(1<<5); //ustawia RS na 0 zeby wyslac instrukcje
	sendEightBytes(val); //wysyla kod instrukcji
}

void sendData(uint8_t val) {
	PORTA |= (1<<5); //ustawia RS na 1 zeby wyslac dane
	sendEightBytes(val); //wysyla dane
}

void sendString(char* str) {
	int i = 0;
	while(str[i] != '\0') {//wysyla kolejne litery napisu
		sendData(str[i++]);
	}
}


void initializeLCD() {
	_delay_ms(15);
    
	int i = 0;
	for (i = 0; i < 3; i++) {
		sendFourBytes(3);
		_delay_ms(5);
	}
    
	sendFourBytes(2);
	_delay_ms(2);
	sendInstruction(136);
	_delay_ms(2);
	sendInstruction(8);
	_delay_ms(2);
	sendInstruction(1);
	_delay_ms(2);
	sendInstruction(6);
	_delay_ms(2);
	sendInstruction(0x0F);
	_delay_ms(2);
	sendInstruction(0x80); //ustawia adres DDRAM na 0, czyli pozycje kursora
}

void clearAndSetZeroPosition() {
	_delay_ms(2);
	sendInstruction(0x01); //zerowanie
	_delay_ms(2);
	sendInstruction(0x80); //adres na poczatek
	_delay_ms(2);
}

void setPosition(uint8_t position) {
	_delay_ms(2);
	sendInstruction(position + 0x80); //adres + inskrukcja ustawienia adresu
	_delay_ms(2);
}




int main(void)
{
	setupPORTA();
	setupPORTB();
	setupPORTD();
	setupTimer();
	initializeLCD();
	clearAndSetZeroPosition();
	USART_Init(103);
	addTask(read, 50, 1, 0);
	addTask(incTime, 1000, 1, 1);
	addTask(incSeed, 7, 1, 2);
	addTask(clearBottomLine, 1000, 0, 3);
	initGame();
	return schedule();
}