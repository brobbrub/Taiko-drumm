Taiko Drumm

Taiko Drumm è un progetto che integra il Bluetooth con l'emulazione di un controller multicore su ESP32.

Requisiti

Hardware: ESP32
4 piezoelettrici, 4 resistenze da 1M

Software:

Arduino IDE

Git

Dipendenze per Bluetooth (ESP32 BLE Library)

Installazione

Clona il repository:

git clone https://github.com/brobbrub/Taiko-drumm.git
cd taiko-drumm

Apri il file ESP32/esp32.ino con Arduino IDE.

Compila e carica il codice su ESP32.

Uso

collega i piezoelettrici ai pin indicati nel codice (pin 32,33 per il codice a 2 piastre con bluetooth,32,33,34,35 per quelli con 4)

Assicurati che l'ESP32 sia alimentato e attivo.

Connetti il controller via Bluetooth o usb(se supportato)

Testa le funzionalità per calibrare i parametri DEBOUNCE_DELAY,THRESHOLD.

note

per sfruttare la connessione USB e necessario un esp32-S2 o esp32-S3.

i tasti scelti hanno come corrispettivo PS :r1,l1, freccia destra e quadrato
i tasti vanno configurati nel gioco.
verranno aggiunte piu avanti anche immagini e il progetto da stampare in 3d ma il lavoro e ancora in corso

