#ifndef GLOBALS_H
#define GLOBALS_H
#include "donnees.h"

extern Client clients[MAX_CLIENTS];
extern int    nb_clients;
extern Compte comptes[MAX_COMPTES];
extern int    nb_comptes;

extern int next_client_id;
extern int next_account_id;

extern int logged_in_client_id;  /* -1 si personne / admin */
extern int is_admin;             /* 1 si l'administrateur est connecte */

void saveNextIds(void);
void loadNextIds(void);

#endif
