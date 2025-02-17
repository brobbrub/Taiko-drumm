#include <Arduino.h>
#include <USB.h>
#include <USBHIDJoystick.h>  // Libreria per USB HID Joystick

#define NUM_SENSORS 4

// Definizione dei pin a cui sono collegati i sensori piezoelettrici
const int sensorPins[NUM_SENSORS] = {32, 33, 34, 35};

// Etichette per identificare i sensori
const char* sensorLabels[NUM_SENSORS] = {
  "Esterno Destro (Blu)",
  "Interno Destro (Rosso)",
  "Interno Sinistro (Rosso)",
  "Esterno Sinistro (Blu)"
};

// Soglia per il rilevamento del colpo (da regolare in base alle tue letture)
const int threshold = 1000;

// Tempo di debounce (in millisecondi)
const unsigned long debounceDelay = 10;

// Delay per ogni ciclo dei task, definito come parametro globale (in ticks)
const TickType_t TASK_DELAY = pdMS_TO_TICKS(1);

// Array per tenere traccia dell'ultimo colpo per ciascun sensore
volatile unsigned long lastHitTime[NUM_SENSORS] = {0, 0, 0, 0};

// Oggetto per emulare un controller HID via USB
USBHIDJoystick usbGamepad;

// I pulsanti usati nel codice sono gli stessi del precedente esempio BLE.
// Assicurati che la libreria USBHIDJoystick li gestisca in modo analogo.
// Se necessario, definisci i pulsanti (questo potrebbe variare in base alla libreria):
#ifndef BUTTON_4
#define BUTTON_4 4
#endif
#ifndef BUTTON_5
#define BUTTON_5 5
#endif
#ifndef BUTTON_6
#define BUTTON_6 6
#endif
#ifndef BUTTON_7
#define BUTTON_7 7
#endif

//
// Task per gestire i sensori Blu (indici 0 e 3)
//
void blueTask(void *parameter) {
  while (true) {
    unsigned long currentTime = millis();

    // Legge i valori dei sensori Blu
    int value0 = analogRead(sensorPins[0]);
    int value3 = analogRead(sensorPins[3]);

    // Verifica se i valori superano la soglia e se il debounce è trascorso
    bool trigger0 = (value0 > threshold) && ((currentTime - lastHitTime[0]) > debounceDelay);
    bool trigger3 = (value3 > threshold) && ((currentTime - lastHitTime[3]) > debounceDelay);

    // Se entrambi i sensori sono attivati contemporaneamente, gestisce il doppio colpo
    if (trigger0 && trigger3) {
      usbGamepad.press(BUTTON_5); // Simula L1 (Blu Sinistro)
      usbGamepad.press(BUTTON_6); // Simula R1 (Blu Destro)
      Serial.println("Doppio colpo Blu rilevato");
      lastHitTime[0] = currentTime;
      lastHitTime[3] = currentTime;
    }
    else {
      // Se solo uno dei due viene attivato, gestisce il colpo singolo
      if (trigger0) {
        usbGamepad.press(BUTTON_5); // Simula L1 (Blu Sinistro)
        Serial.print("Colpo singolo Blu rilevato su: ");
        Serial.println(sensorLabels[0]);
        lastHitTime[0] = currentTime;
      } else {
        usbGamepad.release(BUTTON_5);
      }

      if (trigger3) {
        usbGamepad.press(BUTTON_6); // Simula R1 (Blu Destro)
        Serial.print("Colpo singolo Blu rilevato su: ");
        Serial.println(sensorLabels[3]);
        lastHitTime[3] = currentTime;
      } else {
        usbGamepad.release(BUTTON_6);
      }
    }
    // Usa vTaskDelay invece di delay
    vTaskDelay(TASK_DELAY);
  }
}

//
// Task per gestire i sensori Rossi (indici 1 e 2)
//
void redTask(void *parameter) {
  while (true) {
    unsigned long currentTime = millis();

    // Legge i valori dei sensori Rossi
    int value1 = analogRead(sensorPins[1]);
    int value2 = analogRead(sensorPins[2]);

    // Verifica se i valori superano la soglia e se il debounce è trascorso
    bool trigger1 = (value1 > threshold) && ((currentTime - lastHitTime[1]) > debounceDelay);
    bool trigger2 = (value2 > threshold) && ((currentTime - lastHitTime[2]) > debounceDelay);

    // Se entrambi i sensori sono attivati contemporaneamente, gestisce il doppio colpo
    if (trigger1 && trigger2) {
      usbGamepad.press(BUTTON_7); // Simula Freccia Destra (Rosso Sinistro)
      usbGamepad.press(BUTTON_4); // Simula Quadrato (Rosso Destro)
      Serial.println("Doppio colpo Rosso rilevato");
      lastHitTime[1] = currentTime;
      lastHitTime[2] = currentTime;
    }
    else {
      // Se solo uno dei due viene attivato, gestisce il colpo singolo
      if (trigger1) {
        usbGamepad.press(BUTTON_7); // Simula Freccia Destra (Rosso Sinistro)
        Serial.print("Colpo singolo Rosso rilevato su: ");
        Serial.println(sensorLabels[1]);
        lastHitTime[1] = currentTime;
      } else {
        usbGamepad.release(BUTTON_7);
      }

      if (trigger2) {
        usbGamepad.press(BUTTON_4); // Simula Quadrato (Rosso Destro)
        Serial.print("Colpo singolo Rosso rilevato su: ");
        Serial.println(sensorLabels[2]);
        lastHitTime[2] = currentTime;
      } else {
        usbGamepad.release(BUTTON_4);
      }
    }
    // Usa vTaskDelay invece di delay
    vTaskDelay(TASK_DELAY);
  }
}

//
// Funzione di setup
//
void setup() {
  Serial.begin(115200);

  // Configura i pin dei sensori come ingressi
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  // Inizializza il controller HID via USB
  usbGamepad.begin();
  Serial.println("In attesa di connessione USB come Joystick...");

  // Crea il task per i sensori Blu sul core 0
  xTaskCreatePinnedToCore(
    blueTask,       // Funzione task
    "BlueTask",     // Nome del task
    2048,           // Stack size in bytes
    NULL,           // Parametro passato al task
    1,              // Priorità del task
    NULL,           // Handle del task
    0               // Core su cui eseguire il task
  );

  // Crea il task per i sensori Rossi sul core 1
  xTaskCreatePinnedToCore(
    redTask,        // Funzione task
    "RedTask",      // Nome del task
    2048,           // Stack size in bytes
    NULL,           // Parametro passato al task
    1,              // Priorità del task
    NULL,           // Handle del task
    1               // Core su cui eseguire il task
  );
}

//
// Funzione loop (non utilizzata, in quanto le operazioni sono gestite dai task)
//
void loop() {
  // Il loop principale può rimanere vuoto oppure eseguire altre operazioni
  vTaskDelay(pdMS_TO_TICKS(1000));
}