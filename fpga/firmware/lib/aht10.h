#ifndef AHT10_H_
#define AHT10_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t temperatura; 
    int16_t umidade;     
} aht10;


void i2c_init(void);
void i2c_scan(void);
int aht10_init(void);
bool aht10_get_data(aht10 *d);

#endif
