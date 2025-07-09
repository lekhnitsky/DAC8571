/*
 * @file    dac8571.h
 * @author  lekhnitsky
 * @brief   STM32 HAL-based header for I2C 16-bit DAC DAC8571.
 * @date    2025-01-17
 */

#ifndef INC_DAC8571_H_
#define INC_DAC8571_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief Reference voltage for DAC8571 (in volts).
 */
#define DAC8571_REF_VOLTAGE     2.5 ///< Default reference voltage (adjust as needed)

/**
 * @brief Power-down modes for DAC8571.
 */
#define DAC8571_PD_LOW_POWER    0x00 ///< Low power mode
#define DAC8571_PD_FAST         0x01 ///< Fast recovery mode
#define DAC8571_PD_1_KOHM       0x02 ///< 1 kOhm pull-down
#define DAC8571_PD_100_KOHM     0x03 ///< 100 kOhm pull-down
#define DAC8571_PD_HI_Z         0x04 ///< High-impedance mode

/**
 * @brief Error codes for DAC8571 operations.
 */
#define DAC8571_OK                  0x00 ///< No error
#define DAC8571_I2C_ERROR           0x81 ///< I2C communication error
#define DAC8571_ADDRESS_ERROR       0x82 ///< Invalid address error
#define DAC8571_BUFFER_ERROR        0x83 ///< Buffer overflow error

/**
 * @brief Control byte commands for DAC8571.
 */
#define DAC8571_CMD_WRITE_TMP              0x00 ///< Write to temporary register only (no DAC update)
#define DAC8571_CMD_WRITE_TMP_PWDN         0x01 ///< Write to temporary register with power-down command
#define DAC8571_CMD_WRITE_AND_UPDATE_DAC   0x10 ///< Write to temporary register and update DAC output
#define DAC8571_CMD_WRITE_UPDATE_PWDN      0x11 ///< Write to DAC and enter power-down mode

#define DAC8571_CMD_UPDATE_FROM_TMP        0x20 ///< Update DAC output from temporary register (previously stored data)
#define DAC8571_CMD_BROADCAST_WRITE_TMP    0x30 ///< Broadcast: write to temporary register (all devices)
#define DAC8571_CMD_BROADCAST_WRITE_UPDATE 0x31 ///< Broadcast: write and update all DACs
#define DAC8571_CMD_BROADCAST_PWDN_ALL     0x33 ///< Broadcast: power-down all DACs


/**
 * @brief Handle structure for the DAC8571 digital-to-analog converter.
 */
typedef struct {
    I2C_HandleTypeDef *hi2c; ///< Pointer to the I2C handle
    uint16_t address;         ///< I2C address of the DAC8571
    uint16_t lastValue;      ///< Last written value
    uint8_t writeMode;       ///< Current write mode
    int lastError;           ///< Last error code
} DAC8571_HandleTypeDef;

/**
 * @brief Initialize the DAC8571 handle.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param hi2c Pointer to the I2C handle.
 * @param address I2C address of the DAC8571.
 */
void DAC8571_Init(DAC8571_HandleTypeDef *hdac8571, I2C_HandleTypeDef *hi2c, uint8_t address);

/**
 * @brief Write a value to the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param value 16-bit value to write.
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_Write(DAC8571_HandleTypeDef *hdac8571, uint16_t value);

/**
 * @brief Write an array of values to the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param arr Pointer to the array of 16-bit values to write.
 * @param length Number of values in the array.
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_WriteArray(DAC8571_HandleTypeDef *hdac8571, uint16_t *arr, uint8_t length);

/**
 * @brief Read the last written value from the DAC8571 handle.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return Last written value.
 */
uint16_t DAC8571_Read(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Check if the DAC8571 is connected.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return HAL status indicating if the device is ready.
 */
HAL_StatusTypeDef DAC8571_IsConnected(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Set the DAC output value in volts.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param voltage Output voltage to set (0 to DAC8571_REF_VOLTAGE).
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_SetVoltage(DAC8571_HandleTypeDef *hdac8571, float voltage);

/**
 * @brief Set the write mode for the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param mode Write mode to set (e.g., normal, cache, etc.).
 */
HAL_StatusTypeDef DAC8571_SetWriteMode(DAC8571_HandleTypeDef *hdac8571, uint8_t mode);

/**
 * @brief Get the current write mode of the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return Current write mode.
 */
uint8_t DAC8571_GetWriteMode(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Power mode settings of the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param pdMode Power setting value.
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_PowerMode(DAC8571_HandleTypeDef *hdac8571, uint8_t pdMode);

/**
 * @brief Wake up the DAC8571 from power-down mode.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @param value Value to restore to the DAC after wake-up.
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_WakeUp(DAC8571_HandleTypeDef *hdac8571, uint16_t value);

/**
 * @brief Reset the DAC8571 to its power-on state.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return HAL status of the operation.
 */
HAL_StatusTypeDef DAC8571_Reset(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Get the last error code from the DAC8571 operations.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return Last error code.
 */
int DAC8571_GetLastError(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Get the current I2C address of the DAC8571.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 * @return Current I2C address.
 */
uint8_t DAC8571_GetAddress(DAC8571_HandleTypeDef *hdac8571);

/**
 * @brief Runs a self-test for all DAC8571 functions with valid and invalid parameters.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 */
void DAC8571_SelfTest(DAC8571_HandleTypeDef *hdac8571);

const char* HAL_StatusToString(HAL_StatusTypeDef status);

#ifdef __cplusplus
}
#endif


#endif /* INC_DAC8571_H_ */
