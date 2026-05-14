#include <BleGamepad.h>

BleGamepad bleGamepad("Taiko Portable", "Espressif", 100);

const int PIN_KA_L  = 32;
const int PIN_DON_R = 33;

const int CHANNELS    = 2;
const int WINDOW_SIZE = 16;
const int SAMPLING_US = 500;

const long HIT_THRES   = 180;
const long RESET_THRES = 60;
const float MARGINE    = 1.1f;

float sensitivita[CHANNELS] = {0.7, 0.6};

int  campioni[CHANNELS][WINDOW_SIZE] = {0};
int  indiceBuffer[CHANNELS]          = {0};
long power[CHANNELS]                 = {0};
long lastPower[CHANNELS]             = {0};

long maxPowerDon = 0;
long maxPowerKa  = 0;

bool premuto[CHANNELS]  = {false, false};
bool colpoDonAttivo     = false;
bool colpoKaAttivo      = false;

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
    case 0: bleGamepad.press(BUTTON_7); break; // KA sinistro
    case 1: bleGamepad.press(BUTTON_1); break; // DON destro
  }
  premuto[canale] = true;
  bleGamepad.sendReport();
}

void rilasciaCanale(int canale) {
  if (!premuto[canale]) return;
  switch (canale) {
    case 0: bleGamepad.release(BUTTON_7); break;
    case 1: bleGamepad.release(BUTTON_1); break;
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
  if (!premuto[1]) { maxPowerDon = 0; colpoDonAttivo = false; }
  if (!premuto[0]) { maxPowerKa  = 0; colpoKaAttivo  = false; }
}

// ─── Press con logica vince il colore ────────────────────────────────────────

void gestisciPress() {
  bool donReady = (power[1] >= HIT_THRES);
  bool kaReady  = (power[0] >= HIT_THRES);

  if (!donReady && !kaReady) return;

  bool donVince = (maxPowerDon > maxPowerKa * MARGINE);
  bool kaVince  = (maxPowerKa  > maxPowerDon * MARGINE);

  if (donVince || (!kaVince && donReady && !kaReady)) {
    if (power[1] >= HIT_THRES) premiCanale(1);
    if (premuto[1]) colpoDonAttivo = true;

  } else if (kaVince || (!donVince && kaReady && !donReady)) {
    if (power[0] >= HIT_THRES) premiCanale(0);
    if (premuto[0]) colpoKaAttivo = true;

  } else {
    // Troppo vicini: vince chi ha power più alta
    if (maxPowerDon >= maxPowerKa) {
      if (power[1] >= HIT_THRES) premiCanale(1);
      if (premuto[1]) colpoDonAttivo = true;
    } else {
      if (power[0] >= HIT_THRES) premiCanale(0);
      if (premuto[0]) colpoKaAttivo = true;
    }
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

  Serial.println("Taiko Portable pronto!");
}

// ─── Loop ────────────────────────────────────────────────────────────────────

void loop() {
  if (!bleGamepad.isConnected()) { delay(10); return; }

  unsigned long ultimoTempo = micros();

  aggiornaCampione(0, analogRead(PIN_KA_L));
  aggiornaCampione(1, analogRead(PIN_DON_R));

  // Aggiorna picco sul fronte ascendente
  if (power[1] > lastPower[1] && power[1] > maxPowerDon) maxPowerDon = power[1];
  lastPower[1] = power[1];
  if (power[0] > lastPower[0] && power[0] > maxPowerKa)  maxPowerKa  = power[0];
  lastPower[0] = power[0];

  gestisciRelease();
  gestisciPress();

  unsigned long elapsed = micros() - ultimoTempo;
  if (elapsed < SAMPLING_US) delayMicroseconds(SAMPLING_US - elapsed);
}
