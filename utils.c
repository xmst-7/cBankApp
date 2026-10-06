#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

#include "utils.h"
#include "globals.h"
#include "menu.h"

/* ------------------------------------------------------------------ */
/*  Saisie                                                             */
/* ------------------------------------------------------------------ */

/* Nettoie le buffer d'entree (evite que '\n' perturbe la saisie suivante) */
void viderBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void trimString(char *s) {
    size_t len = strlen(s), start = 0;
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    while (s[start] && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
}

/* Lit une ligne complete. En cas de fin de flux (EOF) renvoie 0 et "cancel". */
int readLine(const char *prompt, char *buf, size_t n) {
    if (prompt) printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, (int)n, stdin)) {
        snprintf(buf, n, "cancel");
        return 0;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';
    else if (len == n - 1) viderBuffer();        /* ligne trop longue : on jette le reste */
    if (len > 0 && buf[len - 1] == '\r') buf[--len] = '\0';
    trimString(buf);
    return 1;
}

#ifdef _WIN32
int readKey(void) {
    int k = _getch();
    if (k == 224 || k == 0) {              /* touche speciale */
        int k2 = _getch();
        if (k2 == 72) return KEY_UP;       /* fleche haut */
        if (k2 == 80) return KEY_DOWN;     /* fleche bas  */
        return KEY_OTHER;
    }
    if (k == 13) return KEY_ENTER;
    if (k == 27) return KEY_ESC;
    if (k == 8)  return KEY_BACKSPACE;
    return k;
}
#else
/* Version Linux/macOS (pour tester hors Windows) */
static int nextChar(int tty) {
    unsigned char c;
    if (tty) return (read(0, &c, 1) == 1) ? c : -1;
    return getchar();
}
int readKey(void) {
    struct termios old, raw;
    int tty = isatty(0), key, c;
    if (tty) {
        tcgetattr(0, &old);
        raw = old;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &raw);
    }
    c = nextChar(tty);
    if (c < 0) key = KEY_ESC;
    else if (c == 27) {
        if (tty) { raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 1; tcsetattr(0, TCSANOW, &raw); }
        int c2 = nextChar(tty);
        if (c2 == '[') {
            int c3 = nextChar(tty);
            key = (c3 == 'A') ? KEY_UP : (c3 == 'B') ? KEY_DOWN : KEY_OTHER;
        } else key = KEY_ESC;
    }
    else if (c == '\n' || c == '\r') key = KEY_ENTER;
    else if (c == 127 || c == 8)     key = KEY_BACKSPACE;
    else key = c;
    if (tty) tcsetattr(0, TCSANOW, &old);
    return key;
}
#endif

/* Saisie masquee pour les mots de passe / PIN */
void readHidden(const char *prompt, char *buf, size_t n) {
    size_t len = 0;
    printf("%s", prompt);
    fflush(stdout);
    for (;;) {
        int k = readKey();
        if (k == KEY_ENTER) break;
        if (k == KEY_ESC) { snprintf(buf, n, "cancel"); printf("\n"); return; }
        if (k == KEY_BACKSPACE) {
            if (len > 0) { len--; printf("\b \b"); fflush(stdout); }
        } else if (k >= 32 && k < 127 && len < n - 1) {
            buf[len++] = (char)k;
            putchar('*'); fflush(stdout);
        }
    }
    buf[len] = '\0';
    printf("\n");
}

/* ------------------------------------------------------------------ */
/*  Conversion                                                         */
/* ------------------------------------------------------------------ */

int parseInt(const char *s, int *out) {
    char *end;
    if (!s || !*s) return 0;
    long v = strtol(s, &end, 10);
    if (*end != '\0' || v < 0 || v > 2000000000L) return 0;
    *out = (int)v;
    return 1;
}

int parseAmount(const char *s, float *out) {
    char tmp[64], *end;
    if (!s || !*s || strlen(s) >= sizeof tmp) return 0;
    strcpy(tmp, s);
    for (char *p = tmp; *p; p++) if (*p == ',') *p = '.';
    double v = strtod(tmp, &end);
    if (*end != '\0') return 0;
    if (!(v > -1e12 && v < 1e12)) return 0;      /* rejette aussi NaN / inf */
    *out = (float)v;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Recherche lineaire O(n)                                            */
/* ------------------------------------------------------------------ */

int trouverIndexClient(int id_client) {
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].id_client == id_client) return i;
    return -1;
}

int trouverIndexCompte(int id_compte) {
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_compte == id_compte) return i;
    return -1;
}

/* ------------------------------------------------------------------ */
/*  Chaines                                                            */
/* ------------------------------------------------------------------ */

int equalsIgnoreCase(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

int isCancelInput(const char *s) { return equalsIgnoreCase(s, "cancel"); }

int containsIgnoreCase(const char *haystack, const char *needle) {
    size_t hn = strlen(haystack), nn = strlen(needle);
    if (nn == 0) return 1;
    if (nn > hn) return 0;
    for (size_t i = 0; i + nn <= hn; i++) {
        size_t j = 0;
        while (j < nn && tolower((unsigned char)haystack[i + j]) == tolower((unsigned char)needle[j])) j++;
        if (j == nn) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Validation                                                         */
/* ------------------------------------------------------------------ */

/* Lettres (a-z, A-Z) et espaces uniquement, 1 a 49 caracteres */
int isValidName(const char *s) {
    size_t len = strlen(s);
    int has_letter = 0;
    if (len == 0 || len > 49) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (isalpha(c)) has_letter = 1;
        else if (c != ' ') return 0;
    }
    return has_letter;
}

/* 10 chiffres, format marocain 06/07 + 8 chiffres, unique dans la base.
   Retour : 1 valide, 0 format invalide, -1 numero deja utilise.        */
int isValidPhoneNumber(const char *s, int exclude_id_client) {
    if (strlen(s) != 10) return 0;
    for (int i = 0; i < 10; i++) if (!isdigit((unsigned char)s[i])) return 0;
    if (s[0] != '0' || (s[1] != '6' && s[1] != '7')) return 0;
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].id_client != exclude_id_client && strcmp(clients[i].num_tel, s) == 0)
            return -1;
    return 1;
}

/* Chiffres uniquement, longueur 4 a 10 (RG-04) */
int isValidPIN(const char *s) {
    size_t len = strlen(s);
    if (len < PIN_MIN_LEN || len > PIN_MAX_LEN) return 0;
    for (size_t i = 0; i < len; i++) if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Authentification                                                   */
/* ------------------------------------------------------------------ */

int verifyAdminCredentials(const char *id, const char *password) {
    return equalsIgnoreCase(id, ADMIN_ID) && strcmp(password, ADMIN_PASSWORD) == 0;
}

int verifyClientCredentials(int id_client, const char *pin) {
    int idx = trouverIndexClient(id_client);
    if (idx == -1) return 0;
    return strcmp(clients[idx].code_pin, pin) == 0;
}

/* ------------------------------------------------------------------ */
/*  Divers                                                             */
/* ------------------------------------------------------------------ */

void displayCancelNotification(void) {
    printf("\n%s", COLOR_WARNING);
    for (int i = 0; i < LINE_WIDTH; i++) putchar('=');
    printf("\n  Operation annulee.\n");
    for (int i = 0; i < LINE_WIDTH; i++) putchar('=');
    printf("%s\n", COLOR_RESET);
    fflush(stdout);
    sleepMs(1000);
    clearScreen();
}

void getCurrentDate(char *buf) {
    time_t t = time(NULL);
    strftime(buf, 11, "%d/%m/%Y", localtime(&t));
}

void getCurrentDateTime(char *buf) {
    time_t t = time(NULL);
    strftime(buf, 20, "%d/%m/%Y %H:%M:%S", localtime(&t));
}

void sleepMs(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep((useconds_t)ms * 1000);
#endif
}
