#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "operations.h"
#include "globals.h"
#include "utils.h"
#include "menu.h"
#include "data.h"

/* ================================================================== */
/*  Retrait (logique commune admin / client)                           */
/* ================================================================== */

static void traiterRetrait(int ci, int ai, int admin) {
    char buf[64];
    float montant = 0;

    for (;;) {
        clearScreen();
        printHeader(admin ? "RETRAIT D'ARGENT" : "RETRAIT");
        if (admin) printf("%s  Client: %s %s%s\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
        printf("%s  Compte: %d | Solde: %.2f DH%s\n\n", COLOR_WARNING, comptes[ai].id_compte, comptes[ai].solde, COLOR_RESET);

        readLine("  Montant a retirer (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }

        /* Validation : > 0, <= plafond (RG-03), <= solde (RG-02) */
        if (!parseAmount(buf, &montant) || montant <= 0) {
            printError("Montant invalide (doit etre superieur a 0).");
        } else if (montant > PLAFOND) {
            printError("Montant superieur au plafond autorise (5000 DH par operation).");
        } else if (montant > comptes[ai].solde) {
            printError("Solde insuffisant.");
        } else break;
        sleepMs(1800);
    }

    clearScreen();
    printHeader("CONFIRMATION RETRAIT");
    if (admin) printf("%s  Client: %s %s%s\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
    printf("%s  Compte: %d\n  Montant: %.2f DH\n  Solde actuel: %.2f DH\n  Solde apres retrait: %.2f DH%s\n",
           COLOR_WARNING, comptes[ai].id_compte, montant, comptes[ai].solde, comptes[ai].solde - montant, COLOR_RESET);
    printf("\n%s  Confirmer ce retrait?%s\n", COLOR_WARNING, COLOR_RESET);

    if (demanderConfirmationAvecFleches() == 'O') {
        char date[20];
        getCurrentDateTime(date);
        comptes[ai].solde -= montant;
        saveAccount();
        saveWithdrawal(clients[ci].id_client, comptes[ai].id_compte, montant, date);
        printf("\n");
        printSuccess("Retrait effectue avec succes!");
        waitForEnter();
    } else {
        displayCancelNotification();
    }
}

void effectuerRetrait(void) {
    char buf[64];
    int ci;

    for (;;) {
        clearScreen();
        printHeader("RETRAIT D'ARGENT");
        readLine("  ID Client (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (parseInt(buf, &id) && (ci = trouverIndexClient(id)) != -1) break;
        printError("Client introuvable.");
        sleepMs(1500);
    }

    clearScreen();
    printHeader("RETRAIT - SELECTION COMPTE");
    printf("%s  Client: %s %s%s\n\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
    int ai = selectClientAccount(clients[ci].id_client, 0);
    if (ai == -1) { sleepMs(1200); displayCancelNotification(); return; }

    traiterRetrait(ci, ai, 1);
}

void clientRetrait(int idx_compte) {
    int ci = trouverIndexClient(logged_in_client_id);
    if (ci == -1 || idx_compte < 0) return;
    traiterRetrait(ci, idx_compte, 0);
}

/* ================================================================== */
/*  Virement (logique commune admin / client)                          */
/* ================================================================== */

/* Etapes : compte destinataire -> montant -> confirmation -> execution.
   ai = index du compte source. */
static void traiterVirement(int ci, int ai, int admin) {
    char buf[64];
    int di = -1;
    float montant = 0;

    /* 1) Compte destinataire (doit exister et etre different de la source) */
    for (;;) {
        clearScreen();
        printHeader("VIREMENT - COMPTE DESTINATION");
        readLine("  Entrez l'ID du compte destinataire (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (!parseInt(buf, &id) || (di = trouverIndexCompte(id)) == -1) {
            printError("Compte destinataire introuvable.");
        } else if (di == ai) {
            printError("Le compte destinataire doit etre different du compte source.");
        } else break;
        sleepMs(1800);
    }

    /* 2) Montant : > 0, <= plafond, <= solde source */
    for (;;) {
        clearScreen();
        printHeader("VIREMENT D'ARGENT");
        if (admin) printf("%s  Client SOURCE: %s %s%s\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
        printf("%s  Compte SOURCE: %d (Solde: %.2f DH)\n  Compte DESTINATION: %d%s\n\n", COLOR_WARNING,
               comptes[ai].id_compte, comptes[ai].solde, comptes[di].id_compte, COLOR_RESET);

        readLine("  Montant a virer (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }

        if (!parseAmount(buf, &montant) || montant <= 0) {
            printError("Montant invalide (doit etre superieur a 0).");
        } else if (montant > PLAFOND) {
            printError("Montant superieur au plafond autorise (5000 DH par operation).");
        } else if (montant > comptes[ai].solde) {
            printError("Solde insuffisant.");
        } else break;
        sleepMs(1800);
    }

    /* 3) Confirmation */
    clearScreen();
    printHeader("CONFIRMATION VIREMENT");
    if (admin) printf("%s  Client SOURCE: %s %s%s\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
    printf("%s  Compte SOURCE: %d (Solde: %.2f DH)\n  Compte DESTINATION: %d (Solde: %.2f DH)\n  Montant: %.2f DH\n  Solde SOURCE apres: %.2f DH%s\n",
           COLOR_WARNING, comptes[ai].id_compte, comptes[ai].solde, comptes[di].id_compte,
           comptes[di].solde, montant, comptes[ai].solde - montant, COLOR_RESET);
    printf("\n%s  Confirmer ce virement?%s\n", COLOR_WARNING, COLOR_RESET);

    if (demanderConfirmationAvecFleches() != 'O') { displayCancelNotification(); return; }

    /* 4) Execution : debit source / credit destinataire, trace des deux cotes */
    char date[20];
    getCurrentDateTime(date);
    comptes[ai].solde -= montant;
    comptes[di].solde += montant;
    saveAccount();
    saveTransferSent(comptes[ai].id_client, comptes[ai].id_compte, comptes[di].id_compte, montant, date);
    saveTransferReceived(comptes[di].id_client, comptes[ai].id_compte, comptes[di].id_compte, montant, date);

    printf("\n");
    printSuccess("Virement effectue avec succes!");
    waitForEnter();
}

void effectuerVirement(void) {
    char buf[64];
    int ci;

    for (;;) {
        clearScreen();
        printHeader("VIREMENT D'ARGENT");
        readLine("  ID Client SOURCE (Emetteur, ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (parseInt(buf, &id) && (ci = trouverIndexClient(id)) != -1) break;
        printError("Client introuvable.");
        sleepMs(1500);
    }

    clearScreen();
    printHeader("VIREMENT - COMPTE SOURCE");
    printf("%s  Client: %s %s%s\n\n", COLOR_WARNING, clients[ci].nom, clients[ci].prenom, COLOR_RESET);
    int ai = selectClientAccount(clients[ci].id_client, 0);
    if (ai == -1) { sleepMs(1200); displayCancelNotification(); return; }

    traiterVirement(ci, ai, 1);
}

void clientVirement(int idx_compte) {
    int ci = trouverIndexClient(logged_in_client_id);
    if (ci == -1 || idx_compte < 0) return;
    traiterVirement(ci, idx_compte, 0);
}

/* ================================================================== */
/*  Historique                                                         */
/* ================================================================== */

static long parseDateTime(const char *s) {
    struct tm t;
    int d, m, y, H, M, S;
    memset(&t, 0, sizeof t);
    if (sscanf(s, "%d/%d/%d %d:%d:%d", &d, &m, &y, &H, &M, &S) != 6) return 0;
    t.tm_mday = d; t.tm_mon = m - 1; t.tm_year = y - 1900;
    t.tm_hour = H; t.tm_min = M; t.tm_sec = S; t.tm_isdst = -1;
    return (long)mktime(&t);
}

/* Charge et fusionne retraits + virements d'un client.
   accountFilter > 0 : ne garde que les lignes concernant ce compte. */
int chargerHistorique(int clientId, int filter, int accountFilter, Transaction *out, int max) {
    int n = 0;
    char path[160], line[256];
    FILE *f;

    if (filter == HIST_TOUS || filter == HIST_RETRAITS) {
        snprintf(path, sizeof path, "%s/client_%d.txt", RETRAITS_DIR, clientId);
        if ((f = fopen(path, "r")) != NULL) {
            while (n < max && fgets(line, sizeof line, f)) {
                int acc; float m; char d[32];
                if (sscanf(line, "%d,%f,%19[^\r\n]", &acc, &m, d) != 3) continue;
                if (accountFilter && acc != accountFilter) continue;
                Transaction *t = &out[n];
                memset(t, 0, sizeof *t);
                t->kind = T_RETRAIT; t->compte_from = acc; t->montant = m;
                strcpy(t->date, d); t->ts = parseDateTime(d); t->seq = n;
                n++;
            }
            fclose(f);
        }
    }

    if (filter == HIST_TOUS || filter == HIST_ENVOYES || filter == HIST_RECUS) {
        snprintf(path, sizeof path, "%s/client_%d.txt", VIREMENTS_DIR, clientId);
        if ((f = fopen(path, "r")) != NULL) {
            while (n < max && fgets(line, sizeof line, f)) {
                int from, to; float m; char d[32], tag[16];
                if (sscanf(line, "%d,%d,%f,%19[^,],%7[^\r\n]", &from, &to, &m, d, tag) != 5) continue;
                int kind = (strcmp(tag, "ENVOYE") == 0) ? T_ENVOYE : T_RECU;
                if (filter == HIST_ENVOYES && kind != T_ENVOYE) continue;
                if (filter == HIST_RECUS   && kind != T_RECU)   continue;
                if (accountFilter && !((kind == T_ENVOYE && from == accountFilter) ||
                                       (kind == T_RECU   && to   == accountFilter))) continue;
                Transaction *t = &out[n];
                memset(t, 0, sizeof *t);
                t->kind = kind; t->compte_from = from; t->compte_to = to; t->montant = m;
                strcpy(t->date, d); t->ts = parseDateTime(d); t->seq = n;
                n++;
            }
            fclose(f);
        }
    }
    return n;
}

static int cmpTransDesc(const void *a, const void *b) {
    const Transaction *x = a, *y = b;
    if (x->ts != y->ts) return (y->ts > x->ts) ? 1 : -1;
    return y->seq - x->seq;                    /* plus recent en premier */
}

void afficherHistorique(int clientId, int filter, int accountFilter) {
    static Transaction tab[MAX_TRANSACTIONS];
    int n = chargerHistorique(clientId, filter, accountFilter, tab, MAX_TRANSACTIONS);
    qsort(tab, (size_t)n, sizeof(Transaction), cmpTransDesc);

    printf("%s  %-20s| %-15s| %-13s| %s%s\n", COLOR_WARNING,
           "Date", "Type", "Montant", "Sender/Receiver", COLOR_RESET);
    printf("  ");
    for (int i = 0; i < 70; i++) putchar('=');
    printf("\n");

    for (int i = 0; i < n; i++) {
        char who[32], type[20];
        switch (tab[i].kind) {
            case T_RETRAIT: strcpy(type, "Retrait");       snprintf(who, sizeof who, "%d", tab[i].compte_from); break;
            case T_ENVOYE:  strcpy(type, "Virement env."); snprintf(who, sizeof who, "Receiver: %d", tab[i].compte_to); break;
            default:        strcpy(type, "Virement recu"); snprintf(who, sizeof who, "Sender: %d", tab[i].compte_from); break;
        }
        char mont[24];
        snprintf(mont, sizeof mont, "%.2f DH", tab[i].montant);
        printf("  %-20s| %-15s| %-13s| %s\n", tab[i].date, type, mont, who);
    }
    printf("\n%s  Total: %d transaction(s)%s\n", COLOR_WARNING, n, COLOR_RESET);
}

void afficherHistoriquePourClient(int clientId) {
    char buf[32];
    int filtre = -1;

    clearScreen();
    printHeader("HISTORIQUE DES OPERATIONS");
    printf("%s  Filtres disponibles:\n  1 - Retraits\n  2 - Virements envoyes\n  3 - Virements recus\n  4 - Afficher tous%s\n\n",
           COLOR_WARNING, COLOR_RESET);

    for (;;) {
        readLine("  Selectionnez un filtre (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        if (parseInt(buf, &filtre) && filtre >= 1 && filtre <= 4) break;
        printError("Filtre invalide (1 a 4).");
    }

    clearScreen();
    printHeader("HISTORIQUE DES OPERATIONS");
    printf("\n");
    afficherHistorique(clientId, filtre == 4 ? HIST_TOUS : filtre, 0);
    waitForEnter();
}
