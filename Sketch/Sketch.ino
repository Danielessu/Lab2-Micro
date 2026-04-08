// ======== PINES ========
const int potPin = A0;

const int pwmPin = 9;
const int in1 = 4;
const int in2 = 5;

const int btnStop = 2;
const int btnDir = 3;

const int ledStop = 12;
const int ledDir = 13;
const int ledPWM = 10;

// ======== VARIABLES ========
volatile bool stopEvent = false;
volatile bool dirEvent = false;

bool motorOn = true;
bool direction = true;

unsigned long lastStop = 0;
unsigned long lastDir = 0;
const int debounce = 200;

// ======== INTERRUPCIONES ========
void isrStop() {
  stopEvent = true;
}

void isrDir() {
  dirEvent = true;
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);
  
  pinMode(pwmPin, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(btnStop, INPUT_PULLUP);
  pinMode(btnDir, INPUT_PULLUP);

  pinMode(ledStop, OUTPUT);
  pinMode(ledDir, OUTPUT);
  pinMode(ledPWM, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(btnStop), isrStop, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnDir), isrDir, FALLING);
  
  Serial.println("Sistema iniciado");
}

// ======== LOOP ========
void loop() {

  // --- BOTÓN STOP ---
  if (stopEvent && millis() - lastStop > debounce) {
    motorOn = !motorOn;
    stopEvent = false;
    lastStop = millis();
    Serial.println(motorOn ? "Motor ON" : "Motor OFF");
  }

  // --- BOTÓN DIRECCIÓN ---
  if (dirEvent && millis() - lastDir > debounce) {
    direction = !direction;
    dirEvent = false;
    lastDir = millis();
    Serial.println(direction ? "Horario" : "Antihorario");
  }

  // --- LECTURA POTENCIÓMETRO ---
  int potValue = analogRead(potPin);
  int pwmValue = map(potValue, 0, 1023, 0, 255);
  Serial.println(pwmValue);

  // LED indicador de intensidad
  analogWrite(ledPWM, pwmValue);

  // --- CONTROL MOTOR ---
  if (!motorOn) {
    analogWrite(pwmPin, 0);
  } else {
    analogWrite(pwmPin, pwmValue);

    if (direction) {
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
    } else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
    }
  }

  // --- LEDs DE ESTADO ---
  digitalWrite(ledStop, !motorOn);
  digitalWrite(ledDir, direction);
}
