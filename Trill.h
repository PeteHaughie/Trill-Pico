#pragma once

#ifndef TRILL_H
#define TRILL_H

#include "hardware/i2c.h" // Use the Pico SDK I2C library
#include <stdint.h>
#include <vector> // Include std::vector
#include <string.h>

/**
 * Trill library adapted for Raspberry Pi Pico.
 * Communicates with Trill sensors using I2C.
 */

class Trill
{
public:
    // Acquisition modes
    typedef enum
    {
        AUTO = -1,    /**< Auto mode: automatically set based on the device type */
        CENTROID = 0, /**< Centroid mode: detect discrete touches */
        RAW = 1,      /**< Raw mode */
        BASELINE = 2, /**< Baseline mode */
        DIFF = 3      /**< Differential mode */
    } Mode;

    // Types of Trill devices
    typedef enum
    {
        NONE = -1,  ///< No device
        ANY = 0,    ///< Any valid device of unknown type
        BAR = 1,    ///< Trill Bar
        SQUARE = 2, ///< Trill Square
        CRAFT = 3,  ///< Trill Craft
        RING = 4,   ///< Trill Ring
        HEX = 5,    ///< Trill Hex
        FLEX = 6,   ///< Trill Flex
        UNKNOWN = ANY
    } Device;

private:
    i2c_inst_t *i2c;                 // I2C instance (i2c0 or i2c1)
    uint8_t i2c_address;             // I2C address of the device
    Mode mode;                       // Current mode of the device
    Device device_type;              // Type of Trill device connected
    uint8_t firmware_version;        // Firmware version
    std::vector<uint8_t> dataBuffer; // Use std::vector for dynamic buffer
    unsigned int numChannels;        // Number of capacitive channels
    float posRescale, sizeRescale;   // Rescaling factors

    // Helper functions
    int writeBytes(uint8_t *data, size_t size);
    int readBytes(uint8_t offset, uint8_t *data, size_t size);

public:
    Trill();
    ~Trill();

    // Setup functions
    int setup(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, Device device, uint8_t i2c_address = 0x20);

    // Device functions
    int identify();
    Device getDeviceType() const { return device_type; }
    Mode getMode() const { return mode; }
    int setMode(Mode mode);

    // Data functions
    int readI2C();
    unsigned int getNumTouches();
    float touchLocation(uint8_t touch_num);
    float touchSize(uint8_t touch_num);

    // Utility functions
    void printDetails();
};
#endif // TRILL_H
