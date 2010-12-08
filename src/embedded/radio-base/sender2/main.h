#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#define DS18B20

#define RADIO_ID 2

#define RX_DEADLINE 10 // ms until we begin receiving the ACK
#define RX_TIMEOUT  40 // ms timeout of reception (vs. jamming etc)

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V
#define ADC_BATT 0
#define SAMPLES 32

#define ADC_init() ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0) // prescaler 8 -> 1 MHz / 8 = 125 kHz
#define ADC_deinit() ADCSRA &= ~(1 << ADEN)
#define RFM12FREQ868(freq)  ((freq-860.0)/0.005)

#define INSYNC_MAX_FEEDBACK_DEVIATION 6000 // ms deviation over which sync is restarted from scratch

void measure(void);
void sync_time(int16_t time_feedback);

#endif /* MAIN_H */

