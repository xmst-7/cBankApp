#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#include "menu.h"
#include "utils.h"
#include "globals.h"
#include "clients.h"
#include "comptes.h"
#include "operations.h"
#include "data.h"

/* ================================================================== */
/*  Affichage de base                                                  */
/* ================================================================== */

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

static void printLine(void) {
    for (int i = 0; i < LINE_WIDTH; i++) putchar('=');
    putchar('\n');
}

/* Banniere standardisee (91 separateurs, cyan) */
void printHeader(const char *title) {
    const char *app = "GESTION BANCAIRE - APPLICATION MASTER";
    int pad = (LINE_WIDTH - (int)strlen(app)) / 2;
    printf("\n%s", COLOR_INFO);
    printLine();
    printf("%*s%s\n", pad, "", app);
    printLine();
    printf(" %s\n", title);
    printLine();
    printf("%s\n", COLOR_RESET);
}

void printFooter(void) {
    printf("\n%s", COLOR_INFO);
    printLine();
    printf("  Appuyez sur ENTREE pour continuer...\n");
    printLine();
    printf("%s", COLOR_RESET);
    fflush(stdout);
}

void printBox(const char *msg)     { printf("%s  >> %s%s\n", COLOR_INFO, msg, COLOR_RESET); }
void printError(const char *msg)   { printf("%s  [!] %s%s\n", COLOR_ERROR, msg, COLOR_RESET); }
void printSuccess(const char *msg) { printf("%s  [OK] %s%s\n", COLOR_SUCCESS, msg, COLOR_RESET); }

void waitForEnter(void) {
    printFooter();
    for (;;) {
        int k = readKey();
        if (k == KEY_ENTER || k == KEY_ESC) break;
    }
    clearScreen();
}

/* ================================================================== */
/*  Navigation avec les fleches                                        */
/* ================================================================== */

/* Retourne l'index choisi, -1 si ESC, -2 si '?' (aide) */
int menuAvecFleches(const char *options[], int n, int allowHelp) {
    int sel = 0;
    for (;;) {
        for (int i = 0; i < n; i++) {
            if (i == sel) printf("%s  > %s%s\033[K\n", COLOR_SELECT, options[i], COLOR_RESET);
            else          printf("%s    %s%s\033[K\n", COLOR_NORMAL, options[i], COLOR_RESET);
        }
        fflush(stdout);
        int k = readKey();
        if      (k == KEY_UP)    sel = (sel - 1 + n) % n;     /* wrapping */
        else if (k == KEY_DOWN)  sel = (sel + 1) % n;
        else if (k == KEY_ENTER) return sel;
        else if (k == KEY_ESC)   return -1;
        else if (k == '?' && allowHelp) return -2;
        printf("\033[%dA", n);                                /* remonte et redessine */
    }
}

char demanderConfirmationAvecFleches(void) {
    const char *options[] = {"Oui", "Non"};
    printf("\n");
    int r = menuAvecFleches(options, 2, 0);
    return (r == 0) ? 'O' : 'N';
}

/* style 0 : liste simple (admin) / style 1 : tableau (client) */
int selectClientAccount(int id_client, int style) {
    int idx[MAX_COMPTES], n = 0;
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_client == id_client) idx[n++] = i;

    if (n == 0) { printError("Ce client n'a aucun compte."); return -1; }

    if (style == 1) {
        printf("%s  %-7s| %-13s| %s%s\n", COLOR_WARNING, "ID Cpt", "Solde (DH)", "Date", COLOR_RESET);
        printf("  ");
        for (int i = 0; i < 36; i++) putchar('=');
        printf("\n");
    } else {
        printf("  Selectionnez un compte (fleches + Entree, ESC pour annuler):\n\n");
    }

    int sel = 0;
    for (;;) {
        for (int i = 0; i < n; i++) {
            const Compte *c = &comptes[idx[i]];
            const char *col = (i == sel) ? COLOR_SELECT : COLOR_NORMAL;
            if (style == 1)
                printf("%s%s %-6d| %-13.2f| %s%s\033[K\n", col, (i == sel) ? "  >" : "   ",
                       c->id_compte, c->solde, c->date_ouverture, COLOR_RESET);
            else
                printf("%s  %s Compte %d (Solde: %.2f DH)%s\033[K\n", col, (i == sel) ? "->" : "  ",
                       c->id_compte, c->solde, COLOR_RESET);
        }
        fflush(stdout);
        int k = readKey();
        if      (k == KEY_UP)    sel = (sel - 1 + n) % n;
        else if (k == KEY_DOWN)  sel = (sel + 1) % n;
        else if (k == KEY_ENTER) return idx[sel];
        else if (k == KEY_ESC)   return -1;
        printf("\033[%dA", n);
    }
}

/* ================================================================== */
/*  Menus administrateur                                               */
/* ================================================================== */

int displayMainMenu(void) {
    const char *opts[] = {"Gestion des clients", "Gestion des comptes", "Gestion des operations", "Quitter"};
    clearScreen();
    printHeader("MENU PRINCIPAL");
    printf("  Utilisez les FLECHES pour naviguer et ENTREE pour selectionner:\n");
    printf("  (Appuyez sur ? pour l'aide)\n\n");
    int r = menuAvecFleches(opts, 4, 1);
    if (r == -2) return 4;
    if (r == -1) return 3;
    return r;
}

void displayClientsMenu(void) {
    const char *opts[] = {"Ajouter un client", "Modifier un client", "Supprimer un client",
                          "Rechercher un client", "Afficher tous les clients",
                          "Reinitialiser le PIN d'un client", "Retour au menu principal"};
    for (;;) {
        clearScreen();
        printHeader("GESTION DES CLIENTS");
        printf("  Utilisez les FLECHES pour naviguer et ENTREE pour selectionner:\n\n");
        int r = menuAvecFleches(opts, 7, 0);
        switch (r) {
            case 0: ajouterClient();      break;
            case 1: modifierClient();     break;
            case 2: supprimerClient();    break;
            case 3: rechercherClient();   break;
            case 4: afficherTousClients();break;
            case 5: reinitialiserPIN();   break;
            default: return;
        }
    }
}

void displayAccountsMenu(void) {
    const char *opts[] = {"Creer un compte", "Consulter les comptes", "Fermer un compte", "Retour au menu principal"};
    for (;;) {
        clearScreen();
        printHeader("GESTION DES COMPTES");
        printf("  Utilisez les FLECHES pour naviguer et ENTREE pour selectionner:\n\n");
        int r = menuAvecFleches(opts, 4, 0);
        switch (r) {
            case 0: ajouterCompte();    break;
            case 1: consulterComptes(); break;
            case 2: fermerCompte();     break;
            default: return;
        }
    }
}

void displayOperationsMenu(void) {
    const char *opts[] = {"Effectuer un retrait", "Effectuer un virement", "Retour au menu principal"};
    for (;;) {
        clearScreen();
        printHeader("GESTION DES OPERATIONS");
        printf("  Utilisez les FLECHES pour naviguer et ENTREE pour selectionner:\n\n");
        int r = menuAvecFleches(opts, 3, 0);
        switch (r) {
            case 0: effectuerRetrait();  break;
            case 1: effectuerVirement(); break;
            default: return;
        }
    }
}

void displayAide(void) {
    clearScreen();
    printHeader("AIDE");
    printf("  Navigation :\n");
    printf("    - Fleches HAUT/BAS : deplacer la selection (retour automatique en haut/bas)\n");
    printf("    - ENTREE           : valider le choix\n");
    printf("    - ECHAP (ESC)      : revenir en arriere\n");
    printf("    - Tapez 'cancel'   : annuler une saisie en cours\n\n");
    printf("  Fonctionnalites administrateur :\n");
    printf("    - Clients   : ajouter, modifier, supprimer, rechercher, lister, reinitialiser un PIN\n");
    printf("    - Comptes   : creer (solde initial >= 1000 DH), consulter, fermer\n");
    printf("    - Operations: retraits et virements (max 5000 DH par operation)\n\n");
    printf("  Regles de gestion :\n");
    printf("    - Aucun decouvert : le solde doit couvrir le montant demande\n");
    printf("    - Un client possedant des comptes ne peut pas etre supprime\n");
    waitForEnter();
}

void adminDashboard(void) {
    for (;;) {
        int c = displayMainMenu();
        switch (c) {
            case 0: displayClientsMenu();    break;
            case 1: displayAccountsMenu();   break;
            case 2: displayOperationsMenu(); break;
            case 4: displayAide();           break;
            default: return;                 /* deconnexion */
        }
    }
}

/* ================================================================== */
/*  Menus client                                                       */
/* ================================================================== */

/* 0 comptes, 1 retrait, 2 virement, 3 historique, 4 changer PIN, 5 quitter */
int displayClientMenu(void) {
    const char *opts[] = {"Mes Comptes", "Retrait", "Virement", "Historique", "Changer mon PIN", "Quitter"};
    clearScreen();
    printHeader("MENU CLIENT");
    printf("  Utilisez les FLECHES pour naviguer et ENTREE pour selectionner:\n\n");
    int r = menuAvecFleches(opts, 6, 0);
    return (r == -1) ? 5 : r;
}

void displayClientAccountsMenu(void) {
    clearScreen();
    printHeader("MES COMPTES");
    printf("%s  %-8s| %-13s| %s%s\n", COLOR_WARNING, "ID Cpt", "Solde (DH)", "Date Ouverture", COLOR_RESET);
    printf("  ");
    for (int i = 0; i < 46; i++) putchar('=');
    printf("\n");
    int n = 0;
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_client == logged_in_client_id) {
            printf("  %-8d| %-13.2f| %s\n", comptes[i].id_compte, comptes[i].solde, comptes[i].date_ouverture);
            n++;
        }
    printf("\n  Total: %d compte(s)\n", n);
    waitForEnter();
}

void displayClientWithdrawalMenu(void) {
    clearScreen();
    printHeader("SELECTIONNER UN COMPTE");
    int idx = selectClientAccount(logged_in_client_id, 1);
    if (idx == -1) { sleepMs(1200); displayCancelNotification(); return; }
    clientRetrait(idx);
}

void displayClientTransferMenu(void) {
    clearScreen();
    printHeader("SELECTIONNER UN COMPTE");
    int idx = selectClientAccount(logged_in_client_id, 1);
    if (idx == -1) { sleepMs(1200); displayCancelNotification(); return; }
    clientVirement(idx);
}

void displayClientHistoryMenu(void) {
    afficherHistoriquePourClient(logged_in_client_id);
}

void resetClientPIN(void) {
    char old_pin[64], new_pin[64], confirm[64];
    int ci = trouverIndexClient(logged_in_client_id);
    if (ci == -1) return;

    clearScreen();
    printHeader("CHANGER MON PIN");
    readHidden("  Ancien PIN (ESC pour annuler): ", old_pin, sizeof old_pin);
    if (isCancelInput(old_pin)) { displayCancelNotification(); return; }
    if (strcmp(old_pin, clients[ci].code_pin) != 0) {
        printf("\n");
        printError("Ancien PIN incorrect.");
        waitForEnter();
        return;
    }
    for (;;) {
        readHidden("  Nouveau PIN (4 a 10 chiffres, ESC pour annuler): ", new_pin, sizeof new_pin);
        if (isCancelInput(new_pin)) { displayCancelNotification(); return; }
        if (isValidPIN(new_pin)) break;
        printError("PIN invalide (4 a 10 chiffres uniquement).");
    }
    readHidden("  Confirmez le nouveau PIN: ", confirm, sizeof confirm);
    if (strcmp(new_pin, confirm) != 0) {
        printf("\n");
        printError("Les deux PIN ne correspondent pas.");
        waitForEnter();
        return;
    }
    strcpy(clients[ci].code_pin, new_pin);
    saveClient();
    printf("\n");
    printSuccess("PIN modifie avec succes!");
    waitForEnter();
}

void clientDashboard(void) {
    for (;;) {
        switch (displayClientMenu()) {
            case 0: displayClientAccountsMenu();   break;
            case 1: displayClientWithdrawalMenu(); break;
            case 2: displayClientTransferMenu();   break;
            case 3: displayClientHistoryMenu();    break;
            case 4: resetClientPIN();              break;
            default: return;                       /* deconnexion */
        }
    }
}

/* ================================================================== */
/*  Connexion + boucle principale                                      */
/* ================================================================== */

/* 1 : succes, 0 : echec, -1 : quitter l'application */
int displayLoginPage(void) {
    char id[64], pwd[64];

    printHeader("CONNEXION");
    printf("  (tapez 'exit' pour quitter l'application)\n\n");
    readLine("  ID (ou 'admin'): ", id, sizeof id);
    if (isCancelInput(id) || equalsIgnoreCase(id, "exit")) return -1;
    readHidden("  Password/PIN: ", pwd, sizeof pwd);

    if (equalsIgnoreCase(id, ADMIN_ID)) {
        if (verifyAdminCredentials(id, pwd)) {
            is_admin = 1;
            logged_in_client_id = -1;
            return 1;
        }
    } else {
        int cid;
        if (parseInt(id, &cid) && verifyClientCredentials(cid, pwd)) {
            is_admin = 0;
            logged_in_client_id = cid;           /* "memoire" de la session */
            return 1;
        }
    }
    printf("\n");
    printError("Erreur d'authentification : identifiants incorrects.");
    sleepMs(1500);
    return 0;
}

static void initConsole(void) {
#ifdef _WIN32
    /* Active les codes ANSI (couleurs) dans la console Windows */
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004);
#endif
}

void runApp(void) {
    initConsole();
    for (;;) {
        clearScreen();
        int r = displayLoginPage();
        if (r == -1) {
            clearScreen();
            printBox("Au revoir !");
            return;
        }
        if (r == 1) {
            if (is_admin) adminDashboard();
            else          clientDashboard();
            is_admin = 0;                         /* deconnexion */
            logged_in_client_id = -1;
        }
    }
}
