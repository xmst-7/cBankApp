#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clients.h"
#include "globals.h"
#include "utils.h"
#include "menu.h"
#include "data.h"

/* ---------- Helpers de saisie (re-demandent jusqu'a validite) ---------- */
/* Retournent 1 si valide, 0 si l'utilisateur tape 'cancel'. */

static int askName(const char *prompt, char *dest) {
    char buf[128];
    for (;;) {
        readLine(prompt, buf, sizeof buf);
        if (isCancelInput(buf)) return 0;
        if (isValidName(buf)) { strcpy(dest, buf); return 1; }
        printError("Nom invalide (lettres et espaces uniquement, 49 car. max).");
    }
}

static int askProfession(const char *prompt, char *dest) {
    char buf[128];
    for (;;) {
        readLine(prompt, buf, sizeof buf);
        if (isCancelInput(buf)) return 0;
        if (strlen(buf) == 0 || strlen(buf) > 49) {
            printError("Profession invalide (1 a 49 caracteres).");
            continue;
        }
        for (char *p = buf; *p; p++) if (*p == ',') *p = ' ';   /* protege le CSV */
        strcpy(dest, buf);
        return 1;
    }
}

static int askPhone(const char *prompt, char *dest, int exclude_id) {
    char buf[64];
    for (;;) {
        readLine(prompt, buf, sizeof buf);
        if (isCancelInput(buf)) return 0;
        int r = isValidPhoneNumber(buf, exclude_id);
        if (r == 1) { strcpy(dest, buf); return 1; }
        if (r == -1) printError("Ce numero de telephone existe deja.");
        else         printError("Numero invalide (06/07 + 8 chiffres).");
    }
}

static int askPIN(const char *prompt, char *dest) {
    char buf[64];
    for (;;) {
        readLine(prompt, buf, sizeof buf);
        if (isCancelInput(buf)) return 0;
        if (isValidPIN(buf)) { strcpy(dest, buf); return 1; }
        printError("PIN invalide (4 a 10 chiffres uniquement).");
    }
}

/* ---------- Ajouter ---------- */
void ajouterClient(void) {
    clearScreen();
    printHeader("AJOUTER UN CLIENT");

    if (nb_clients >= MAX_CLIENTS) {
        printError("Capacite maximale de clients atteinte.");
        waitForEnter();
        return;
    }

    Client c;
    memset(&c, 0, sizeof c);
    c.id_client = next_client_id;
    printf("  ID Client (auto): %d\n\n", c.id_client);

    if (!askName("  Nom (ou 'cancel'): ", c.nom) ||
        !askName("  Prenom (ou 'cancel'): ", c.prenom) ||
        !askProfession("  Profession (ou 'cancel'): ", c.profession) ||
        !askPhone("  Num Tel (Marocain 06/07 + 8 chiffres, ou 'cancel'): ", c.num_tel, 0) ||
        !askPIN("  Code PIN (4 a 10 chiffres, ou 'cancel'): ", c.code_pin)) {
        displayCancelNotification();
        return;
    }

    clients[nb_clients++] = c;
    next_client_id++;
    saveClient();
    saveNextIds();

    printf("\n");
    printSuccess("Client ajoute avec succes!");
    waitForEnter();
}

/* ---------- Modifier ---------- */
void modifierClient(void) {
    char buf[64];
    while (1) {
        clearScreen();
        printHeader("MODIFIER UN CLIENT");

        readLine("  ID du client a modifier (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }

        int id, idx = -1;
        if (!parseInt(buf, &id) || (idx = trouverIndexClient(id)) == -1) {
            clearScreen();
            printError("Client introuvable.");
            sleepMs(1500);
            continue;                                   /* reessayer */
        }

        Client tmp = clients[idx];
        printf("\n  --- Modification de %s %s ---\n\n", tmp.nom, tmp.prenom);
        printf("  1. Nom\n  2. Prenom\n  3. Profession\n  4. Tel\n  5. Code PIN\n  6. Tout modifier\n\n");

        int ch;
        readLine("  Votre choix: ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        if (!parseInt(buf, &ch) || ch < 1 || ch > 6) {
            printError("Choix invalide.");
            sleepMs(1200);
            continue;
        }

        int ok = 1;
        if (ok && (ch == 1 || ch == 6)) ok = askName("  Nouveau Nom (ou 'cancel'): ", tmp.nom);
        if (ok && (ch == 2 || ch == 6)) ok = askName("  Nouveau Prenom (ou 'cancel'): ", tmp.prenom);
        if (ok && (ch == 3 || ch == 6)) ok = askProfession("  Nouvelle Profession (ou 'cancel'): ", tmp.profession);
        if (ok && (ch == 4 || ch == 6)) ok = askPhone("  Nouveau Tel (Marocain 06/07 + 8 chiffres, ou 'cancel'): ", tmp.num_tel, tmp.id_client);
        if (ok && (ch == 5 || ch == 6)) ok = askPIN("  Nouveau Code PIN (4 a 10 chiffres, ou 'cancel'): ", tmp.code_pin);

        if (!ok) { displayCancelNotification(); return; }

        clients[idx] = tmp;
        saveClient();                                   /* persistance immediate */
        clearScreen();
        printSuccess("Client modifie avec succes!");
        sleepMs(1000);
        clearScreen();
        break;                                          /* quitter la boucle */
    }
}

/* ---------- Supprimer ---------- */
void supprimerClient(void) {
    char buf[64];
    while (1) {
        clearScreen();
        printHeader("SUPPRIMER UN CLIENT");
        readLine("  ID Client a supprimer (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }

        int id, idx = -1;
        if (!parseInt(buf, &id) || (idx = trouverIndexClient(id)) == -1) {
            printError("Client introuvable.");
            sleepMs(1500);
            continue;
        }

        /* Regle d'integrite : pas de suppression si des comptes existent */
        int nb = 0;
        for (int i = 0; i < nb_comptes; i++) if (comptes[i].id_client == id) nb++;
        if (nb > 0) {
            printf("\n");
            printError("Suppression impossible : ce client possede des comptes actifs.");
            printf("  Fermez d'abord ses %d compte(s).\n", nb);
            waitForEnter();
            return;
        }

        clearScreen();
        printHeader("CONFIRMATION SUPPRESSION");
        printf("  Informations du client:\n");
        printf("  - ID: %d\n  - Nom: %s\n  - Prenom: %s\n  - Profession: %s\n  - Tel: %s\n",
               clients[idx].id_client, clients[idx].nom, clients[idx].prenom,
               clients[idx].profession, clients[idx].num_tel);
        printf("\n%s  Etes-vous sur de vouloir supprimer ce client?%s\n", COLOR_WARNING, COLOR_RESET);

        if (demanderConfirmationAvecFleches() == 'O') {
            deleteClientFolder(id);
            for (int i = idx; i < nb_clients - 1; i++) clients[i] = clients[i + 1];
            nb_clients--;
            saveClient();
            printf("\n");
            printSuccess("Client supprime avec succes!");
            waitForEnter();
        } else {
            displayCancelNotification();
        }
        return;
    }
}

/* ---------- Rechercher ---------- */
static void printClientLine(const Client *c) {
    printf("%s  -> ID: %d | %s %s | Job: %s | Tel: %s%s\n", COLOR_SUCCESS,
           c->id_client, c->nom, c->prenom, c->profession, c->num_tel, COLOR_RESET);
}

void rechercherClient(void) {
    char buf[128];
    clearScreen();
    printHeader("RECHERCHER UN CLIENT");

    for (;;) {
        readLine("  Rechercher par: 1. ID  2. Nom (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        if (strcmp(buf, "1") == 0 || strcmp(buf, "2") == 0) break;
        printError("Choix invalide (1 ou 2).");
    }
    int mode = buf[0] - '0';

    if (mode == 1) {                                    /* correspondance exacte */
        readLine("  ID (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id, idx;
        printf("\n");
        if (!parseInt(buf, &id) || (idx = trouverIndexClient(id)) == -1)
            printError("Client non trouve.");
        else
            printClientLine(&clients[idx]);
    } else {                                            /* partielle, insensible a la casse */
        readLine("  Nom ou prenom (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int found = 0;
        printf("\n");
        for (int i = 0; i < nb_clients; i++) {
            char full[110];
            snprintf(full, sizeof full, "%s %s", clients[i].nom, clients[i].prenom);
            if (containsIgnoreCase(full, buf)) { printClientLine(&clients[i]); found++; }
        }
        if (found == 0) printError("Client non trouve.");
        else printf("\n  %d resultat(s) trouve(s).\n", found);
    }
    waitForEnter();
}

/* ---------- Afficher tous (tri par ID croissant) ---------- */
static int cmpClientId(const void *a, const void *b) {
    return ((const Client *)a)->id_client - ((const Client *)b)->id_client;
}

void afficherTousClients(void) {
    clearScreen();
    printHeader("LISTE DE TOUS LES CLIENTS");

    Client tri[MAX_CLIENTS];
    memcpy(tri, clients, sizeof(Client) * (size_t)nb_clients);
    qsort(tri, (size_t)nb_clients, sizeof(Client), cmpClientId);

    printf("%s  %-5s| %-18s| %-18s| %-18s| %-13s| %s%s\n", COLOR_WARNING,
           "ID", "Nom", "Prenom", "Profession", "Telephone", "PIN", COLOR_RESET);
    printf("  ");
    for (int i = 0; i < 88; i++) putchar('=');
    printf("\n");
    for (int i = 0; i < nb_clients; i++)
        printf("  %-5d| %-18s| %-18s| %-18s| %-13s| %s\n", tri[i].id_client, tri[i].nom,
               tri[i].prenom, tri[i].profession, tri[i].num_tel, tri[i].code_pin);

    printf("\n%s  Total: %d client(s) enregistre(s)%s\n", COLOR_WARNING, nb_clients, COLOR_RESET);
    waitForEnter();
}

/* ---------- Reinitialisation administrative du PIN ---------- */
void reinitialiserPIN(void) {
    char buf[64];
    clearScreen();
    printHeader("REINITIALISER LE PIN D'UN CLIENT");

    Client tri[MAX_CLIENTS];
    memcpy(tri, clients, sizeof(Client) * (size_t)nb_clients);
    qsort(tri, (size_t)nb_clients, sizeof(Client), cmpClientId);

    printf("%s  %-5s| %-18s| %-18s| %s%s\n", COLOR_WARNING, "ID", "Nom", "Prenom", "PIN", COLOR_RESET);
    printf("  ");
    for (int i = 0; i < 52; i++) putchar('=');
    printf("\n");
    for (int i = 0; i < nb_clients; i++)
        printf("  %-5d| %-18s| %-18s| %s\n", tri[i].id_client, tri[i].nom, tri[i].prenom, tri[i].code_pin);

    int idx;
    for (;;) {
        readLine("\n  Entrez l'ID du client a modifier (ou 'cancel'): ", buf, sizeof buf);
        if (isCancelInput(buf)) { displayCancelNotification(); return; }
        int id;
        if (parseInt(buf, &id) && (idx = trouverIndexClient(id)) != -1) break;
        printError("Client introuvable.");
    }

    char pin[20];
    if (!askPIN("\n  Entrez le nouveau PIN (4 a 10 chiffres, ou 'cancel'): ", pin)) {
        displayCancelNotification();
        return;
    }
    strcpy(clients[idx].code_pin, pin);
    saveClient();
    printf("\n");
    printSuccess("PIN reinitialise avec succes!");
    waitForEnter();
}
