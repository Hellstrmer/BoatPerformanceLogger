#include "Arduino.h"
#include "rpm.h"
#include "driver/pcnt.h"

float RPM = 0;

void initRPM() {
  pcnt_config_t cfg = {};
  cfg.pulse_gpio_num = RPM_PIN;
  cfg.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  cfg.channel = PCNT_CHANNEL_0;
  cfg.unit = PCNT_UNIT;
  cfg.pos_mode = PCNT_COUNT_INC;
  cfg.neg_mode = PCNT_COUNT_DIS;
  cfg.counter_h_lim = 32767;
  cfg.counter_l_lim = 0;
  pcnt_unit_config(&cfg);

  //Brusfilter
  pcnt_set_filter_value(PCNT_UNIT, 1023); // 12.8us vid 80MHz, kommer behöva justeras.
  pcnt_filter_enable(PCNT_UNIT);

  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);
  readRPM();
}

float readRPM()
{
    static int16_t lastCount = 0;
    static unsigned long lastTime = 0;

    int16_t count;
    pcnt_get_counter_value(PCNT_UNIT, &count);
    unsigned long now = millis();

    int16_t pulses = count - lastCount;
    if (pulses < 0) pulses += 32768;

    float dt = (now - lastTime) / 1000.0f; // Sekunder
    lastTime = now;
    lastCount = count;

    if (dt <= 0) return 0;
    float freq = pulses / dt; // Pulser/sekund
    return (freq / PULSES_PER_REV) * 60.0f; // RPM
}
