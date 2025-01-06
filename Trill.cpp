#include "Trill.h"
#include <cstdio>
#include "hardware/gpio.h"

// Constants
#define COMMAND_OFFSET 0
#define DATA_OFFSET 4
#define DEFAULT_I2C_FREQ 400000 // 400 kHz

// Constructor
Trill::Trill() : i2c(nullptr), i2c_address(0), mode(AUTO), device_type(NONE), firmware_version(0), numChannels(0),
                 posRescale(0), sizeRescale(0) {}

// Destructor
Trill::~Trill()
{
    // Nothing to clean up, as the Pico SDK handles I2C cleanup
}

// Setup function
int Trill::setup(i2c_inst_t *i2c_instance, uint sda_pin, uint scl_pin, Device device, uint8_t address)
{
    i2c = i2c_instance;
    i2c_address = address;

    // Initialize I2C
    i2c_init(i2c, DEFAULT_I2C_FREQ);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    // Identify the device
    device_type = device;
    if (identify() != 0)
    {
        printf("Failed to identify Trill device.\n");
        return -1;
    }

    // Set default mode
    if (setMode(CENTROID) != 0)
    {
        printf("Failed to set mode.\n");
        return -1;
    }

    printf("Trill device initialized successfully.\n");
    return 0;
}

// Write data to the device
int Trill::writeBytes(uint8_t *data, size_t size)
{
    int result = i2c_write_blocking(i2c, i2c_address, data, size, false);
    if (result < 0)
    {
        printf("I2C write failed.\n");
    }
    return result;
}

// Read data from the device
int Trill::readBytes(uint8_t offset, uint8_t *data, size_t size)
{
    // Set read offset
    int result = i2c_write_blocking(i2c, i2c_address, &offset, 1, true);
    if (result < 0)
    {
        printf("Failed to set read offset.\n");
        return result;
    }

    // Read data
    result = i2c_read_blocking(i2c, i2c_address, data, size, false);
    if (result < 0)
    {
        printf("I2C read failed.\n");
    }
    return result;
}

// Identify the device
int Trill::identify()
{
    uint8_t command = COMMAND_OFFSET;
    uint8_t response[3] = {0};

    // Send identify command
    int result = writeBytes(&command, 1);
    if (result < 0)
    {
        return -1;
    }

    // Read response
    result = readBytes(COMMAND_OFFSET, response, 3);
    if (result < 0)
    {
        return -1;
    }

    device_type = static_cast<Device>(response[1]);
    firmware_version = response[2];

    printf("Trill device identified: type=%d, firmware=%d\n", device_type, firmware_version);
    return 0;
}

// Set the device mode
int Trill::setMode(Mode new_mode)
{
    uint8_t command[2] = {COMMAND_OFFSET, static_cast<uint8_t>(new_mode)};
    if (writeBytes(command, sizeof(command)) < 0)
    {
        return -1;
    }
    mode = new_mode;
    return 0;
}

// Read I2C data
int Trill::readI2C()
{
    size_t dataSize = 20; // Default length for CENTROID mode
    if (device_type == SQUARE || device_type == HEX)
    {
        dataSize = 32; // Length for 2D devices
    }
    dataBuffer.resize(dataSize);

    if (readBytes(DATA_OFFSET, dataBuffer.data(), dataSize) < 0)
    {
        return -1;
    }

    return 0;
}

// Get the number of touches
unsigned int Trill::getNumTouches()
{
    if (mode != CENTROID)
    {
        return 0;
    }

    // Count the number of valid touches
    unsigned int numTouches = 0;
    for (size_t i = 0; i < dataBuffer.size(); i += 2)
    {
        if (dataBuffer[i] == 0xFF && dataBuffer[i + 1] == 0xFF)
        {
            break;
        }
        numTouches++;
    }
    return numTouches;
}

// Get the location of a touch
float Trill::touchLocation(uint8_t touch_num)
{
    if (mode != CENTROID || touch_num >= getNumTouches())
    {
        return -1;
    }

    uint16_t location = (dataBuffer[2 * touch_num] << 8) | dataBuffer[2 * touch_num + 1];
    return location * posRescale;
}

// Get the size of a touch
float Trill::touchSize(uint8_t touch_num)
{
    if (mode != CENTROID || touch_num >= getNumTouches())
    {
        return -1;
    }

    uint16_t size = (dataBuffer[2 * touch_num + 20] << 8) | dataBuffer[2 * touch_num + 21];
    return size * sizeRescale;
}

// Print device details
void Trill::printDetails()
{
    printf("Trill Device Details:\n");
    printf("  Type: %d\n", device_type);
    printf("  Firmware: %d\n", firmware_version);
    printf("  Address: 0x%02X\n", i2c_address);
}
