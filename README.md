# 🏦 cBank App

**A console-based banking management system written in pure C.**
Manage clients and bank accounts, perform withdrawals and transfers, and keep a full transaction history, with data saved automatically to plain files.

![Language](https://img.shields.io/badge/language-C99-blue)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![Build](https://img.shields.io/badge/build-GCC%20%7C%20CMake-green)
![Status](https://img.shields.io/badge/status-academic%20prototype-orange)

> End-of-module project in C: *"Gestion des clients et des comptes bancaires"*
> Département de Mathématiques et Informatique, Faculté des Sciences Ben M'sick, Université Hassan II de Casablanca, 2024/2025.

---

## 📑 Table of contents

1. [Overview](#-overview)
2. [Features](#-features)
3. [Screenshots](#-screenshots)
4. [Getting started](#-getting-started)
5. [How to use it](#-how-to-use-it)
6. [Business rules](#-business-rules)
7. [Project architecture](#-project-architecture)
8. [Data storage](#-data-storage)
9. [Function reference](#-function-reference)
10. [Key implementation details](#-key-implementation-details)
11. [Known limitations](#-known-limitations)
12. [Roadmap](#-roadmap)
13. [Credits](#-credits)

---

## 🔎 Overview

Managing a bank with paper registers or loose spreadsheets leads to human errors, inconsistent balances, no traceability, and no access control. **cBank App** is a small but complete answer to that problem:

- one program, two roles (**Administrator** and **Client**) with separated privileges,
- every operation validated by business rules *before* any balance changes,
- every withdrawal and transfer written to a history that can be filtered,
- everything saved to disk so the bank is restored exactly as it was after a restart.

It is deliberately built with **no external libraries**: only the C standard library, a few platform headers for keyboard input, and plain text/CSV files. That makes it a good study project for structured programming, file handling, input validation, and modular design in C.

> The user interface is in **French** (menus, prompts, messages), as required by the course.

---

## ✨ Features

### 👨‍💼 Administrator space
| Area | What you can do |
|---|---|
| **Clients** | Add, modify (one field or everything), delete, search (by ID or by partial name, case-insensitive), list all clients, reset a client's PIN |
| **Accounts** | Create an account for a client (initial balance ≥ 1000 DH), list all accounts with their history, close an account |
| **Operations** | Perform withdrawals and transfers on behalf of any client |
| **Help** | Press `?` on the main menu for the built-in help screen |

### 👤 Client space
| Area | What you can do |
|---|---|
| **My accounts** | View all of your accounts with balance and opening date |
| **Withdrawal** | Withdraw from one of your accounts |
| **Transfer** | Send money from one of your accounts to any other account |
| **History** | View your operations, filtered by *withdrawals / sent transfers / received transfers / all*, newest first |
| **PIN** | Change your own PIN (old PIN required, new PIN confirmed) |

### 🛡️ Quality-of-life & safety
- **Arrow-key navigation** in every menu (with wrap-around), `ENTER` to validate, `ESC` to go back.
- **Type `cancel`** at any prompt to abort the current operation.
- **Confirmation screens** (Oui / Non) before every destructive or financial action, showing the balance *before* and *after*.
- **Masked password/PIN input** (`****`).
- **Robust input handling:** invalid input never crashes the program; it shows a clear message and asks again.
- **Automatic persistence:** data is saved after each change.

---

## 🖼️ Screenshots

> Add your screenshots to a `docs/` folder and update the paths below.

| Login | Admin main menu |
|---|---|
| ![Login](docs/login.png) | ![Admin menu](docs/admin-menu.png) |

| Withdrawal confirmation | Transaction history |
|---|---|
| ![Withdrawal](docs/withdrawal.png) | ![History](docs/history.png) |

---

## 🚀 Getting started

### Requirements
- A C compiler (**GCC** recommended; developed with GCC 15.1.0 / MinGW-w64 on Windows 11)
- Optional: **CMake ≥ 3.10**
- A terminal that supports ANSI colors (Windows Terminal, PowerShell on Windows 10+, any Linux/macOS terminal)

### Clone
```bash
git clone https://github.com/<your-username>/cbank-app.git
cd cbank-app
```

### Build

**Option A: plain GCC**

```bash
# Windows (MinGW)
gcc *.c -o cbank_app.exe

# Linux / macOS
gcc -std=c99 -D_DEFAULT_SOURCE *.c -o cbank_app
```

**Option B: CMake**

```bash
cmake -S . -B build
cmake --build build
```
The executable is placed in the `bin/` folder.

### Run

```bash
./cbank_app        # Linux / macOS
cbank_app.exe      # Windows
```

> ⚠️ **Run the program from the folder where you want the data to live.** The `data/` directory is created automatically in the *current working directory* on first launch.

---

## 🧭 How to use it

### 1. Log in

| Role | ID | Password / PIN |
|---|---|---|
| Administrator | `admin` | `admin123` |
| Client | the client's numeric ID (e.g. `1`) | the client's PIN |

Type `exit` at the ID prompt to quit the application.

### 2. A typical first session

1. Log in as **admin**.
2. **Gestion des clients → Ajouter un client**: the ID is generated automatically. Enter name, first name, profession, phone (`06`/`07` + 8 digits) and a PIN.
3. **Gestion des comptes → Creer un compte**: give the client's ID and an initial balance (≥ 1000 DH).
4. Create a second account (for this client or another) to try transfers.
5. **Gestion des operations →** do a withdrawal or a transfer.
6. Quit, then log in as the **client** (ID + PIN) and explore *Mes Comptes*, *Retrait*, *Virement*, *Historique*.

### 3. Keyboard cheat-sheet

| Key | Action |
|---|---|
| `↑` / `↓` | Move the selection (wraps around) |
| `ENTER` | Validate |
| `ESC` | Go back / log out |
| `?` | Help (admin main menu) |
| `cancel` (typed) | Abort the current input |

---

## 📏 Business rules

These rules are enforced in the code, not just documented.

| Code | Rule | Where it applies |
|---|---|---|
| **RG-01** | Client IDs and account IDs are unique (auto-generated counters, never reused). Phone numbers are unique too. | Client / account creation |
| **RG-02** | No overdraft: a withdrawal or transfer is accepted only if `balance ≥ amount`. | Withdrawals, transfers |
| **RG-03** | Maximum **5000 DH** per single operation. | Withdrawals, transfers |
| **RG-04** | A PIN contains digits only (4 to 10). | Client creation / PIN changes |
| Minimum opening balance | **1000 DH** at account creation. | Account creation |
| Referential integrity | A client who still owns accounts **cannot be deleted**. | Client deletion |
| Transfer checks | Destination account must exist and differ from the source account. | Transfers |
| Name validation | Letters and spaces only, 1–49 characters. | Client names |
| Phone validation | Exactly 10 digits, starting with `06` or `07`. | Client phone |

---

## 🏗️ Project architecture

```
cbank-app/
├── main.c          # Entry point: init folders → load IDs → load data → runApp()
├── menu.c / .h     # All UI: login, menus, dashboards, arrow-key navigation, headers/footers
├── clients.c / .h  # Client CRUD, search, listing, PIN reset
├── comptes.c / .h  # Account creation, listing, closing, per-account history
├── operations.c / .h # Withdrawals, transfers, history loading / filtering / display
├── data.c / .h     # File persistence (load/save CSV, history logs, deletions)
├── utils.c / .h    # Validation, search helpers, authentication, input/keyboard, dates
├── globals.c / .h  # In-memory arrays, counters, session variables
├── donnees.h       # Structures (Client, Compte, Transaction) and constants
└── CMakeLists.txt  # Build configuration
```

### Call hierarchy

```
main()
 ├─ initializeDataFolders()
 ├─ loadNextIds()
 ├─ loadAllDataFromFiles()
 └─ runApp()
     ├─ displayLoginPage()        → verifyAdminCredentials() / verifyClientCredentials()
     ├─ adminDashboard()
     │    ├─ displayClientsMenu()    → ajouter / modifier / supprimer / rechercher / afficher / reinitialiserPIN
     │    ├─ displayAccountsMenu()   → ajouterCompte / consulterComptes / fermerCompte
     │    ├─ displayOperationsMenu() → effectuerRetrait / effectuerVirement
     │    └─ displayAide()
     └─ clientDashboard()
          ├─ displayClientAccountsMenu()
          ├─ displayClientWithdrawalMenu() → clientRetrait()
          ├─ displayClientTransferMenu()   → clientVirement()
          ├─ displayClientHistoryMenu()    → afficherHistoriquePourClient()
          └─ resetClientPIN()
```

### Data structures (`donnees.h`)

```c
typedef struct {
    int  id_client;        // unique identifier
    char nom[50];          // last name
    char prenom[50];       // first name
    char profession[50];
    char num_tel[15];      // 10 digits, unique
    char code_pin[20];     // 4 to 10 digits
} Client;

typedef struct {
    int   id_compte;           // unique identifier
    int   id_client;           // owner (foreign key → Client)
    float solde;               // balance in DH
    char  date_ouverture[11];  // DD/MM/YYYY
} Compte;
```

The data lives in static arrays (`clients[100]`, `comptes[100]`), which is why the app is limited to **100 clients and 100 accounts**.

---

## 💾 Data storage

No database: everything is stored in readable text files under `data/`.

```
data/
├── ids_counter.txt          # next_client_id / next_account_id
├── comptes.txt              # all accounts (CSV)
├── clients/
│   └── src                  # all clients (CSV)
├── retraits/
│   └── client_<ID>.txt      # withdrawal log of one client (append-only)
└── virements/
    └── client_<ID>.txt      # transfer log of one client (append-only)
```

### File formats

| File | Format | Example |
|---|---|---|
| `clients/src` | `ID,Nom,Prenom,Profession,Tel,PIN` | `1,Black,Mouad,student,0620181444,6767` |
| `comptes.txt` | `IDCompte,IDClient,Solde,DateOuverture` | `1,1,4550.00,06/10/2026` |
| `ids_counter.txt` | key=value | `next_client_id=2` |
| `retraits/client_1.txt` | `IDCompte,Montant,DateHeure` | `1,100.00,06/10/2026 00:57:16` |
| `virements/client_1.txt` | `From,To,Montant,DateHeure,ENVOYE\|RECU` | `1,2,300.00,06/10/2026 00:57:16,ENVOYE` |

**How a transfer is recorded:** a single transfer writes **two lines**: an `ENVOYE` line in the *sender's* file and a `RECU` line in the *receiver's* file. That is what lets each client filter "sent" and "received" transfers separately.

> 💡 You can reset the whole bank by simply deleting the `data/` folder.

---

## 📚 Function reference

<details>
<summary><b>menu.c: user interface</b></summary>

| Function | Role |
|---|---|
| `runApp()` | Main loop: login page → dashboard → logout → back to login |
| `displayLoginPage()` | Asks ID and masked password/PIN; returns `1` (success), `0` (failure), `-1` (quit) |
| `adminDashboard()` / `clientDashboard()` | Route to the right sub-menus for each role |
| `displayMainMenu()`, `displayClientsMenu()`, `displayAccountsMenu()`, `displayOperationsMenu()` | Admin menus |
| `displayClientMenu()` and the `displayClient…Menu()` family | Client menus |
| `menuAvecFleches()` | Generic arrow-key menu (redraws in place using ANSI cursor movement) |
| `demanderConfirmationAvecFleches()` | Oui/Non selector; returns `'O'` or `'N'` |
| `selectClientAccount()` | Lets the user pick one of a client's accounts with the arrow keys |
| `printHeader()`, `printFooter()`, `printBox()`, `printError()`, `printSuccess()`, `waitForEnter()`, `clearScreen()` | Display helpers |

</details>

<details>
<summary><b>clients.c / comptes.c: business logic</b></summary>

| Function | Role |
|---|---|
| `ajouterClient()` | Collects and validates client data, assigns the next ID, saves |
| `modifierClient()` | Edits one field or all fields; changes are applied only if the whole edit is completed |
| `supprimerClient()` | Refuses if the client owns accounts; asks for confirmation; removes history files |
| `rechercherClient()` | Exact search by ID, or partial case-insensitive search by name |
| `afficherTousClients()` | Table of all clients sorted by ID |
| `reinitialiserPIN()` | Admin resets a client's PIN |
| `ajouterCompte()` | Creates an account linked to an existing client |
| `consulterComptes()` | Lists all accounts and optionally opens an account's history |
| `fermerCompte()` | Closes an account after confirmation |

</details>

<details>
<summary><b>operations.c: transactions</b></summary>

| Function | Role |
|---|---|
| `effectuerRetrait()` / `clientRetrait()` | Withdrawal for admin / for the logged-in client (shared validation) |
| `effectuerVirement()` / `clientVirement()` | Transfer for admin / for the logged-in client (shared validation) |
| `chargerHistorique()` | Reads the client's log files and returns a merged list |
| `afficherHistorique()` | Sorts (newest first) and prints the history table |
| `afficherHistoriquePourClient()` | Asks for a filter, then displays the history |

</details>

<details>
<summary><b>data.c, utils.c, globals.c: infrastructure</b></summary>

| Function | Role |
|---|---|
| `initializeDataFolders()` | Creates the `data/` tree |
| `loadAllDataFromFiles()` | Loads clients and accounts into memory |
| `saveClient()` / `saveAccount()` | Rewrite the full CSV file from memory |
| `saveWithdrawal()`, `saveTransferSent()`, `saveTransferReceived()` | Append a line to the right log |
| `deleteClientFolder()`, `deleteAccountFile()` | Clean-up on deletion |
| `isValidName()`, `isValidPhoneNumber()`, `isValidPIN()` | Input validation |
| `trouverIndexClient()`, `trouverIndexCompte()` | Linear search, O(n), returns `-1` if not found |
| `readLine()`, `readHidden()`, `readKey()`, `viderBuffer()` | Safe line input, masked input, raw key reading |
| `saveNextIds()` / `loadNextIds()` | Persist the ID counters |

</details>

---

## 🔬 Key implementation details

**Arrow-key menus.** `readKey()` returns a unified key code: on Windows it uses `_getch()` (special keys arrive as `224` + `72`/`80`), on Linux/macOS it switches the terminal to raw mode and parses `ESC [ A/B`. `menuAvecFleches()` then redraws the menu in place with the ANSI sequence `\033[<n>A`.

**Safe input.** User input is read with `fgets()` (never a bare `scanf("%s")`), trimmed, and converted with `strtol` / `strtod`, so overlong or non-numeric input cannot overflow a buffer or corrupt the input stream. End-of-input is treated as `cancel`, so the program never loops forever.

**Validate first, mutate later.** Withdrawals and transfers go through the same sequence: validate amount → show a before/after summary → ask for confirmation → update balances → write to disk → log. Nothing changes if the user answers *Non* or types `cancel`.

**Append-only history.** Transaction logs are opened in `"a"` mode, so new lines are added without ever rewriting old history. Dates are parsed back into timestamps (`mktime`) to sort the merged history chronologically.

**ID continuity.** `ids_counter.txt` stores the next IDs; on load, the counters are also raised above the highest existing ID as a safety net.

---

## ⚠️ Known limitations

This is an **academic prototype**, not production banking software.

- **Security is basic:** PINs are stored in **plain text**, and the admin credentials are **hard-coded** (`admin` / `admin123`). Not PCI-DSS compliant.
- **No concurrency control:** designed for a single user on a single machine, with no file locking and no ACID transactions.
- **Flat files, not a DBMS:** the whole file is rewritten on each save; this does not scale to large datasets.
- **Fixed capacity:** 100 clients, 100 accounts (static arrays).
- **Floating-point balances:** `float` is used for money; a real system would use integer cents or a decimal type.
- **Console only:** no GUI.
- **No login lock-out:** repeated wrong PINs are not blocked.

---

## 🗺️ Roadmap

- [ ] Hash PINs/passwords (bcrypt / SHA-256) and move admin credentials out of the source
- [ ] Limit login attempts (lock-out after 3 failures)
- [ ] Replace flat files with **SQLite** (transactions, indexes)
- [ ] Dynamic memory (`malloc`) to remove the 100-record cap
- [ ] Integer cents for monetary values
- [ ] Daily withdrawal limits, low-balance alerts, interest calculation
- [ ] Export statements to CSV / PDF
- [ ] Graphical interface (e.g., GTK 4)
- [ ] Unit tests for validation and transaction logic

---

## 👥 Credits

**Author:** Mohammed MOUSTAFID  
**Module lead:** Pr. Sanaa EL FILALI  
**Supervisor:** Mlle. Hanane ABOURIFA  
**Institution:** Faculté des Sciences Ben M'sick, Université Hassan II de Casablanca, Département de Mathématiques et Informatique  
**Academic year:** 2024/2025  

Feedback, issues and suggestions are welcome. This project marked a step in my learning journey. ⭐ If it helped you, consider starring the repo!  

---
