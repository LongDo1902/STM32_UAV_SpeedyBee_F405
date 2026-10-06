# STM32 UAV Firmware — SpeedyBee F405

This project develops custom UAV flight-controller firmware for the STM32F405 platform used by SpeedyBee F405 boards. Written primarily in C, it combines STM32 HAL peripheral support, FreeRTOS scheduling, and application-specific modules for inertial sensing and motor control. The goal is to understand and build the complete path from a physical sensor measurement to a controlled motor output.

The current development focus is reliable ICM42688 IMU acquisition. An IMU measures acceleration and angular velocity, providing the motion information needed for stabilization. The repository also contains software filtering, cascaded PID controllers, DShot motor-output code, and supporting utilities. These components form a foundation for a flight controller; their presence does not mean the current firmware implements a complete, validated flight-control system.

## How low-level is this project?

Development reaches below application logic into sensor registers, SPI transactions, GPIO interrupts, DMA buffers, hardware timers, and interrupt-safe shared state. STM32 HAL handles peripheral operations, while custom code determines when transfers begin, who owns each buffer, how measurements are decoded, and how failures are recovered.

The ICM42688 driver separates register definitions and access functions from features such as FIFO handling and sensor-data conversion. A FIFO is a buffer inside the sensor that stores measurements until the microcontroller reads them. Configuration involves selecting register banks and setting operating modes, measurement rates, interrupt routing, and FIFO behavior. Raw bytes must then be interpreted and converted into acceleration, angular velocity, and temperature values.

Timing and concurrency are central concerns. Interrupt callbacks and a FreeRTOS task share acquisition state, so critical sections protect ownership changes. The code also accesses Cortex-M interrupt-control instructions and STM32 EXTI registers directly where necessary. This makes the project a practical study of embedded drivers and real-time execution, as well as control-system development.

## How the firmware runs

At startup, `main()` initializes HAL, the clock tree, GPIO, DMA, SPI1, ADC1, UART4, TIM8, and TIM5. It then creates the FreeRTOS objects and starts the scheduler. The active IMU task records its task handle and initializes acquisition using the SPI, chip-select, interrupt-pin, and timer configuration supplied by the application.

The acquisition configuration targets an 8 kHz sensor measurement rate. Two consecutive FIFO frames are averaged into one output, giving a nominal 4 kHz publication rate, or one output every 250 microseconds. These are configured targets; actual timing and reliability require measurement on the target hardware.

The normal acquisition sequence is:

1. The sensor collects measurements until its FIFO reaches the configured watermark of two frames.
2. The sensor signals its interrupt pin. The STM32 EXTI callback reserves a free buffer and starts a SPI DMA read.
3. DMA transfers the FIFO bytes into RAM while the CPU can execute other work.
4. The SPI completion callback marks the buffer ready and sends a FreeRTOS notification to the IMU task.
5. The task wakes, processes available batches, publishes samples, and returns to waiting when no work remains.

Two DMA slots allow acquisition and processing to overlap. A notification signals that work may be available; the measurements themselves remain in the buffers. The task drains available work rather than assuming each notification represents exactly one sample.

Processing decodes FIFO packets, applies calibration and scaling, averages measurements, and attaches timestamps, sequence numbers, and health information. TIM5 supplies a dedicated 1 MHz timebase, allowing timestamps to be measured independently of task scheduling. A latest-sample mailbox provides access for slower consumers.

Error handling tracks DMA failures, unavailable buffers, parsing problems, and unexpected sample intervals. Recovery coordinates with active transfers, resynchronizes FIFO state, and resets timing continuity. Published samples include health and fault fields; publication alone does not guarantee that a sample is healthy.

## Current status and intended outcome

The IMU task and its EXTI/SPI callbacks are connected in `Core/Src/freertos.c`. The branch that receives a published sample is still a placeholder. Software low-pass filtering and cascaded angle/rate PID modules exist, while DShot code provides packet construction and timer/DMA output support. The motor-control task creation is currently commented out.

The intended final outcome is a working stabilization pipeline: acquire motion data, filter it, estimate the required vehicle state, calculate control corrections, mix those corrections into motor commands, and transmit them to ESCs through DShot. Completing this requires integration, measured timing, hardware validation, controller tuning, and defined arming and failsafe behavior. The repository currently demonstrates the underlying architecture and ongoing implementation, rather than verified flight performance.
