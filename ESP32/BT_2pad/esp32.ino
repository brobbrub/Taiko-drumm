#include <Arduino.h>
#include <BleGamepad.h>

#define NUM_SENSORS          2
#define ADC_RESOLUTION       9
#define THRESHOLD_SOFT      42    // Soglia base per tutti i sensori
#define DEBOUNCE_DELAY      30    // ms
#define TASK_DELAY          pdMS_TO_TICKS(2)

struct SensorConfig {
    uint8_t pin;                  // Pin del sensore
    uint8_t mainButton;           // Tasto principale (sempre attivato)
    uint8_t secondaryAction;      // Azione secondaria (BUTTON o HAT)
    bool isHatAction;             // true = HAT, false = BUTTON
    uint16_t hardThreshold;       // Soglia "forte" personalizzata
    volatile uint32_t lastHitTime;
    bool wasTriggered;
};

SensorConfig sensors[NUM_SENSORS] = {
    // Destro: BUTTON_7 (soft), BUTTON_8 (hard)
    {32, BUTTON_7, BUTTON_8, false, 160, 0, false}, 
    // Sinistro: BUTTON_4 (soft), HAT_RIGHT (hard)
    {33, BUTTON_4, HAT_RIGHT, true, 150, 0, false}  
};

BleGamepad bleGamepad;

void handleAction(uint8_t action, bool isHat, bool press) {
    if (isHat) {
        bleGamepad.setHat(press ? action : HAT_CENTERED);
    } else {
        press ? bleGamepad.press(action) : bleGamepad.release(action);
    }
}

void sensorTask(void *parameter) {
    SensorConfig *config = (SensorConfig *)parameter;
    
    while(true) {
        const uint32_t currentTime = millis();
        const int value = analogRead(config->pin);
        const bool isTriggered = (value > THRESHOLD_SOFT) && 
                               (currentTime - config->lastHitTime > DEBOUNCE_DELAY);
        const bool isHardHit = (value > config->hardThreshold);

        if (isTriggered != config->wasTriggered) {
            if (isTriggered) {
                // Press main button + secondary action (se hard hit)
                bleGamepad.press(config->mainButton);
                if (isHardHit) {
                    handleAction(config->secondaryAction, config->isHatAction, true);
                }
                config->lastHitTime = currentTime;
            } else {
                // Release tutto
                bleGamepad.release(config->mainButton);
                handleAction(config->secondaryAction, config->isHatAction, false);
            }
            config->wasTriggered = isTriggered;
        }
        vTaskDelay(TASK_DELAY);
    }
}

void setup() {
    analogReadResolution(ADC_RESOLUTION);
    
    BleGamepadConfiguration config;
    config.setAutoReport(true);
    config.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    config.setHatSwitchCount(1);
    bleGamepad.begin(&config);

    for (auto &sensor : sensors) {
        xTaskCreatePinnedToCore(
            sensorTask,
            "SensorTask",
            4096, 
            &sensor,
            1,
            NULL,
            tskNO_AFFINITY
        );
    }
}

void loop() { vTaskDelete(NULL); }
