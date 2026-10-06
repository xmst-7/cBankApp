#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "donnees.h"

/* Filtres d'historique */
#define HIST_TOUS      0
#define HIST_RETRAITS  1
#define HIST_ENVOYES   2
#define HIST_RECUS     3

/* Operations administrateur */
void effectuerRetrait(void);
void effectuerVirement(void);

/* Operations client (index du compte source dans comptes[]) */
void clientRetrait(int idx_compte);
void clientVirement(int idx_compte);

/* Historique */
int  chargerHistorique(int clientId, int filter, int accountFilter, Transaction *out, int max);
void afficherHistorique(int clientId, int filter, int accountFilter);
void afficherHistoriquePourClient(int clientId);

#endif
