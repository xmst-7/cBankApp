#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>

/* Codes de touches renvoyes par readKey() */
#define KEY_ENTER      13
#define KEY_ESC        27
#define KEY_BACKSPACE  8
#define KEY_UP         1001
#define KEY_DOWN       1002
#define KEY_OTHER      1003

/* Saisie */
void viderBuffer(void);
int  readLine(const char *prompt, char *buf, size_t n);   /* 0 si EOF (buf = "cancel") */
void readHidden(const char *prompt, char *buf, size_t n); /* saisie masquee (*)        */
int  readKey(void);
void trimString(char *s);

/* Conversion */
int  parseInt(const char *s, int *out);
int  parseAmount(const char *s, float *out);

/* Recherche */
int  trouverIndexClient(int id_client);
int  trouverIndexCompte(int id_compte);

/* Chaines */
int  isCancelInput(const char *s);
int  equalsIgnoreCase(const char *a, const char *b);
int  containsIgnoreCase(const char *haystack, const char *needle);

/* Validation */
int  isValidName(const char *s);
int  isValidPhoneNumber(const char *s, int exclude_id_client); /* 1 ok, 0 format, -1 doublon */
int  isValidPIN(const char *s);

/* Authentification */
int  verifyAdminCredentials(const char *id, const char *password);
int  verifyClientCredentials(int id_client, const char *pin);

/* Divers */
void displayCancelNotification(void);
void getCurrentDate(char *buf);       /* JJ/MM/AAAA          (>= 11 car.) */
void getCurrentDateTime(char *buf);   /* JJ/MM/AAAA HH:MM:SS (>= 20 car.) */
void sleepMs(int ms);

#endif
