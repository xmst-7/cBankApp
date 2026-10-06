#include <stdio.h>
#include <string.h>

#include "comptes.h"
#include "globals.h"
#include "utils.h"
#include "menu.h"
#include "data.h"
#include "operations.h"

/* Mettre a 1 pour exiger un solde nul avant la fermeture d'un compte
   (regle decrite dans le tableau 7). La figure 28 du rapport montre
   une fermeture avec solde > 0 apres confirmation : valeur par defaut 0. */
#define EXIGER_SOLDE_NUL 0

void ajouterCompte(void) {
    char buf[64];
    int idx;

    while (1) {
        clearScreen();
        printHeader("CREER UN COMPTE");
        readLine("  ID du Client proprietaire (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (parseInt(buf, &id) && (idx = trouverIndexClient(id)) != -1) break;
        printError("Client introuvable.");
        sleepMs(1500);
    }

    if (nb_comptes >= MAX_COMPTES) {
        printError("Capacite maximale de comptes atteinte.");
        waitForEnter();
        return;
    }

    clearScreen();
    printHeader("CREATION DE COMPTE");
    printf("%s  Creation d'un compte pour %s %s (ID: %d)%s\n\n", COLOR_WARNING,
           clients[idx].nom, clients[idx].prenom, clients[idx].id_client, COLOR_RESET);
    printf("  ID Compte (auto): %d\n", next_account_id);

    float solde;
    for (;;) {
        readLine("  Solde initial (Min 1000.00 DH, ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        if (!parseAmount(buf, &solde)) { printError("Montant invalide."); continue; }
        if (solde < SOLDE_MIN) { printError("Le solde initial doit etre >= 1000.00 DH."); continue; }
        break;
    }

    Compte c;
    memset(&c, 0, sizeof c);
    c.id_compte = next_account_id;
    c.id_client = clients[idx].id_client;
    c.solde = solde;
    getCurrentDate(c.date_ouverture);

    comptes[nb_comptes++] = c;
    next_account_id++;
    saveAccount();
    saveNextIds();

    printf("\n");
    printSuccess("Compte cree avec succes!");
    waitForEnter();
}

void consulterComptes(void) {
    char buf[64];
    clearScreen();
    printHeader("LISTE DES COMPTES");

    printf("%s  %-10s| %-10s| %-14s| %s%s\n", COLOR_WARNING,
           "ID Compte", "ID Client", "Solde (DH)", "Date Ouverture", COLOR_RESET);
    printf("  ");
    for (int i = 0; i < 56; i++) putchar('-');
    printf("\n");
    for (int i = 0; i < nb_comptes; i++)
        printf("  %-10d| %-10d| %-14.2f| %s\n", comptes[i].id_compte, comptes[i].id_client,
               comptes[i].solde, comptes[i].date_ouverture);
    printf("\n  Total: %d compte(s)\n", nb_comptes);

    readLine("\n  ID d'un compte pour voir son historique (ENTREE pour retour): ", buf, sizeof buf);
    int id;
    if (parseInt(buf, &id)) {
        if (trouverIndexCompte(id) != -1) afficherHistoriquePourCompte(id);
        else { printError("Compte introuvable."); waitForEnter(); }
        return;
    }
    clearScreen();
}

void fermerCompte(void) {
    char buf[64];
    int idx;

    while (1) {
        clearScreen();
        printHeader("FERMER UN COMPTE");
        readLine("  ID Compte a fermer (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (parseInt(buf, &id) && (idx = trouverIndexCompte(id)) != -1) break;
        printError("Compte introuvable.");
        sleepMs(1500);
    }

#if EXIGER_SOLDE_NUL
    if (comptes[idx].solde > 0.0f) {
        printf("\n");
        printError("Fermeture impossible : le solde doit etre egal a 0.");
        waitForEnter();
        return;
    }
#endif

    clearScreen();
    printHeader("CONFIRMATION FERMETURE");
    printf("  Informations du compte:\n");
    printf("  - ID Compte: %d\n  - ID Client: %d\n  - Solde: %.2f DH\n  - Date ouverture: %s\n",
           comptes[idx].id_compte, comptes[idx].id_client, comptes[idx].solde, comptes[idx].date_ouverture);
    if (comptes[idx].solde > 0.0f)
        printf("%s  Attention : le solde restant sera perdu a la fermeture.%s\n", COLOR_WARNING, COLOR_RESET);
    printf("\n%s  Etes-vous sur de vouloir fermer ce compte?%s\n", COLOR_WARNING, COLOR_RESET);

    if (demanderConfirmationAvecFleches() == 'O') {
        deleteAccountFile(comptes[idx].id_compte);      /* reecrit le fichier sans ce compte */
        for (int i = idx; i < nb_comptes - 1; i++) comptes[i] = comptes[i + 1];
        nb_comptes--;
        printf("\n");
        printSuccess("Compte ferme avec succes!");
        waitForEnter();
    } else {
        displayCancelNotification();
    }
}

/* Historique d'un compte : on lit les fichiers du proprietaire, filtres sur ce compte */
void afficherHistoriquePourCompte(int id_compte) {
    int idx = trouverIndexCompte(id_compte);
    if (idx == -1) { printError("Compte introuvable."); waitForEnter(); return; }
    clearScreen();
    printHeader("HISTORIQUE DU COMPTE");
    printf("  Compte: %d (Client %d)\n\n", id_compte, comptes[idx].id_client);
    afficherHistorique(comptes[idx].id_client, HIST_TOUS, id_compte);
    waitForEnter();
}
