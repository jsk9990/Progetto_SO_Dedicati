# Gestione dei Thread Concorrenziali con Allegro

Questo progetto mostra l'implementazione di un'applicazione C++ che gestisce thread concorrenti con diversi algoritmi di scheduling, utilizzando la libreria Allegro 5 per la visualizzazione grafica. Le forme geometriche si muovono nello schermo e i thread sono gestiti con diverse politiche di schedulazione.

## Funzionalità
- **Forme geometriche animate:** Ogni thread rappresenta una forma geometrica (cerchio, quadrato o triangolo) che si muove sullo schermo.
- **Gestione dei thread con diverse politiche di scheduling:** L'utente può selezionare tra SCHED_FIFO, SCHED_RR e SCHED_OTHER.
- **Misurazione delle prestazioni:** Il programma visualizza metriche come il numero di context switch e l'uso della CPU.

## Requisiti
Per compilare e eseguire questo progetto, sono necessarie le seguenti librerie e strumenti:
- **GCC**
- **pthread:** Per la gestione dei thread
- **Allegro 5:** Per la grafica e il rendering
  - allegro5/allegro
  - allegro5/allegro_primitives
  - allegro5/allegro_font
  - allegro5/allegro_ttf

### Installazione su Ubuntu
```bash
sudo apt-get install liballegro5-dev
```

```bash
sudo apt-get install g++
```

## Compilazione e Esecuzione

### Compilazione
Per compilare il progetto, puoi usare un semplice comando `g++`:
```bash
g++ -o progetto progetto.cpp -lallegro -lallegro_primitives -lallegro_font -lallegro_ttf -lpthread
```
### Esecuzione

Dopo aver compilato il programma, esegui il binario generato:

```bash
sudo ./progetto
```

All'avvio del programma, verrà mostrato un menù dove puoi selezionare il tipo di politica di schedulazione da utilizzare. Una volta selezionata, inizieranno le animazioni.


## Dettagli Implementativi

- **Forme geometriche**: Il programma disegna forme che si muovono lungo lo schermo. Ogni forma è associata a un thread.
- **Affinità dei thread**: Ogni thread è fissato su una specifica CPU utilizzando pthread_setaffinity_np.
- **Politiche di scheduling** : L'utente può scegliere tra tre politiche di scheduling:
  - *SCHED_FIFO*
  - *SCHED_RR*
  - *SCHED_OTHER*

## Controlli

- **ESC: Per terminare il programma.**

