#ifndef DATA_H
#define DATA_H

void initializeDataFolders(void);
void loadAllDataFromFiles(void);
void saveClient(void);      /* reecrit data/clients/src (tous les clients)  */
void saveAccount(void);     /* reecrit data/comptes.txt (tous les comptes)  */
void saveWithdrawal(int clientId, int accountId, float amount, const char *date);
void saveTransferSent(int clientId, int from, int to, float amount, const char *date);
void saveTransferReceived(int clientId, int from, int to, float amount, const char *date);
void deleteClientFolder(int clientId);   /* supprime l'historique du client */
void deleteAccountFile(int accountId);   /* reecrit comptes.txt sans ce compte */

#endif
