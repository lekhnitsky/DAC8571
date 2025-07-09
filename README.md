DAC8571 is a lightweight, STM32 HAL-based C library for driving the Texas Instruments DAC8571 16-bit I²C digital-to-analog converter. It exposes a simple, high-level API—initialization, single-value and array writes, voltage setting, power-down modes, wake-up, reset and raw reads—while handling all of the low-level I²C transactions, error checking and state tracking internally. A built-in self-test routine exercises each function with valid and invalid parameters, printing pass/fail results over `printf()` so you can verify both hardware connectivity and library correctness at startup or on demand.

Integration is straightforward: copy `dac8571.c` and `dac8571.h` into your project, include the header, and ensure your HAL I²C peripheral is up and running. To begin, declare and zero-initialize a `DAC8571_HandleTypeDef`, call

```c
DAC8571_Init(&hdac, &hi2c1, 0x4C);
```

then use `DAC8571_SetVoltage(&dac, voltage)` to drive the output, `DAC8571_Read(&dac)` or `DAC8571_ReadVoltage(&dac, mode, &voltage)` to check out voltage, and `DAC8571_PowerMode`, `DAC8571_Reset`, or `DAC8571_WakeUp` as needed for power management. All HAL errors are converted to human-readable status messages by `HAL_StatusToString()`.

For diagnostics, define `DEBUG_DAC8571` before including the library to enable rich `DEBUG_PRINT()` logs at each step—connection attempts, raw I²C buffers, error codes and retries—without any impact on the API. You can also disable debug entirely by omitting that macro, leaving only the core functionality. The library itself has no RTOS or heap dependencies and is safe to call from both main and interrupt contexts (apart from its own small delays in retries).

This code is distributed under the MIT License—copy, modify and integrate it freely in your STM32CubeIDE or Makefile-based projects. For complete usage examples and wiring diagrams, see the repository’s sample application; for detailed timing and addressing requirements, refer to the DAC8571 datasheet.
