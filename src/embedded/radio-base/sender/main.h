#ifndef MAIN_H
#define MAIN_H

#define INT0_init() MCUCR &= ~((1 << ISC01) | (1 << ISC00)) // INT0: low level
#define INT0_on() GICR |= (1 << INT0)
#define INT0_off() GICR &= ~(1 << INT0)

#define DS18B20

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V
#define ADC_BATT 0
#define SAMPLES 32

#define ADC_init() ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0) // prescaler 8 -> 1 MHz / 8 = 125 kHz
#define ADC_deinit() ADCSRA &= ~(1 << ADEN)
#define RFM12FREQ868(freq)  ((freq-860.0)/0.005)

void send(void);

#endif /* MAIN_H */

