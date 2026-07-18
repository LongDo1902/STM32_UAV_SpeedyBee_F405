// #include "dshot_task.h"

// /**
//  * @brief  Function implementing the DshotMotorControlTask thread.
//  * @param  argument: Not used
//  * @retval None
//  */
// void
// DshotMotorControlTask(void *argument)
// {
//     uint32_t flags;

//     for (;;) {
//         // (0x01) wait PID loop
//         flags = osThreadFlagsWait(0x00000001U, osFlagsWaitAny, osWaitForever);
//         // PID trigger by: osThreadFlagsSet(dshotMotorTaskHandle, 0x00000001U);
//         if (flags > 0) {
//             DShot_Update();  // update frame DShot
//             DShot_Trigger(); // trigger DMA pwm to Motor
//         }
//     }
// }
