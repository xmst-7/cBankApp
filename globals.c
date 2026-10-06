#include <stdio.h>
#include "globals.h"

Client clients[MAX_CLIENTS];
int    nb_clients = 0;
Compte comptes[MAX_COMPTES];
int    nb_comptes = 0;

int next_client_id  = 1;
int next_account_id = 1;

int logged_in_client_id = -1;
int is_admin = 0;

/* Sauvegarde les compteurs d'ID (continuite au redemarrage) */
void saveNextIds(void) {
    FILE *f = fopen(IDS_FILE, "w");
    if (!f) return;
    fprintf(f, "next_client_id=%d\nnext_account_id=%d\n", next_client_id, next_account_id);
    fclose(f);
}

/* Charge les compteurs d'ID (valeurs par defaut : 1) */
void loadNextIds(void) {
    next_client_id = 1;
    next_account_id = 1;
    FILE *f = fopen(IDS_FILE, "r");
    if (!f) return;
    int c, a;
    if (fscanf(f, "next_client_id=%d next_account_id=%d", &c, &a) == 2) {
        if (c >= 1) next_client_id = c;
        if (a >= 1) next_account_id = a;
    }
    fclose(f);
}
