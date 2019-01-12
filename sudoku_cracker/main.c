/*
 * Sudoku Cracker : Cracka un codice sudoku velocemente
 * Copyright (C) 2018   <robert@battlestation>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/* includi le librerie richieste */
#include <ncurses.h>		/* libreria per la TUI */
#include <stdlib.h>		/* per exit() e atexit() */
#include <time.h>		/* per il tempo */
#include <stdio.h>		/* per la gestione dei file */
#include <string.h>		/* per memcpy() */

/* definisci il valore di ritorno */
#define EXIT_OK 0		/* tutto con successo */
#define EXIT_CANT_CRACK 1	/* errore nel cracking del puzzle */
#define EXIT_RESIZED_WINDOW 2	/* finestra ha cambiato dimensione */

/* definisci altre funzionalità */
#define INPUT_MOVIMENTO 1
#define INPUT_CRACCA 2
#define INPUT_CANCELLA 4
#define INPUT_CHIUDI 8

/* struttura dei dati globali */
struct dati_globali {
	int tabella[9][9];	/* tabella sudoku */
	int posizione[2];	/* posizione nella tabella */
	int max_x, max_y;	/* dimensioni dello schermo */
};

/* funzioni del programma */
void configura_ncurses(struct dati_globali *dati);
void disegna_tabella(struct dati_globali *dati);
void wendwin(void);
void disegna_bordo(struct dati_globali *dati);
int input(struct dati_globali *dati);
int cracca_codice(struct dati_globali *dati);
int soluzione_casella(struct dati_globali *dati);
int nella_regione(int numero, struct dati_globali *dati);



/* funzioni del gioco */
void configura_ncurses(struct dati_globali *dati)
{				            /* configura la TUI */
	initscr();              /* inizializza ncurses */
	noecho();               /* nascondi i tasti premuti */
	raw();                  /* disattiva CTRL-C */
	keypad(stdscr, TRUE);   /* Attiva F1, F2... */
	atexit(wendwin);        /* chiama endwin() prima di exit() */
	start_color();          /* attiva i colori del terminale */
    curs_set(1);            /* attiva il cursore in caso esso sia disattivato */

	getmaxyx(stdscr, dati->max_y, dati->max_x);
	if (dati->max_y < 20 || dati->max_x < 44)
		exit(EXIT_RESIZED_WINDOW);
	
	/* imposta alcuni colori */
	init_pair(1, COLOR_WHITE, COLOR_BLUE);  /* la coppia 1 ha testo bianco su sfondo blu */
	init_pair(2, COLOR_BLACK, COLOR_WHITE); /* la coppia 2 ha testo nero su sfondo bianco */
	attron(COLOR_PAIR(1));
	wbkgd(stdscr, COLOR_PAIR(1));
	refresh();
}

void disegna_tabella(struct dati_globali *dati)
{				/* disegna la tabella sudoku */
	
	/* imposta i colori */
	attron(COLOR_PAIR(1));

	/* rileva le dimensioni del terminale */
	int a, b;
	getmaxyx(stdscr, a, b);

	if (a != dati->max_y || b != dati->max_x) {
		exit(EXIT_RESIZED_WINDOW);
	}
	
	/* disegna i lati */
	for (int i = 0; i < 9; i++) {
		mvprintw(i * 2, dati->max_x / 2 - 19, "+---+---+---+---+---+---+---+---+---+");
		mvprintw(i*2+1, dati->max_x / 2 - 19, "|   |   |   |   |   |   |   |   |   |");
	}

	mvprintw(18, dati->max_x / 2 - 19, "+---+---+---+---+---+---+---+---+---+");

        /* disegna i numeri */
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (dati->tabella[i][j] != 0)
				mvprintw(i*2+1, j * 4 + (dati->max_x / 2 - 17), "%d", dati->tabella[i][j]);
		}
	}
	
	disegna_bordo(dati);
	move(dati->posizione[1]*2+1, dati->posizione[0]*4 + (dati->max_x / 2 - 17));
	refresh();
}

void disegna_bordo(struct dati_globali *dati)
{				/* disegna la barra di stato */
	attron(COLOR_PAIR(2));

	for (int i = 0; i < dati->max_x; i++)
		mvprintw(dati->max_y - 1, i, " ");
	
	mvprintw(dati->max_y - 1, dati->max_x / 2 - 22, "F1: CRACK CODE    F2: CLEAR ALL    F3: EXIT");
}

void wendwin(void)
{				/* wrapper di endwin() */
	endwin();		/* termina NCURSES */
}


int input(struct dati_globali *dati)
{				/* gestisci secondo l'input */
	int fine = 0, i = 0;
	while (fine == 0) {
		i = getch();
		switch (i) {
            case KEY_UP: case 'w': case 'W': case 'k': case 'K':
			(dati->posizione[1] > 0) ? dati->posizione[1]-- : printw("");
			fine = INPUT_MOVIMENTO;
			break;

        case KEY_DOWN: case 's': case 'S': case 'j': case 'J':
			(dati->posizione[1] < 8) ? dati->posizione[1]++ : printw("");
			fine = INPUT_MOVIMENTO;
			break;

        case KEY_LEFT: case 'a': case 'A': case 'h': case 'H':
			(dati->posizione[0] > 0) ? dati->posizione[0]-- : printw("");
			fine = INPUT_MOVIMENTO;
			break;

        case KEY_RIGHT: case 'd': case 'D': case 'l': case 'L':
			(dati->posizione[0] < 8) ? dati->posizione[0]++ : printw("");
			fine = INPUT_MOVIMENTO;
			break;

		case KEY_F(1):
			fine = INPUT_CRACCA;
			break;

		case KEY_F(2):
			fine = INPUT_CANCELLA;
			break;

		case KEY_F(3):
			fine = INPUT_CHIUDI;
			break;
		}

		if (0x31 <= i && i <= 0x39) {
			dati->tabella[dati->posizione[1]][dati->posizione[0]] = i - 0x30;
			fine = INPUT_MOVIMENTO;
		}

		else if (i == 0x30 || i == 0x20) {
			dati->tabella[dati->posizione[1]][dati->posizione[0]] = 0;
			fine = INPUT_MOVIMENTO;
		}
	}

	return fine;
}

int cracca_codice(struct dati_globali *dati)
{				/* beh... cracca il sudoku (o almeno ci prova) */
	int finish = 0;     /* fine del codice */
    curs_set(0);        /* nascondi il cursore */
    int oldpos[2];
    memcpy(oldpos, dati->posizione, sizeof oldpos);
	while (finish < 81) {
		//fprintf(log, "[i] turno %d\n", finish);
		for (int i = 0; i < 9; i++) { /* riga */
			for (int j = 0; j < 9; j++) { /* colonna */
				/* se la casella è occupata vai alla prossima casella */
				dati->posizione[0] = j;
				dati->posizione[1] = i;
				
				if (dati->tabella[j][i] != 0) {
					//fprintf(log, "[i] collisione nella casella [%d][%d]\n", i, j);
					continue;
				}

				/* la casella è vuota, verifica e imposta la soluzione se possibile */
				dati->tabella[j][i] = soluzione_casella(dati);
			}
		}

		finish++;
	}

    dati->posizione[0] = oldpos[0];
    dati->posizione[1] = oldpos[1];
    curs_set(1);
    disegna_tabella(dati);
    return 0;
}

int soluzione_casella(struct dati_globali *dati)
{				/* restituisce il valore unico della cella
				 * oppure 0 se ce ne sono di più.
				 */
	int numero = 0;
	
	for (int i = 1; i < 10; i++) {
		if (!nella_regione(i, dati)) {
			if (numero == 0) {
				numero = i;
				//fprintf(log, "[+] numero: %d\n", numero);
			}
			
			else {
				//fprintf(log, "[-] Numero azzerato... %d\n", i);
				return 0;
			}
		}
	}

	return numero;
}

int nella_regione(int numero, struct dati_globali *dati)
{				/* verifica se il numero << numero >> è nella regione,
				 * colonna o riga. Ritorna 1 se presente, 0 se assente.
				 */

    /* per le caselle quadrate */
    int riga = (dati->posizione[1] / 3) * 3;
    int colonna = (dati->posizione[0] / 3) * 3;

    for (int i = riga; i < riga + 3; i++) {
        for (int j = colonna; j < colonna + 3; j++)
            if (dati->tabella[j][i] == numero)
                return 1;
    }

    /* per le linee orizzontali / verticali */
    for (int i = 0; i < 9; i++) {
        if (dati->tabella[dati->posizione[0]][i] == numero)
            return 1;

        if (dati->tabella[i][dati->posizione[1]] == numero)
            return 1;
    }

    return 0;
}

/* funzione main() */
int main(void)
{
	/* crea variabili */
	struct dati_globali dati;
	int fine_gioco = -1;
	int stato;		/* per il valore di ritorno di input() */
    FILE *tmp = fopen("tmp.txt", "w");
    int fine = 0;
	
	/* imposta tabella a 0 */
	memset(dati.tabella, 0, sizeof dati.tabella);
	memset(dati.posizione, 0, sizeof dati.posizione);
	
	/* inizializza ncurses */
	configura_ncurses(&dati);
	disegna_tabella(&dati);

	while (fine_gioco == -1) {
		stato = input(&dati);
		switch (stato)
		{
		case INPUT_MOVIMENTO:
			disegna_tabella(&dati);
			break;

		case INPUT_CRACCA:
            fprintf(tmp, "[+] Hacking avviato!\n");
            fine = 1;
			cracca_codice(&dati);
            fprintf(tmp, "[+] Hacking terminato!\n");
			break;

		case INPUT_CANCELLA: /* elimina il contenuto della tabella */
			memset(dati.tabella, 0, sizeof dati.tabella);
            disegna_tabella(&dati);
			break;

		case INPUT_CHIUDI:
			fine_gioco = 1;
			break;
		}
	}
	
	/* esci con successo */
	endwin();
	
    fclose(tmp);

    printf("Fine: %d\n", fine);
	exit(EXIT_OK);
}
