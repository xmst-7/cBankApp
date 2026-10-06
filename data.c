#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #define MKDIR(p) mkdir((p), 0755)
#endif

#include "data.h"
#include "globals.h"
#include "donnees.h"

/* Cree l'arborescence data/ si elle n'existe pas */
void initializeDataFolders(void) {
    MKDIR(DATA_DIR);
    MKDIR(CLIENTS_DIR);
    MKDIR(RETRAITS_DIR);
    MKDIR(VIREMENTS_DIR);
}

/* Charge clients + comptes en memoire et recale les compteurs d'ID */
void loadAllDataFromFiles(void) {
    char line[256];
    nb_clients = 0;
    nb_comptes = 0;

    FILE *f = fopen(CLIENTS_FILE, "r");
    if (f) {
        while (nb_clients < MAX_CLIENTS && fgets(line, sizeof line, f)) {
            Client c;
            memset(&c, 0, sizeof c);
            if (sscanf(line, "%d,%49[^,],%49[^,],%49[^,],%14[^,],%19[^\r\n]",
                       &c.id_client, c.nom, c.prenom, c.profession,
                       c.num_tel, c.code_pin) == 6)
                clients[nb_clients++] = c;
        }
        fclose(f);
    }

    f = fopen(COMPTES_FILE, "r");
    if (f) {
        while (nb_comptes < MAX_COMPTES && fgets(line, sizeof line, f)) {
            Compte c;
            memset(&c, 0, sizeof c);
            if (sscanf(line, "%d,%d,%f,%10[^\r\n]",
                       &c.id_compte, &c.id_client, &c.solde, c.date_ouverture) == 4)
                comptes[nb_comptes++] = c;
        }
        fclose(f);
    }

    /* Securite : les compteurs doivent depasser les ID existants */
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].id_client >= next_client_id) next_client_id = clients[i].id_client + 1;
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_compte >= next_account_id) next_account_id = comptes[i].id_compte + 1;
}

/* Format CSV : ID,Nom,Prenom,Profession,Tel,PIN */
void saveClient(void) {
    FILE *f = fopen(CLIENTS_FILE, "w");
    if (!f) return;
    for (int i = 0; i < nb_clients; i++)
        fprintf(f, "%d,%s,%s,%s,%s,%s\n", clients[i].id_client, clients[i].nom,
                clients[i].prenom, clients[i].profession, clients[i].num_tel,
                clients[i].code_pin);
    fclose(f);
}

/* Format CSV : IDCompte,IDClient,Solde,DateOuverture */
void saveAccount(void) {
    FILE *f = fopen(COMPTES_FILE, "w");
    if (!f) return;
    for (int i = 0; i < nb_comptes; i++)
        fprintf(f, "%d,%d,%.2f,%s\n", comptes[i].id_compte, comptes[i].id_client,
                comptes[i].solde, comptes[i].date_ouverture);
    fclose(f);
}

/* Ajout (mode "a") : IDCompte,Montant,DateHeure */
void saveWithdrawal(int clientId, int accountId, float amount, const char *date) {
    char filename[150];
    MKDIR(RETRAITS_DIR);
    snprintf(filename, sizeof filename, "%s/client_%d.txt", RETRAITS_DIR, clientId);
    FILE *file = fopen(filename, "a");
    if (!file) return;
    fprintf(file, "%d,%.2f,%s\n", accountId, amount, date);
    fclose(file);
}

static void appendTransfer(int clientId, int from, int to, float amount,
                           const char *date, const char *tag) {
    char filename[150];
    MKDIR(VIREMENTS_DIR);
    snprintf(filename, sizeof filename, "%s/client_%d.txt", VIREMENTS_DIR, clientId);
    FILE *file = fopen(filename, "a");
    if (!file) return;
    fprintf(file, "%d,%d,%.2f,%s,%s\n", from, to, amount, date, tag);
    fclose(file);
}

void saveTransferSent(int clientId, int from, int to, float amount, const char *date) {
    appendTransfer(clientId, from, to, amount, date, "ENVOYE");
}

void saveTransferReceived(int clientId, int from, int to, float amount, const char *date) {
    appendTransfer(clientId, from, to, amount, date, "RECU");
}

void deleteClientFolder(int clientId) {
    char filename[150];
    snprintf(filename, sizeof filename, "%s/client_%d.txt", RETRAITS_DIR, clientId);
    remove(filename);
    snprintf(filename, sizeof filename, "%s/client_%d.txt", VIREMENTS_DIR, clientId);
    remove(filename);
}

/* Reecrit comptes.txt sans le compte supprime */
void deleteAccountFile(int accountId) {
    FILE *f = fopen(COMPTES_FILE, "w");
    if (!f) return;
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_compte == accountId) continue;
        fprintf(f, "%d,%d,%.2f,%s\n", comptes[i].id_compte, comptes[i].id_client,
                comptes[i].solde, comptes[i].date_ouverture);
    }
    fclose(f);
}
