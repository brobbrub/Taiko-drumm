#include <BleGamepad.h>

BleGamepad bleGamepad("Taiko S3 DualCore", "Espressif", 100);

const int PIN_KA_L  = 4;
const int PIN_DON_L = 5;
const int PIN_DON_R = 6;
const int PIN_KA_R  = 7;

const int CHANNELS    = 4;
const int WINDOW_SIZE = 16;
const int SAMPLING_US = 650;

const long HIT_THRES   = 255;
const long RESET_THRES = 100;
const float MARGINE    = 1.1f;

float sensitivita[CHANNELS] = {0.7, 0.7, 0.4, 0.6};

int  campioni[CHANNELS][WINDOW_SIZE] = {0};
int  indiceBuffer[CHANNELS]          = {0};
long power[CHANNELS]                 = {0};
long lastPower[CHANNELS]             = {0};

long maxPowerDon = 0;
long maxPowerKa  = 0;

bool premuto[CHANNELS]  = {false, false, false, false};
bool colpoDonAttivo     = false;
bool colpoKaAttivo      = false;

TaskHandle_t TaskDestra;

// ─── Finestra mobile ─────────────────────────────────────────────────────────

void aggiornaCampione(int canale, int nuovoValore) {
  int valoreScalato = (int)(sensitivita[canale] * nuovoValore);
  int idxVecchio    = (indiceBuffer[canale] + 1) % WINDOW_SIZE;
  power[canale]     = power[canale] - campioni[canale][idxVecchio] + valoreScalato;
  campioni[canale][indiceBuffer[canale]] = valoreScalato;
  indiceBuffer[canale] = (indiceBuffer[canale] + 1) % WINDOW_SIZE;
}

// ─── Press/Release per canale ────────────────────────────────────────────────

void premiCanale(int canale) {
  if (premuto[canale]) return;
  switch (canale) {
    case 0: bleGamepad.setHat(HAT_RIGHT);  break; // DON sinistro
    case 1: bleGamepad.press(BUTTON_1); break; // DON destro
    case 2: bleGamepad.setHat(HAT_UP); break; // KA sinistro HAT_UP
    case 3: bleGamepad.press(BUTTON_8); break; // KA destro
  }
  premuto[canale] = true;
  bleGamepad.sendReport();
}

void rilasciaCanale(int canale) {
  if (!premuto[canale]) return;
  switch (canale) {
    case 0: bleGamepad.setHat(HAT_CENTERED); break;
    case 1: bleGamepad.release(BUTTON_1);    break;
    case 2: bleGamepad.setHat(HAT_CENTERED);    break;
    case 3: bleGamepad.release(BUTTON_8);    break;
  }
  premuto[canale] = false;
  bleGamepad.sendReport();
}

// ─── Release ─────────────────────────────────────────────────────────────────

void gestisciRelease() {
  for (int c = 0; c < CHANNELS; c++) {
    if (premuto[c] && power[c] < RESET_THRES) {
      rilasciaCanale(c);
    }
  }
  if (!premuto[0] && !premuto[1]) { maxPowerDon = 0; colpoDonAttivo = false; }
  if (!premuto[2] && !premuto[3]) { maxPowerKa  = 0; colpoKaAttivo  = false; }
}

// ─── Press con logica vince il colore ────────────────────────────────────────

void gestisciPress() {
  bool donReady = (power[0] >= HIT_THRES || power[1] >= HIT_THRES);
  bool kaReady  = (power[2] >= HIT_THRES || power[3] >= HIT_THRES);

  if (!donReady && !kaReady) return;

  bool donVince = (maxPowerDon > maxPowerKa * MARGINE);
  bool kaVince  = (maxPowerKa  > maxPowerDon * MARGINE);

  if (donVince || (!kaVince && donReady && !kaReady)) {
    // DON vince: premi ogni lato indipendentemente, anche se colpoDonAttivo
    if (power[0] >= HIT_THRES) premiCanale(0);
    if (power[1] >= HIT_THRES) premiCanale(1);
    if (premuto[0] || premuto[1]) colpoDonAttivo = true;

  } else if (kaVince || (!donVince && kaReady && !donReady)) {
    // KA vince: premi ogni lato indipendentemente, anche se colpoKaAttivo
    if (power[2] >= HIT_THRES) premiCanale(2);
    if (power[3] >= HIT_THRES) premiCanale(3);
    if (premuto[2] || premuto[3]) colpoKaAttivo = true;

  } else {
    // Troppo vicini: premi chi ha superato la soglia su entrambi i colori
    if (power[0] >= HIT_THRES) premiCanale(0);
    if (power[1] >= HIT_THRES) premiCanale(1);
    if (power[2] >= HIT_THRES) premiCanale(2);
    if (power[3] >= HIT_THRES) premiCanale(3);
    if (premuto[0] || premuto[1]) colpoDonAttivo = true;
    if (premuto[2] || premuto[3]) colpoKaAttivo  = true;
  }
}

// ─── Task Core 0: campiona DON_R e KA_R ──────────────────────────────────────

void TaskDestraCode(void* pvParameters) {
  unsigned long ultimoTempo = micros();
  for (;;) {
    unsigned long elapsed = micros() - ultimoTempo;
    if (elapsed < SAMPLING_US) delayMicroseconds(SAMPLING_US - elapsed);
    ultimoTempo = micros();
    aggiornaCampione(1, analogRead(PIN_DON_R));
    aggiornaCampione(3, analogRead(PIN_KA_R));
    vTaskDelay(1);
  }
}

// ─── Setup ───────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  analogReadResolution(9);

  BleGamepadConfiguration bleGamepadConfig;
  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setButtonCount(16);
  bleGamepadConfig.setWhichAxes(false,false,false,false,false,false,false,false);
  bleGamepad.begin(&bleGamepadConfig);

  xTaskCreatePinnedToCore(TaskDestraCode, "TaskDestra", 4096, NULL, 1, &TaskDestra, 0);

  Serial.println("Taiko pronto!");
}

// ─── Loop Core 1: campiona DON_L e KA_L ──────────────────────────────────────

void loop() {
  if (!bleGamepad.isConnected()) { delay(10); return; }

  unsigned long ultimoTempo = micros();

  aggiornaCampione(0, analogRead(PIN_DON_L));
  aggiornaCampione(2, analogRead(PIN_KA_L));

  // Aggiorna picco sul fronte ascendente
  for (int c = 0; c <= 1; c++) {
    if (power[c] > lastPower[c] && power[c] > maxPowerDon) maxPowerDon = power[c];
    lastPower[c] = power[c];
  }
  for (int c = 2; c <= 3; c++) {
    if (power[c] > lastPower[c] && power[c] > maxPowerKa) maxPowerKa = power[c];
    lastPower[c] = power[c];
  }

  gestisciRelease();
  gestisciPress();

  unsigned long elapsed = micros() - ultimoTempo;
  if (elapsed < SAMPLING_US) delayMicroseconds(SAMPLING_US - elapsed);
}