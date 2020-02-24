//
// Created by alex on 11/27/19.
//

#include <Arduino.h>

//Pin definitions
const byte pin_led_mirror_down = 2;
const byte pin_led_mirror_up = 3;
const byte pin_led_alarm = 7;
const byte pin_speaker = 8;
const byte pin_reed_switch = 9;

//Reed switch debounce time (ms)
unsigned int debounce_time = 200;

//Mirror position variable
//False means mirror is down
bool mirror_pos;

//Time of last mirror position change
unsigned int time_of_last_change;

//Some notes
const unsigned int B = 123;
const unsigned int C = 277;
const unsigned int D = 294;
const unsigned int E = 330;

const unsigned long eighth = 278;
const unsigned long half = 1111;
const unsigned long full = 2222;

//Start and stop the light alarm
void start_light_alarm();
void stop_light_alarm();
void play_sound_alarm(byte sound_pin);

bool reed_switch_changed(byte reed_pin, unsigned int time_of_last, bool mirror_position);

void setup()
{
    pinMode(pin_led_mirror_down, OUTPUT);
    pinMode(pin_led_mirror_up, OUTPUT);
    pinMode(pin_led_alarm, OUTPUT);
    pinMode(pin_speaker, OUTPUT);

    pinMode(pin_reed_switch, INPUT_PULLUP);

    time_of_last_change = millis();

    mirror_pos = digitalRead(pin_reed_switch);
}

void loop()
{
    if(reed_switch_changed(pin_reed_switch, time_of_last_change, mirror_pos))
    {
        start_light_alarm();
        play_sound_alarm(pin_speaker);
        stop_light_alarm();
    }

    mirror_pos = digitalRead(pin_reed_switch);

    digitalWrite(pin_led_mirror_down, !mirror_pos);
    digitalWrite(pin_led_mirror_up, mirror_pos);
}

void start_light_alarm()
{
    // Disable interrupts
    noInterrupts();

    //Clear timer control registers
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;

    //Clear ounter and set compare value (results in 4 Hz frequency)
    TCNT1 = 0;
    OCR1A = 5000;//15625;

    //Enable CTC mode and timer compare itnerrupt
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);

    //Enable interrupts
    interrupts();
}

void stop_light_alarm()
{
    noInterrupts();
    TCCR1B = 0;
    interrupts();
    digitalWrite(pin_led_alarm, LOW);
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(pin_led_alarm, !digitalRead(pin_led_alarm));
}

void play_sound_alarm(byte sound_pin)
{
    tone(sound_pin, E, eighth*9/10);
    delay(eighth);
    tone(sound_pin, E, eighth*9/10);
    delay(eighth);
    tone(sound_pin, E, eighth*9/10);
    delay(eighth);
    tone(sound_pin, C, half*9/10);
    delay(half);
    delay(eighth);

    tone(sound_pin, D, eighth*9/10);
    delay(eighth);
    tone(sound_pin, D, eighth*9/10);
    delay(eighth);
    tone(sound_pin, D, eighth*9/10);
    delay(eighth);
    tone(sound_pin, B, half*9/10);
    delay(full);
}

bool reed_switch_changed(byte reed_pin, unsigned int time_of_last, bool mirror_position)
{
    if((millis() - time_of_last) < debounce_time) return false;
    else return digitalRead(reed_pin) != mirror_position;
}