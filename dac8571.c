/*
 * @file    dac8571.c
 * @author  lekhnitsky
 * @brief   STM32 HAL-based library implementation for I2C 16-bit DAC DAC8571.
 * @date    2025-01-17
 */

#include "dac8571.h"
#include <stdio.h>
#include <stdbool.h>

// Enable or disable debug mode
#define DEBUG_DAC8571

#ifdef DEBUG_DAC8571
  #include <stdio.h>
  #define DEBUG_PRINT(fmt, ...)  \
      do {                       \
          printf((fmt), ##__VA_ARGS__); \
      } while (0)
#else
  #define DEBUG_PRINT(fmt, ...)  \
      do { /* nothing */         \
      } while (0)
#endif


void DAC8571_Init(DAC8571_HandleTypeDef *hdac8571, I2C_HandleTypeDef *hi2c, uint8_t address) {
	const int max_attempts = 5;
	const uint32_t retry_delay_ms = 25;

	if (!hdac8571 || !hi2c) {
        DEBUG_PRINT("Error: Invalid handle or I2C pointer in DAC8571_Init\r\n");
        return;
    }

    if ((address != 0x4C) && (address != 0x4E)){
    	DEBUG_PRINT("Error: Invalid I2C address in DAC8571_Init\r\n");
    	return;
    }

    hdac8571->hi2c = hi2c;
    hdac8571->address = (uint16_t)address;
    hdac8571->lastValue = 0;
    hdac8571->writeMode = DAC8571_CMD_WRITE_AND_UPDATE_DAC;
    hdac8571->lastError = DAC8571_OK;

    if (__HAL_I2C_GET_FLAG(hdac8571->hi2c, I2C_FLAG_BUSY)) {
        __HAL_I2C_CLEAR_FLAG(hdac8571->hi2c, I2C_FLAG_BUSY);
    }

    // Try to detect the device multiple times
    bool connected = false;
    for (int attempt = 1; attempt <= max_attempts; attempt++) {
        if (DAC8571_IsConnected(hdac8571) == HAL_OK) {
            DEBUG_PRINT("DAC8571 connection attempt %d: OK\r\n", attempt);
            connected = true;
            break;
        } else {
            DEBUG_PRINT("DAC8571 connection attempt %d: FAIL\r\n", attempt);
            HAL_Delay(retry_delay_ms);
        }
    }

    if (!connected) {
        DEBUG_PRINT("Error: DAC8571 not responding after %d attempts!\r\n", max_attempts);
        //hdac8571->lastError = DAC8571_ERROR_NOT_CONNECTED;
        return;
    }

    DEBUG_PRINT("DAC8571_Init successful\r\n");
}

HAL_StatusTypeDef DAC8571_Write(DAC8571_HandleTypeDef *hdac8571, uint16_t value) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_Write\r\n");
        return HAL_ERROR;
    }

    uint8_t buffer[3] = {hdac8571->writeMode, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    //DEBUG_PRINT("Data on input: 0x%04X \r\n", value);
    //DEBUG_PRINT("Data to send:  0x%02X 0x%02X 0x%02X\r\n", buffer[0], buffer[1], buffer[2]);

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(hdac8571->hi2c, hdac8571->address << 1, buffer, sizeof(buffer), 100);
    if (status != HAL_OK) {
        hdac8571->lastError = DAC8571_I2C_ERROR;
        DEBUG_PRINT("Error: Failed to write value 0x%04X to DAC8571 at address 0x%02X. ERROR = %s \r\n", value, hdac8571->address, HAL_StatusToString(status));
        return status;
    }

    hdac8571->lastValue = value;
    hdac8571->lastError = DAC8571_OK;
    return HAL_OK;
}

HAL_StatusTypeDef DAC8571_IsConnected(DAC8571_HandleTypeDef *hdac8571) {
//    if (!hdac8571) {
//        DEBUG_PRINT("Error: Invalid handle in DAC8571_IsConnected\r\n");
//        return HAL_ERROR;
//    }
//
//    HAL_StatusTypeDef status = DAC8571_Read(hdac8571);
//    if (status != HAL_OK) {
//        hdac8571->lastError = DAC8571_I2C_ERROR;
//        DEBUG_PRINT("Error: DAC8571 not responding at address 0x%02X\r\n", hdac8571->address);
//    }
	if (!hdac8571) {
		printf("DAC8571_IsConnected: invalid handle\r\n");
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(
		hdac8571->hi2c,
		hdac8571->address << 1,
		1,
		10
	);

	if (status != HAL_OK) {
		hdac8571->lastError = DAC8571_I2C_ERROR;
		DEBUG_PRINT("Error: DAC8571 not responding at address 0x%02X\r\n", hdac8571->address);
	}

	//printf("DAC8571_IsConnected returned: %s (0x%02X)\r\n", HAL_StatusToString(status), (uint8_t)status);
    return status;
}

HAL_StatusTypeDef DAC8571_WriteArray(DAC8571_HandleTypeDef *hdac8571, uint16_t *arr, uint8_t length) {
    if (!hdac8571 || !arr || length == 0) {
        DEBUG_PRINT("Error: Invalid parameters in DAC8571_WriteArray\r\n");
        return HAL_ERROR;
    }

    if (length > 14) {
        hdac8571->lastError = DAC8571_BUFFER_ERROR;
        DEBUG_PRINT("Error: Buffer overflow in DAC8571_WriteArray\r\n");
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;
    for (uint8_t i = 0; i < length; i++) {
        status = DAC8571_Write(hdac8571, arr[i]);
        if (status != HAL_OK) {
            return status;
        }
    }

    return HAL_OK;
}

uint16_t DAC8571_Read(DAC8571_HandleTypeDef *hdac8571) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_Read\r\n");
        return 0;
    }

    uint8_t received_data[3] = {0}; // Buffer for received data
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
            hdac8571->hi2c,
            hdac8571->address << 1| 0x01,  // адрес + бит чтения
			received_data,
            3,
            HAL_MAX_DELAY
        );
    //DEBUG_PRINT("Received data: 0x%02X 0x%02X 0x%02X \r\n", received_data[0], received_data[1], received_data[2]);

    if (status != HAL_OK) {
        hdac8571->lastError = DAC8571_I2C_ERROR;
        DEBUG_PRINT("Error: I2C Read Failed from DAC8571 at address 0x%02X, ERROR: %s \r\n", hdac8571->address, HAL_StatusToString(status));
        return 0;
    }

    uint16_t value = (received_data[0] << 8) | received_data[1]; // Combine high and low bytes
    hdac8571->lastError = DAC8571_OK;

    //DEBUG_PRINT("DAC8571_Read: 0x%04X\r\n", value);
    return value;
}



HAL_StatusTypeDef DAC8571_SetVoltage(DAC8571_HandleTypeDef *hdac8571, float voltage) {
    if (!hdac8571 || voltage < 0.0f || voltage > DAC8571_REF_VOLTAGE) {
        DEBUG_PRINT("Error: Invalid voltage parameter in DAC8571_SetVoltage\r\n");
        return HAL_ERROR;
    }

    uint16_t value = (uint16_t)((voltage / DAC8571_REF_VOLTAGE) * 65535);
    return DAC8571_Write(hdac8571, value);
}

HAL_StatusTypeDef DAC8571_SetWriteMode(DAC8571_HandleTypeDef *hdac8571, uint8_t mode) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_SetWriteMode\r\n");
        return HAL_ERROR;
    }

    switch (mode) {
        case DAC8571_CMD_WRITE_TMP:
        case DAC8571_CMD_WRITE_TMP_PWDN:
        case DAC8571_CMD_WRITE_AND_UPDATE_DAC:
        case DAC8571_CMD_WRITE_UPDATE_PWDN:
        case DAC8571_CMD_UPDATE_FROM_TMP:
        case DAC8571_CMD_BROADCAST_WRITE_TMP:
        case DAC8571_CMD_BROADCAST_WRITE_UPDATE:
        case DAC8571_CMD_BROADCAST_PWDN_ALL:
            hdac8571->writeMode = mode;
            return HAL_OK;

        default:
            DEBUG_PRINT("Error: Invalid mode in DAC8571_SetWriteMode: 0x%02X\r\n", mode);
            return HAL_ERROR;
    }
}

uint8_t DAC8571_GetWriteMode(DAC8571_HandleTypeDef *hdac8571) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_GetWriteMode\r\n");
        return 0;
    }
    return hdac8571->writeMode;
}

HAL_StatusTypeDef DAC8571_PowerMode(DAC8571_HandleTypeDef *hdac8571, uint8_t pdMode) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_PowerMode\r\n");
        return HAL_ERROR;
    }

    hdac8571->writeMode = DAC8571_CMD_WRITE_TMP_PWDN;
    uint16_t pdValue = 0;

    switch (pdMode) {
        case DAC8571_PD_LOW_POWER:   pdValue = (0b000 << 13); break;
        case DAC8571_PD_FAST:        pdValue = (0b001 << 13); break;
        case DAC8571_PD_1_KOHM:      pdValue = (0b010 << 13); break;
        case DAC8571_PD_100_KOHM:    pdValue = (0b110 << 13); break;
        case DAC8571_PD_HI_Z:        pdValue = (0b111 << 13); break;
        default:
            DEBUG_PRINT("Error: Invalid power-down mode in DAC8571_PowerMode\r\n");
            return HAL_ERROR;
    }
    return DAC8571_Write(hdac8571, pdValue);
}


HAL_StatusTypeDef DAC8571_WakeUp(DAC8571_HandleTypeDef *hdac8571, uint16_t value) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_WakeUp\r\n");
        return HAL_ERROR;
    }

    return DAC8571_Write(hdac8571, value);
}

HAL_StatusTypeDef DAC8571_Reset(DAC8571_HandleTypeDef *hdac8571) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_Reset\r\n");
        return HAL_ERROR;
    }

    return DAC8571_Write(hdac8571, 0);
}

int DAC8571_GetLastError(DAC8571_HandleTypeDef *hdac8571) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_GetLastError\r\n");
        return DAC8571_ADDRESS_ERROR;
    }

    int error = hdac8571->lastError;
    hdac8571->lastError = DAC8571_OK;
    return error;
}

uint8_t DAC8571_GetAddress(DAC8571_HandleTypeDef *hdac8571) {
    if (!hdac8571) {
        DEBUG_PRINT("Error: Invalid handle in DAC8571_GetAddress\r\n");
        return 0;
    }
    return hdac8571->address;
}

/**
 * @brief Runs a self-test for all DAC8571 functions with valid and invalid parameters.
 * @param hdac8571 Pointer to the DAC8571 handle structure.
 */
void DAC8571_SelfTest(DAC8571_HandleTypeDef *hdac8571) {
    printf("\r\n===================================\r\n");
    printf("        DAC8571 SELF-TEST\r\n");
    printf("===================================\r\n");

    int passedTests = 0;
    int failedTests = 0;
    HAL_StatusTypeDef status;

    float voltages[] = {-2.0f, 0.0f, 1.25f, 2.0f, 3.3f, /*DAC8571_REF_VOLTAGE + 0.1f*/};
    uint16_t testArray[] = {0x0000, 0x8000, 0xFFFF};
    uint8_t allWriteModes[] = {
        DAC8571_CMD_WRITE_TMP,
        DAC8571_CMD_WRITE_TMP_PWDN,
        DAC8571_CMD_WRITE_AND_UPDATE_DAC,
        DAC8571_CMD_WRITE_UPDATE_PWDN,
        DAC8571_CMD_UPDATE_FROM_TMP,
        DAC8571_CMD_BROADCAST_WRITE_TMP,
        DAC8571_CMD_BROADCAST_WRITE_UPDATE,
        DAC8571_CMD_BROADCAST_PWDN_ALL,
        0xFF // Invalid
    };
    uint8_t powerModes[] = {
        DAC8571_PD_LOW_POWER,
        DAC8571_PD_FAST,
        DAC8571_PD_1_KOHM,
        DAC8571_PD_100_KOHM,
        DAC8571_PD_HI_Z,
        0xFF // Invalid
    };

    printf("\r\n[1] Voltage Write Tests\r\n-----------------------------------\r\n");
    for (size_t i = 0; i < sizeof(voltages) / sizeof(voltages[0]); i++) {
        status = DAC8571_SetVoltage(hdac8571, voltages[i]);
        if ((voltages[i] >= 0.0f && voltages[i] <= DAC8571_REF_VOLTAGE && status == HAL_OK) ||
            ((voltages[i] < 0.0f || voltages[i] > DAC8571_REF_VOLTAGE) && status != HAL_OK)) {
            printf("[PASSED] SetVoltage(%.2fV)\r\n", voltages[i]);
            passedTests++;
        } else {
            printf("[FAILED] SetVoltage(%.2fV)\r\n", voltages[i]);
            failedTests++;
        }
    }

    printf("\r\n[2] Write Mode Tests\r\n-----------------------------------\r\n");
    for (size_t i = 0; i < sizeof(allWriteModes); i++) {
        status = DAC8571_SetWriteMode(hdac8571, allWriteModes[i]);
        if ((allWriteModes[i] <= DAC8571_CMD_BROADCAST_PWDN_ALL && status == HAL_OK) ||
            (allWriteModes[i] > DAC8571_CMD_BROADCAST_PWDN_ALL && status != HAL_OK)) {
            printf("[PASSED] SetWriteMode(0x%02X)\r\n", allWriteModes[i]);
            passedTests++;
        } else {
            printf("[FAILED] SetWriteMode(0x%02X)\r\n", allWriteModes[i]);
            failedTests++;
        }
    }

    printf("\r\n[3] Power Mode Tests\r\n-----------------------------------\r\n");
    for (size_t i = 0; i < sizeof(powerModes); i++) {
        status = DAC8571_PowerMode(hdac8571, powerModes[i]);
        if ((powerModes[i] <= DAC8571_PD_HI_Z && status == HAL_OK) ||
            (powerModes[i] > DAC8571_PD_HI_Z && status != HAL_OK)) {
            printf("[PASSED] PowerMode(0x%02X)\r\n", powerModes[i]);
            passedTests++;
        } else {
            printf("[FAILED] PowerMode(0x%02X)\r\n", powerModes[i]);
            failedTests++;
        }
    }

    printf("\r\n[4] Array Write Test\r\n-----------------------------------\r\n");
    status = DAC8571_WriteArray(hdac8571, testArray, 3);
    if (status == HAL_OK) {
        printf("[PASSED] WriteArray(valid)\r\n");
        passedTests++;
    } else {
        printf("[FAILED] WriteArray(valid)\r\n");
        failedTests++;
    }

    status = DAC8571_WriteArray(hdac8571, testArray, 20);
    if (status != HAL_OK) {
        printf("[PASSED] WriteArray(overflow)\r\n");
        passedTests++;
    } else {
        printf("[FAILED] WriteArray(overflow)\r\n");
        failedTests++;
    }

    printf("\r\n[5] Read / Write / Reset Tests\r\n-----------------------------------\r\n");
    status = DAC8571_Write(hdac8571, 0x8000);
    if (status == HAL_OK) {
        printf("[PASSED] Write(0x8000)\r\n");
        passedTests++;
    } else {
        printf("[FAILED] Write(0x8000)\r\n");
        failedTests++;
    }

    uint16_t readVal = DAC8571_Read(hdac8571);
    printf("Read Value: 0x%04X\r\n", readVal);

    status = DAC8571_Reset(hdac8571);
    if (status == HAL_OK) {
        printf("[PASSED] Reset\r\n");
        passedTests++;
    } else {
        printf("[FAILED] Reset\r\n");
        failedTests++;
    }

    printf("\r\n[6] Wake-Up Test\r\n-----------------------------------\r\n");
    status = DAC8571_WakeUp(hdac8571, 0x8000);
    if (status == HAL_OK) {
        printf("[PASSED] WakeUp\r\n");
        passedTests++;
    } else {
        printf("[FAILED] WakeUp\r\n");
        failedTests++;
    }

    printf("\r\n[7] Get Functions\r\n-----------------------------------\r\n");
    printf("GetAddress: 0x%02X\r\n", DAC8571_GetAddress(hdac8571));
    printf("GetWriteMode: 0x%02X\r\n", DAC8571_GetWriteMode(hdac8571));
    printf("GetLastError: %d\r\n", DAC8571_GetLastError(hdac8571));

    printf("\r\n===================================\r\n");
    printf("DAC8571 SELF-TEST COMPLETED\r\n");
    printf("Total: %d | Passed: %d | Failed: %d\r\n", passedTests + failedTests, passedTests, failedTests);
    printf("===================================\r\n");

    DAC8571_SetVoltage(hdac8571, 0.0f);

}

const char* HAL_StatusToString(HAL_StatusTypeDef status) {
    switch (status) {
        case HAL_OK:       return "HAL_OK";
        case HAL_ERROR:    return "HAL_ERROR";
        case HAL_BUSY:     return "HAL_BUSY";
        case HAL_TIMEOUT:  return "HAL_TIMEOUT";
        default:           return "UNKNOWN_STATUS";
    }
}

