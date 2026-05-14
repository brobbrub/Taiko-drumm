# 🥁 Taiko Drumm

**Taiko Drumm** è un controller tamburo BLE open-source basato su ESP32, pensato per giocare a [Taiko no Tatsujin](https://taiko.namco-ch.net/) e [osu! Taiko](https://osu.ppy.sh/) via Bluetooth come gamepad.

---

## Varianti disponibili

| Variante | File | Hardware | Stato |
|---|---|---|---|
| **S3 DualCore** (4 zone) | `taiko_s3_dualcore.ino` | ESP32-S3, 4 piezo | ✅ In sviluppo |
| **Portable** (2 zone) | `taiko_portable.ino` | ESP32, 2 piezo | ✅ Testato e funzionante |

---

## Come funziona

I piezoelettrici rilevano i colpi sul tamburo. Il segnale viene letto tramite ADC e filtrato con una **finestra mobile** per calcolare la potenza dell'impatto. Quando supera la soglia (`HIT_THRES`), viene inviato il tasto corrispondente via BLE come gamepad.
La logica **"vince il colore"** risolve i colpi simultanei: se DON e KA vengono colpiti insieme, vince chi ha la potenza più alta (con un margine configurabile).

---

## Hardware richiesto

### Variante S3 DualCore (4 zone)
- ESP32-S3
- 4 sensori piezoelettrici
- 4 resistenze da 1 MΩ
- Pin usati: `4` (KA_L), `5` (DON_L), `6` (DON_R), `7` (KA_R)

### Variante Portable (2 zone)
- ESP32 (qualsiasi modello con BLE)
- 2 sensori piezoelettrici
- 2 resistenze da 1 MΩ
- Pin usati: `32` (KA_L), `33` (DON_R)

---

## Software richiesto

- [Arduino IDE](https://www.arduino.cc/en/software)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- Libreria [ESP32-BLE-Gamepad](https://github.com/lemmingDev/ESP32-BLE-Gamepad)
- Git

---

## Installazione

```bash
# 1. Clona il repository
git clone https://github.com/brobbrub/Taiko-drumm.git
cd Taiko-drumm
```

1. Apri il file `.ino` della variante scelta con **Arduino IDE**
2. Installa la libreria **ESP32-BLE-Gamepad** dal Library Manager
3. Seleziona la scheda corretta (`ESP32 Dev Module` o `ESP32S3 Dev Module`)
4. **Compila e carica** il codice sull'ESP32

---

## Configurazione

Tutti i parametri si trovano all'inizio del file `.ino`:

| Parametro | Default | Descrizione |
|---|---|---|
| `HIT_THRES` | 255 / 180 | Soglia minima per registrare un colpo |
| `RESET_THRES` | 100 / 60 | Soglia sotto cui il tasto viene rilasciato |
| `MARGINE` | 1.1 | Margine per la logica "vince il colore" |
| `sensitivita[]` | varia per canale | Scala il segnale di ogni piezo |
| `WINDOW_SIZE` | 16 | Dimensione della finestra mobile |
| `SAMPLING_US` | 650 / 500 µs | Intervallo di campionamento |

---

## Mappatura tasti

I tasti emulati corrispondono ai seguenti input PS:

| Zona | Tasto BLE | Equivalente PS |
|---|---|---|
| DON sinistro | HAT_RIGHT (D-Pad →) | Freccia destra |
| DON destro | BUTTON_1 | Quadrato (□) |
| KA sinistro | HAT_UP (D-Pad ↑) | Freccia su |
| KA destro | BUTTON_8 | R1 |

> ⚠️ I tasti vanno configurati anche all'interno del gioco (Taiko no Tatsujin / osu!).

---

## Connessione

1. Alimenta l'ESP32
2. Sul tuo PC/telefono cerca il dispositivo Bluetooth:
   - `Taiko S3 DualCore` (variante 4 zone)
   - `Taiko Portable` (variante 2 zone)
3. Abbina il dispositivo — verrà riconosciuto come **gamepad BLE**
4. Avvia il gioco e configura i tasti nelle impostazioni

---

## Roadmap

- [x] Versione Portable a 2 zone (testata e funzionante)
- [ ] Versione S3 DualCore a 4 zone (realizzata ora in test)
- [ ] Immagini del circuito e del montaggio
- [ ] File STL per la scocca stampata in 3D

---

## Licenza

Progetto open-source — libero per uso personale e non commerciale.
