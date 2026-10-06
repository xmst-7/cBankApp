#ifndef MENU_H
#define MENU_H

/* Boucle principale */
void runApp(void);
int  displayLoginPage(void);        /* 1 succes, 0 echec, -1 quitter l'application */
void adminDashboard(void);
void clientDashboard(void);

/* Menus administrateur */
int  displayMainMenu(void);         /* 0 clients, 1 comptes, 2 operations, 3 quitter, 4 aide */
void displayClientsMenu(void);
void displayAccountsMenu(void);
void displayOperationsMenu(void);
void displayAide(void);

/* Menus client */
int  displayClientMenu(void);
void displayClientAccountsMenu(void);
void displayClientWithdrawalMenu(void);
void displayClientTransferMenu(void);
void displayClientHistoryMenu(void);
void resetClientPIN(void);
int  selectClientAccount(int id_client, int style); /* index dans comptes[] ou -1 */

/* Affichage */
void clearScreen(void);
void printHeader(const char *title);
void printFooter(void);
void printBox(const char *msg);
void printError(const char *msg);
void printSuccess(const char *msg);
void waitForEnter(void);
int  menuAvecFleches(const char *options[], int n, int allowHelp);
char demanderConfirmationAvecFleches(void);   /* 'O' ou 'N' */

#endif
