#ifndef DONNEES_H
#define DONNEES_H

/* ===================== Constantes ===================== */
#define MAX_CLIENTS       100
#define MAX_COMPTES       100
#define MAX_TRANSACTIONS  1000

#define SOLDE_MIN         1000.0f   /* RG : solde minimum a la creation   */
#define PLAFOND           5000.0f   /* RG-03 : plafond par operation (DH) */

#define PIN_MIN_LEN       4
#define PIN_MAX_LEN       10

#define ADMIN_ID          "admin"
#define ADMIN_PASSWORD    "admin123"

#define LINE_WIDTH        91

/* ===================== Chemins de donnees ===================== */
#define DATA_DIR       "data"
#define CLIENTS_DIR    "data/clients"
#define RETRAITS_DIR   "data/retraits"
#define VIREMENTS_DIR  "data/virements"
#define CLIENTS_FILE   "data/clients/src"
#define COMPTES_FILE   "data/comptes.txt"
#define IDS_FILE       "data/ids_counter.txt"

/* ===================== Couleurs ANSI ===================== */
#define COLOR_RESET    "\033[0m"
#define COLOR_INFO     "\033[96m"   /* cyan    */
#define COLOR_NORMAL   "\033[97m"   /* blanc   */
#define COLOR_SELECT   "\033[92m"   /* vert    */
#define COLOR_SUCCESS  "\033[92m"
#define COLOR_ERROR    "\033[91m"   /* rouge   */
#define COLOR_WARNING  "\033[93m"   /* jaune   */

/* ===================== Structures ===================== */
typedef struct {
    int  id_client;        /* Identifiant unique            */
    char nom[50];          /* Nom de famille                */
    char prenom[50];       /* Prenom                        */
    char profession[50];   /* Profession/Metier             */
    char num_tel[15];      /* Numero de telephone           */
    char code_pin[20];     /* Code PIN authentification     */
} Client;

typedef struct {
    int   id_compte;          /* Identifiant unique         */
    int   id_client;          /* Reference au client        */
    float solde;              /* Solde actuel               */
    char  date_ouverture[11]; /* JJ/MM/AAAA                 */
} Compte;

/* Type d'une ligne d'historique */
enum { T_RETRAIT = 1, T_ENVOYE = 2, T_RECU = 3 };

typedef struct {
    int    kind;            /* T_RETRAIT / T_ENVOYE / T_RECU */
    int    compte_from;     /* compte debite (ou compte du retrait) */
    int    compte_to;       /* compte credite (0 pour un retrait)   */
    float  montant;
    char   date[20];        /* JJ/MM/AAAA HH:MM:SS           */
    long   ts;              /* date convertie (tri)          */
    int    seq;             /* ordre de lecture (tri stable) */
} Transaction;

#endif
