#include "menu.h"
#include "data.h"
#include "globals.h"

int main(void) {
    initializeDataFolders();    /* cree data/, data/clients/, data/retraits/, data/virements/ */
    loadNextIds();              /* compteurs d'ID                                            */
    loadAllDataFromFiles();     /* clients + comptes en memoire                              */
    runApp();                   /* boucle principale (connexion -> menus)                    */
    return 0;
}
