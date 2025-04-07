#include <Arduino.h>
#include <BleGamepad.h>

#define NUM_SENSORS 4
#define THRESHOLD 1000  // Soglia unica per tutti i sensori
#define DEBOUNCE_DELAY 10
#define TASK_DELAY pdMS_TO_TICKS(2)

struct SensorConfig {
    uint8_t pin;
    uint8_t button1;  // Pulsante principale
    uint8_t button2;  // Pulsante secondario (0 se non usato)
    bool useHat;      // true per usare HAT invece di button2
    volatile uint32_t lastHitTime;
};

// Configurazione con la tua mappatura esatta:
// - Esterno sinistro: BUTTON_7
// - Esterno destro: BUTTON_8
// - Centro sinistro: HAT_RIGHT
// - Centro destro: BUTTON_4
SensorConfig sensors[NUM_SENSORS] = {
    // Pin | Button1 | Button2 | UseHat
    {32,   BUTTON_8, 0,        false},  // Esterno destro
    {33,   BUTTON_4, 0,        false},  // Centro destro
    {34,   0,       HAT_RIGHT, true},   // Centro sinistro (usa HAT)
    {35,   BUTTON_7, 0,        false}   // Esterno sinistro
};

BleGamepad bleGamepad;

void sensorTask(void *parameter) {
    SensorConfig *config = (SensorConfig *)parameter;
    
    while(true) {
        const uint32_t currentTime = millis();
        const int value = analogRead(config->pin);
        const bool isTriggered = (value > THRESHOLD) && 
                               (currentTime - config->lastHitTime > DEBOUNCE_DELAY);

        if(isTriggered) {
            // Gestione pulsante principale
            if(config->button1 != 0) {
                bleGamepad.press(config->button1);
            }
            
            // Gestione azione secondaria (HAT o pulsante)
            if(config->useHat) {
                bleGamepad.setHat(HAT_RIGHT);
            } else if(config->button2 != 0) {
                bleGamepad.press(config->button2);
            }
            
            config->lastHitTime = currentTime;
        } else {
            // Rilascio
            if(config->button1 != 0) {
                bleGamepad.release(config->button1);
            }
            if(config->useHat) {
                bleGamepad.setHat(HAT_CENTERED);
            } else if(config->button2 != 0) {
                bleGamepad.release(config->button2);
            }
        }
        vTaskDelay(TASK_DELAY);
    }
}

void setup() {
    // Configurazione BLE Gamepad
    BleGamepadConfiguration bleConfig;
    bleConfig.setAutoReport(true);
    bleConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    bleConfig.setHatSwitchCount(1);  // Abilita HAT
    bleConfig.setButtonCount(8);     // Supporta fino a BUTTON_8
    bleGamepad.begin(&bleConfig);

    // Crea task per ogni sensore
    for(int i = 0; i < NUM_SENSORS; i++) {
        xTaskCreatePinnedToCore(
            sensorTask,
            "SensorTask",
            4096,
            &sensors[i],
            1,
            NULL,
            i % 2  // Alterna i core
        );
    }
}

void loop() { vTaskDelete(NULL); }
