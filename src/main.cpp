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

const bool mirror_up_pos = false;
const bool mirror_down_pos = true;

//Reed switch debounce time (ms)
unsigned int debounce_time = 200;

//Mirror position variable
bool mirror_pos;

//Some notes
const unsigned int NOTE_HIGH = 1200;
const unsigned int NOTE_LOW = 800;

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

    mirror_pos = digitalRead(pin_reed_switch);
}

void loop()
{
    static unsigned long time_of_last_change = millis();
    if(reed_switch_changed(pin_reed_switch, time_of_last_change, mirror_pos))
    {
        start_light_alarm();
        play_sound_alarm(pin_speaker);
        stop_light_alarm();
        time_of_last_change = millis();
    }

    mirror_pos = digitalRead(pin_reed_switch);

    digitalWrite(pin_led_mirror_down, mirror_pos==mirror_down_pos);
    digitalWrite(pin_led_mirror_up, mirror_pos==mirror_up_pos);
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
  for(byte i=0; i<3; i++)
  {
    tone(sound_pin, NOTE_HIGH);
    delay(250);
    tone(sound_pin, NOTE_LOW);
    delay(250);
  }
  noTone(sound_pin);
}

bool reed_switch_changed(byte reed_pin, unsigned int time_of_last, bool mirror_position)
{
    if((millis() - time_of_last) < debounce_time) return false;
    else return digitalRead(reed_pin) != mirror_position;
}