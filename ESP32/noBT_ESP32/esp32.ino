#include <Arduino.h>
#include <BleGamepad.h>
#include <USB.h>
#include <USBHIDGamepad.h>

#define NUM_SENSORS 4
#define THRESHOLD 1000
#define DEBOUNCE_DELAY 10
#define TASK_DELAY pdMS_TO_TICKS(2)

// Configurazione sensori
struct SensorConfig {
    uint8_t pin;
    uint8_t button;
    bool useHat;
    volatile uint32_t lastHitTime;
};

SensorConfig sensors[NUM_SENSORS] = {
    {32, BUTTON_8, false},  // Esterno destro
    {33, BUTTON_4, false},  // Centro destro
    {34, 0, true},         // Centro sinistro (HAT)
    {35, BUTTON_7, false}  // Esterno sinistro
};

// Inizializza entrambi i gamepad
BleGamepad bleGamepad;
USBHIDGamepad usbGamepad;

void handleGamepad(uint8_t button, bool useHat, bool press) {
    if(useHat) {
        bleGamepad.setHat(press ? HAT_RIGHT : HAT_CENTERED);
        usbGamepad.setHat(press ? HAT_RIGHT : HAT_CENTERED);
    } else {
        if(press) {
            bleGamepad.press(button);
            usbGamepad.press(button);
        } else {
            bleGamepad.release(button);
            usbGamepad.release(button);
        }
    }
}

void sensorTask(void *parameter) {
    SensorConfig *config = (SensorConfig *)parameter;
    
    while(true) {
        uint32_t currentTime = millis();
        int value = analogRead(config->pin);
        bool isTriggered = (value > THRESHOLD) && 
                          (currentTime - config->lastHitTime > DEBOUNCE_DELAY);

        if(isTriggered) {
            handleGamepad(config->button, config->useHat, true);
            config->lastHitTime = currentTime;
        } else {
            handleGamepad(config->button, config->useHat, false);
        }
        vTaskDelay(TASK_DELAY);
    }
}

void setup() {
    // Inizializza USB (solo per ESP32-S2/S3)
    USB.begin();

    // Configura BLE
    BleGamepadConfiguration bleConfig;
    bleConfig.setAutoReport(true);
    bleConfig.setControllerType(CONTROLLER_TYPE_GAMEPAD);
    bleConfig.setHatSwitchCount(1);
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
            i % 2
        );
    }
}

void loop() { 
    vTaskDelay(pdMS_TO_TICKS(1000));
}
