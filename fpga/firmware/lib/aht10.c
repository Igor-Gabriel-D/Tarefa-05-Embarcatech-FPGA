#include "aht10.h"
#include <stdio.h>
#include <generated/csr.h>
#include <system.h> 

#define AHT10_I2C_ADDR 0x38

static uint32_t i2c_w_reg = 0;

static void i2c_set_scl(int val) {
    if(val) 
    {
        i2c_w_reg |= (1 << CSR_I2C_W_SCL_OFFSET);
    }
    else     
    {
        i2c_w_reg &= ~(1 << CSR_I2C_W_SCL_OFFSET);
    }
    i2c_w_write(i2c_w_reg);
}

static void i2c_set_sda(int val) {
    if(val) 
    {
        i2c_w_reg |= (1 << CSR_I2C_W_SDA_OFFSET);
    }
    else     
    {
        i2c_w_reg &= ~(1 << CSR_I2C_W_SDA_OFFSET);
    }
    i2c_w_write(i2c_w_reg);
}

static void i2c_set_oe(int val) {
    if(val) 
    {
        i2c_w_reg |= (1 << CSR_I2C_W_OE_OFFSET);
    }
    else     
    {
        i2c_w_reg &= ~(1 << CSR_I2C_W_OE_OFFSET);
    }
    i2c_w_write(i2c_w_reg);
}



void i2c_init(void) {
    i2c_set_oe(1); 
    i2c_set_scl(1); 
    i2c_set_sda(1);
    busy_wait_us(1000);
}

static void i2c_start(void) {
    i2c_set_sda(1); 
    i2c_set_oe(1); 
    i2c_set_scl(1); 
    busy_wait_us(5);
    i2c_set_sda(0); 
    busy_wait_us(5);
    i2c_set_scl(0); 
    busy_wait_us(5);
}

static void i2c_stop(void) {
    i2c_set_sda(0); 
    i2c_set_oe(1); 
    i2c_set_scl(0); 
    busy_wait_us(5);
    i2c_set_scl(1); 
    busy_wait_us(5);
    i2c_set_sda(1); 
    busy_wait_us(5);
}

static bool i2c_write_byte(uint8_t byte) {
    int i; 
    bool ack;
    i2c_set_oe(1);
    for (i = 0; i < 8; i++) {
        i2c_set_sda((byte & 0x80) != 0); 
        busy_wait_us(5);
        i2c_set_scl(1); 
        busy_wait_us(5);
        i2c_set_scl(0); 
        busy_wait_us(5);
        byte <<= 1;
    }
    i2c_set_oe(0); 
    i2c_set_sda(1); 
    busy_wait_us(5);
    i2c_set_scl(1); 
    busy_wait_us(5);
    ack = !( (i2c_r_read() & (1 << CSR_I2C_R_SDA_OFFSET)) != 0 );
    i2c_set_scl(0); 
    busy_wait_us(5);
    return ack;
}

static uint8_t i2c_read_byte(bool send_ack) {
    int i; uint8_t byte = 0;
    i2c_set_oe(0); 
    i2c_set_sda(1); 
    busy_wait_us(5);
    for (i = 0; i < 8; i++) {
        byte <<= 1;
        i2c_set_scl(1); 
        busy_wait_us(5);
        if ( (i2c_r_read() & (1 << CSR_I2C_R_SDA_OFFSET)) != 0 ) byte |= 1;
        i2c_set_scl(0); 
        busy_wait_us(5);
    }
    i2c_set_oe(1); 
    i2c_set_sda(!send_ack); 
    busy_wait_us(5);


    i2c_set_scl(1); 
    busy_wait_us(5);
    i2c_set_scl(0); 
    busy_wait_us(5);
    return byte;
}


int aht10_init(void) {
    i2c_start();
    if (!i2c_write_byte(AHT10_I2C_ADDR << 1 | 0)) 
    { 
        i2c_stop(); return -1; 
    }
    if (!i2c_write_byte(0xE1)) 
    { 
        i2c_stop(); return -1; 
    }
    if (!i2c_write_byte(0x08)) 
    { 
        i2c_stop(); return -1; 
    }
    if (!i2c_write_byte(0x00)) 
    { 
        i2c_stop(); return -1; 
    }
    i2c_stop();
    busy_wait_us(100000);
    return 0;
}

bool aht10_get_data(aht10 *d) {
    uint8_t data[6];
    uint32_t raw_humidity, raw_temperature;

    i2c_start();
    if (!i2c_write_byte(AHT10_I2C_ADDR << 1 | 0)) 
    { 
        i2c_stop(); return false; 
    } 
    if (!i2c_write_byte(0xAC)) 
    { 
        i2c_stop(); return false; 
    }
    if (!i2c_write_byte(0x33)) 
    { 
        i2c_stop(); return false; 
    }
    if (!i2c_write_byte(0x00)) 
    { 
        i2c_stop(); return false; 
    }
    i2c_stop();

    busy_wait_us(80000);

    i2c_start();
    if (!i2c_write_byte(AHT10_I2C_ADDR << 1 | 1)) { i2c_stop(); return false; } 
    data[0] = i2c_read_byte(true);
    data[1] = i2c_read_byte(true);
    data[2] = i2c_read_byte(true);
    data[3] = i2c_read_byte(true);
    data[4] = i2c_read_byte(true);
    data[5] = i2c_read_byte(false); 
    i2c_stop();

    raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    d->umidade = (int16_t)(((uint64_t)raw_humidity * 10000) / 0x100000);

    d->temperatura = (int16_t)((((uint64_t)raw_temperature * 20000) / 0x100000) - 5000);

    return true;
}

