# Progetto 2D Arcade - Fondamenti di Computer Grafica

## Requisiti di Sistema
- Compilatore C++
- CMake (versione 3.15 o superiore)
- SFML 3.0

## Istruzioni di Compilazione
Il progetto utilizza CMake per gestire la build di tutte le tappe simultaneamente.
Dalla directory principale del progetto, aprire il terminale ed eseguire i seguenti comandi:

# 1. Creare la cartella di build
mkdir build
cd build

# 2. Generare i file di configurazione
cmake ..

# 3. Compilare tutti gli eseguibili
cmake --build .

## Istruzioni di Esecuzione
Esempio di esecuzione da terminale:
./Tappa11

## Comandi di Gioco (Tastiera)
Le meccaniche vengono introdotte gradualmente tappa per tappa. Di seguito i controlli finali previsti per la versione completa del gioco:
- **Frecce Direzionali / W-A-S-D**: Movimento della navicella spaziale (Su, Giù, Destra, Sinistra).
- **Tasto sinistro Mouse**: Fuoco principale.
- **Tasto P**: Metti in pausa o riprendi il gioco.
- **Tasto R**: Riavvia la partita nella schermata di Game Over.
- **Tasto ESC**: Uscita immediata dall'applicazione.
