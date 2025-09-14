#include "ina.h"

static i2c_master_bus_handle_t bus_handle;
static i2c_master_dev_handle_t dev_handle;
static i2c_device_config_t conf = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address = 0x40,
    .scl_speed_hz = 400000,
};

esp_err_t ina_init()
{

    esp_err_t err = 0;
    i2c_master_get_bus_handle(I2C_NUM_0, &bus_handle);
    if ((err = i2c_master_probe(bus_handle, conf.device_address, -1)) == ESP_OK)
    {
        return i2c_master_bus_add_device(bus_handle, &conf, &dev_handle);
    }

    return err;
}

float ina_get_mV()
{
    uint8_t comm = 0x02;
    uint8_t read[2] = {0};

    if (i2c_master_transmit_receive(dev_handle, &comm, sizeof(comm), read, sizeof(read), -1) == ESP_OK)
    {
        uint16_t raw_voltage = (read[0] << 8) | read[1]; // Конвертация из Big-Endian
        float voltage = raw_voltage * 1.25;
        return voltage;
    }

    return -1.0;
}

float ina_get_voltage()
{
    float voltage = ina_get_mV() / 1000.0;
    return roundf(voltage * 10) / 10;
}