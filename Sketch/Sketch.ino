#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

// ======== VARIABLES ========
volatile bool stopEvent = false;
volatile bool dirEvent  = false;

bool motorOn   = true;
bool direction = true;

// 🔥 NUEVO: control de bloqueo
bool stopLocked = false;
bool dirLocked  = false;

unsigned long lastStop = 0;
unsigned long lastDir  = 0;

const int debounce = 200;

// ======== INTERRUPCIONES ========
ISR(INT0_vect) {
  if (!(PIND & (1 << PD2))) { // botón presionado
    stopEvent = true;
  }
}

ISR(INT1_vect) {
  if (!(PIND & (1 << PD3))) {
    dirEvent = true;
  }
}

// ======== ADC ========
void setupADC() {
  ADMUX  = (1 << REFS0);
  ADCSRA = (1 << ADEN) |
           (1 << ADPS2) |
           (1 << ADPS1) |
           (1 << ADPS0);
}

uint16_t readADC() {
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADCW;
}

// ======== PWM ========
void setupTimer1() {
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
  TCCR1B = (1 << WGM12)  | (1 << CS11)   | (1 << CS10);

  OCR1A = 0;
  OCR1B = 0;
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);

  DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB4) | (1 << PB5);

  DDRD |= (1 << PD4) | (1 << PD5);
  DDRD &= ~((1 << PD2) | (1 << PD3));

  PORTD |= (1 << PD2) | (1 << PD3);

  EICRA |= (1 << ISC01) | (1 << ISC11);
  EICRA &= ~((1 << ISC00) | (1 << ISC10));

  EIMSK |= (1 << INT0) | (1 << INT1);

  setupADC();
  setupTimer1();

  sei();

  Serial.println("Sistema iniciado");
}

// ======== DIRECCIÓN ========
void setMotorDirection(bool dir) {
  if (dir) {
    PORTD |=  (1 << PD4);
    PORTD &= ~(1 << PD5);
  } else {
    PORTD &= ~(1 << PD4);
    PORTD |=  (1 << PD5);
  }
}

// ======== LOOP ========
void loop() {

  // -------- BOTÓN STOP (ANTI DOBLE CLICK) --------
  if (!(PIND & (1 << PD2))) {  // presionado
    if (!stopLocked && millis() - lastStop > debounce) {
      motorOn = !motorOn;
      lastStop = millis();
      stopLocked = true;

      Serial.println(motorOn ? "Motor ON" : "Motor OFF");
    }
  } else {
    stopLocked = false; // se liberó el botón
  }

  // -------- BOTÓN DIRECCIÓN --------
  if (!(PIND & (1 << PD3))) {
    if (!dirLocked && millis() - lastDir > debounce) {
      direction = !direction;
      lastDir = millis();
      dirLocked = true;

      Serial.println(direction ? "Horario" : "Antihorario");
    }
  } else {
    dirLocked = false;
  }

  // -------- POTENCIÓMETRO --------
  uint16_t potValue = readADC();
  uint8_t pwmValue  = (potValue * 255UL) / 1023;

  OCR1B = pwmValue;

  // -------- MOTOR --------
  if (!motorOn) {
    OCR1A = 0;
  } else {
    OCR1A = pwmValue;
    setMotorDirection(direction);
  }

  // -------- LEDs --------
  if (!motorOn) PORTB |=  (1 << PB4);
  else          PORTB &= ~(1 << PB4);

  if (direction) PORTB |=  (1 << PB5);
  else           PORTB &= ~(1 << PB5);
}
