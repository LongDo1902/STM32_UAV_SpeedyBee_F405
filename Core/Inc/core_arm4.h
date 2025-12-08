/*
 * core_arm4.h
 *
 *  Created on: Nov 16, 2025
 *      Author: dobao
 */

#ifndef INC_CORE_ARM4_H_
#define INC_CORE_ARM4_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * ==========================================================
 * 					CORTEX-M4 BASE ADDRESS
 * ==========================================================
 */
#define SCS_BASE_ADDR		0xE000E000u
#define SysTick_BASE_ADDR	(SCS_BASE_ADDR + 0x0010u)
#define NVIC_BASE_ADDR		(SCS_BASE_ADDR + 0x0100u)
#define SCB_BASE_ADDR		(SCS_BASE_ADDR + 0x0D00u)

#define NVIC_REG	((NVIC_RegOffset_t*)	NVIC_BASE_ADDR)
#define SCB_REG		((SCB_RegOffset_t*)		SCB_BASE_ADDR)
#define SysTick_REG	((SysTick_RegOffset_t*)	SysTick_BASE_ADDR)

/*
 * ==========================================================
 * 					CONFIGURATION of CORTEX-M4
 * ==========================================================
 */
#define NVIC_PRIO_BITS			4u	//!Number of priority bits available that a MCU support (e.g. F405 offer 4-bits slot for priority)
#define MPU_PRESENT				1u
#define	Vendor_SysTickConfig	0u
#define FPU_PRESENT				1u
#define NVIC_USER_IRQn_OFFSET	16 	//16 = Vector Table Offset where external interrupts begin
									//Vector Table declares 0-15 are reserved for ARM Cortex-M exceptions

/*
 * ==========================================================
 * 					PRE-DEFINED POSITIONS & MASKS
 * ==========================================================
 */
#define SCB_AIRCR_VECTKEY_Pos_		16u
#define SCB_AIRCR_VECTKEY_Msk_		(0xFFFFu << SCB_AIRCR_VECTKEY_Pos_)

#define SCB_AIRCR_PRIGROUP_Pos_		8u
#define SCB_AIRCR_PRIGROUP_Msk_		(7u << SCB_AIRCR_PRIGROUP_Pos_)

#define SCB_AIRCR_SYSRESETREQ_Pos_	2u
#define SCB_AIRCR_SYSRESETREQ_Msk_	(1u << SCB_AIRCR_SYSRESETREQ_Pos_)

#define SysTick_CTRL_COUNTFLAG_Pos_	16u
#define SysTick_CTRL_COUNTFLAG_Msk_	(1u << SysTick_CTRL_COUNTFLAG_Pos_)	//Status/Flag when SysTick counters reach 0 (COUNTFLAG = 1 -> SysTick hits 0)

#define SysTick_CTRL_CLKSOURCE_Pos_	2u
#define SysTick_CTRL_CLKSOURCE_Msk_ (1u << SysTick_CTRL_CLKSOURCE_Pos_)	//Select which clock the SysTick timer uses (external or processor clock)

#define SysTick_CTRL_TICKINT_Pos_	1u
#define SysTick_CTRL_TICKINT_Msk_	(1u << SysTick_CTRL_TICKINT_Pos_)	//Enable or Disable SysTick Interrupt when the counter hits 0

#define SysTick_CTRL_ENABLE_Pos_	0u
#define SysTick_CTRL_ENABLE_Msk_	1u									//Controls whether SysTick counter is running

#define SysTick_LOAD_RELOAD_Pos_	0u
#define	SysTick_LOAD_RELOAD_Msk_	0xFFFFFFu							//SysTick Load: Reload Mask

#define SysTick_VAL_CURRENT_Pos_	0u
#define	SysTick_VAL_CURRENT_Msk_	0xFFFFFFu							//It shows what number SysTick timer is currently on. It counts down (It acts like a STOP watch)

#define SysTick_CALIB_NOREF_Pos_	31u
#define SysTick_CALIB_NOREF_Msk_	(1u << SysTick_CALIB_NOREF_Pos_)	//NOREF(ReadOnly): if the SysTick has a ref Clock (1 means does not have an external ref clock)

#define SysTick_CALIB_SKEW_Pos_		30u
#define SysTick_CALIB_SKEW_Msk_		(1u << SysTick_CALIB_SKEW_Pos_)		//SKEW(ReadOnly): if = 1 means TENMS value is nnot exact (has error/skew)

#define SysTick_CALIB_TENMS_Pos_	0u
#define SysTick_CALIB_TENMS_Msk_	0xFFFFFFu							//TENMS(ReadOnly) tells us how many SysTick ticks happen in 10ms

/*
 * ==========================================================
 * 					PRE-DEFINED POSITIONS & MASKS
 * ==========================================================
 */
typedef enum{
	NVIC_PRIORITY_GROUP_0_ = 0x00000007u, //0 bits for pre-emption priority
										 //4 bits for subpriority

	NVIC_PRIORITY_GROUP_1_ = 0x00000006u, //1 bits for pre-emption priority
										 //3 bits for subpriority

	NVIC_PRIORITY_GROUP_2_ = 0x00000005u, //2 bits for pre-emption priority
										 //2 bits for subpriority

	NVIC_PRIORITY_GROUP_3_ = 0x00000004u, //3 bits for pre-emption priority
										 //1 bits for subpriority

	NVIC_PRIORITY_GROUP_4_ = 0x00000003u  //4 bits for pre-emption priority
										 //0 bits for subpriority
}NVIC_PriorityGroup_t;

typedef struct{
	volatile uint32_t ISER_[8u];		//0xE000E100	Interrupt Set Enable Reg
	uint32_t RESERVED0[24u];			//Reserved gap	(0xE000E120 to 0xE000E17F)
	volatile uint32_t ICER_[8u];		//0xE000E180	Interrupt Clear-Enable Reg
	uint32_t RESERVED1[24u];			//Reserved gap
	volatile uint32_t ISPR_[8u];		//0xE000E200	Interrupt Set Pending Reg
	uint32_t RESERVED2[24u];			//Reserved gap
	volatile uint32_t ICPR_[8u];		//0xE000E280	Interrupt Clear-Pending Reg
	uint32_t RESERVED3[24u];			//Reserved gap
	volatile uint32_t IABR_[8u];		//0xE000E300	Interrupt Active Bit Reg (Read only)
	uint32_t RESERVED4[56u];			//Reserved gap
	volatile uint8_t IPR_[240u];		//0xE000E400	Interrupt Priority Reg 1 byte each (it is a 4 byte, 60 index)
	uint32_t RESERVED5[644U];
	uint32_t STIR;						//Offset: 0xE00 (Write only) Software Trigger Interrupt Register
}NVIC_RegOffset_t;

typedef struct{
	uint32_t CPUID;			//Offset: 0x00(R/ )
	uint32_t ICSR;			//Offset: 0x04(R/W)  Interrupt Control and State Register
	uint32_t VTOR;			//Offset: 0x08(R/W)  Vector Table Offset Register
	uint32_t AIRCR;			//Offset: 0x0C(R/W)  Application Interrupt and Reset Control Register
	uint32_t SCR;			//Offset: 0x10(R/W)  System Control Register
	uint32_t CCR;			//Offset: 0x14(R/W)  Configuration and Control Register
	uint8_t	 SHPR[12u];		//Offset: 0x18(R/W)  32-bit register SHPR1-2-3, System Handler Priority Registers
	uint32_t SHCSR;			//Offset: 0x24(R/W)  System Handler Control and State Register
	uint32_t CFSR;			//Offset: 0x28(R/W)  Configurable Fault Status Register
	uint32_t HFSR;			//Offset: 0x2C(R/W)  HardFault Status Register
	uint32_t DFSR;			//Offset: 0x30(R/W)  Debug Fault Status Register
	uint32_t MMFAR;			//Offset: 0x34(R/W)  MemManage Address Register
	uint32_t BFAR;			//Offset: 0x38(R/W)  BusFault Address Register
	uint32_t AFSR;			//Offset: 0x3C(R/W)  Auxiliary Fault Status Register

	uint32_t ID_PFR[2u];	//Offset: 0x40(R/ )  Processor Feature Registers
	uint32_t ID_DFR0;		//Offset: 0x48(R/ )  Debug Features Register 0
	uint32_t ID_AFR0;		//Offset: 0x4C(R/ )  Debug Features Register 1
	uint32_t ID_MMFR[4u];	//Offset: 0x50(R/ )  Memory Model Feature Registers
	uint32_t ID_ISAR[5u];	//Offset: 0x60(R/ )  Instruction Set Attributes Registers
	uint32_t RESERVED[5u];
	uint32_t CPACR;			//Offset: 0x88(R/W)  Coprocessor Access Control Register
}SCB_RegOffset_t;

typedef struct{
	uint32_t CTRL;		//Offset 0x00	SysTick Control and Status Register
	uint32_t RELOAD;	//Offset 0x04	SysTick Reload Value Register
	uint32_t CURVAL;	//Offset 0x08	SysTick Current Value Register
	uint32_t CALIB;		//Offset 0x0C	SysTick Calibration Value Register
}SysTick_RegOffset_t;

typedef enum{
	ISER_,
	ICER_,
	ISPR_,
	ICPR_,
	IABR_,
	IPR_
}NVIC_t;

typedef enum{
	/* Cortex-M4 Processor Exceptions Numbers */
	NonMaskableInt_IRQn_	= -14,
	HardFault_IRQn_			= -13,
	MemoryManagement_IRQn_	= -12,
	BusFault_IRQn_			= -11,
	UsageFault_IRQn_		= -10,
	SVCall_IRQn_			= -5,
	DebugMonitor_IRQn_		= -4,
	PendSV_IRQn_			= -2,
	SysTick_IRQn_			= -1,

	/* STM32 Specific Interrupt Numbers */
	WWDG_IRQn_				= 0,	//Window Watchdog Interrupt
	PVD_IRQn_				= 1,	//PVD through EXTI Line detection Interrupt
	TAMP_STAMP_IRQn_		= 2,	//Tamper and TimeStamp interrupts through the EXTI line
	RTC_WKUP_IRQn_			= 3,	//RTC Wakeup interrupt through the EXTI line
	FLASH_IRQn_				= 4,	//FLASH global Interrupt
	RCC_IRQn_				= 5,	//RCC global Interrupt

	EXTI0_IRQn_				= 6,	//EXTI Line0 Interrupt
	EXTI1_IRQn_				= 7,	//EXTI Line1 Interrupt
	EXTI2_IRQn_				= 8,	//EXTI Line2 Interrupt
	EXTI3_IRQn_				= 9,	//EXTI Line3 Interrupt
	EXTI4_IRQn_				= 10,	//EXTI Line4 Interrupt

	DMA1_Stream0_IRQn_		= 11,	//DMA1 Stream0 Global Interrupt
	DMA1_Stream1_IRQn_		= 12,	//DMA1 Stream1 Global Interrupt
	DMA1_Stream2_IRQn_		= 13,	//DMA1 Stream2 Global Interrupt
	DMA1_Stream3_IRQn_		= 14,	//DMA1 Stream3 Global Interrupt
	DMA1_Stream4_IRQn_		= 15,	//DMA1 Stream4 Global Interrupt
	DMA1_Stream5_IRQn_		= 16,	//DMA1 Stream5 Global Interrupt
	DMA1_Stream6_IRQn_		= 17,	//DMA1 Stream6 Global Interrupt

	ADC_IRQn_				= 18,	//ADC1, ADC2 and ADC3 Global Interrupts

	CAN1_TX_IRQn_			= 19,	//CAN1 TX Interrupt
	CAN1_RX0_IRQn_			= 20,	//CAN1 RX0 Interrupt
	CAN1_RX1_IRQn_			= 21,	//CAN1 RX1 Interrupt
	CAN1_SCE_IRQn_			= 22,	//CAN1 SCE Interrupt

	EXTI9_5_IRQn_			= 23,	//External Line[9:5] Interrupt

	TIM1_BRK_TIM9_IRQn_		= 24,	//TIM1 Break Interrupt and TIM9 Global Interrupt
	TIM1_UP_TIM10_IRQn_		= 25,	//TIM1 Update Interrupt and TIM10 Global Interrupt
	TIM1_TRGCOM_TIM11_IRQn_ = 26,	//TIM1 Trigger and Commutation Interrupt and TIM11 Global Interrupt
	TIM1_CC_IRQn_			= 27,	//TIM1 Capture Compare Interrupt
	TIM2_IRQn_				= 28,	//TIM2 Global Interrupt
	TIM3_IRQn_				= 29,	//TIM3 Global Interrupt
	TIM4_IRQn_				= 30,	//TIM4 Global Interrupt

	I2C1_EV_IRQn_			= 31,	//I2C1 Event Interrupt
	I2C1_ER_IRQn_			= 32,	//I2C1 Error Interrupt
	I2C2_EV_IRQn_			= 33,	//I2C2 Event Interrupt
	I2C2_ER_IRQn_			= 34,	//I2C2 Error Interrupt

	SPI1_IRQn_				= 35,	//SPI1 Global Interrupt
	SPI2_IRQn_				= 36,	//SPI2 Global Interrupt

	USART1_IRQn_			= 37,	//USART1 Global Interrupt
	USART2_IRQn_			= 38, 	//USART2 Global Interrupt
	USART3_IRQn_			= 39,	//USART3 Global Interrupt

	EXTI15_10_IRQn_			= 40, 	//External Line [15:10] Interrupts

	RTC_ALARM_IRQn_			= 41,	//RTC ALARM (A and B) through EXTI Line Interrupt
	OTG_FS_WKUP_IRQn_		= 42, 	//USB OTG FS Wakeup through EXTI Line Interrupt

	TIM8_BRK_TIM12_IRQn_	= 43,	//TIM8 Break Interrupt and TIM12 Global Interrupt
	TIM8_UP_TIM13_IRQn_		= 44,	//TIM8 Update Interrupt and TIM13 Global Interrupt
	TIM8_TRGCOM_TIM14_IRQn_ = 45,	//TIM8 Trigger and Commutation Interrupt and TIM14 Global Interrupt
	TIM8_CC_IRQn_			= 46,	//TIM8 Capture Compare Global Interrupt

	DMA1_Stream7_IRQn_		= 47,	//DMA1 Stream7 Interrupt

	FSMC_IRQn_				= 48,	//FSMC Global Interrupt
	SDIO_IRQn_				= 49,	//SDIO Global Interrupt

	TIM5_IRQn_				= 50,	//TIM5 Global Interrupt
	SPI3_IRQn_				= 51,	//SPI3 Global Interrupt
	UART4_IRQn_				= 52,	//UART4 Global Interrupt
	UART5_IRQn_				= 53, 	//UART5 Global interrupt
	TIM6_DAC_IRQn_			= 54,	//TIM6 Global and DAC1&2 underrun error Interrupts
	TIM7_IRQn_				= 55,	//TIM7 global interrupt

	DMA2_Stream0_IRQn_		= 56,	//DMA2 Stream0 Global Interrupt
	DMA2_Stream1_IRQn_		= 57,	//DMA2 Stream1 Global Interrupt
	DMA2_Stream2_IRQn_		= 58,	//DMA2 Stream2 Global Interrupt
	DMA2_Stream3_IRQn_		= 59,	//DMA2 Stream3 Global Interrupt
	DMA2_Stream4_IRQn_		= 60,	//DMA2 Stream4 Global Interrupt

	ETH_IRQn_				= 61,	//Ethernet Global Interrupt
	ETH_WKUP_IRQn_			= 62,	//Ethernet Wakeup Global Interrupt

	CAN2_TX_IRQn_			= 63,	//CAN2 TX Global Interrupt
	CAN2_RX0_IRQn_			= 64,	//CAN2 RX0 Global Interrupt
	CAN2_RX1_IRQn_			= 65,	//CAN2 RX1 Global Interrupt
	CAN2_SCE_IRQn_			= 66,	//CAN2 SCE Interrupt

	OTG_FS_IRQn_			= 67,	//USB OTG FS Global Interrupt

	DMA2_Stream5_IRQn_		= 68,	//DMA2 Stream5 Global Interrupt
	DMA2_Stream6_IRQn_		= 69, 	//DMA2 Stream6 Global interrupt
	DMA2_Stream7_IRQn_		= 70, 	//DMA2 Stream7 Global Interrupt

	USART6_IRQn_			= 71, 	//USART6 Global Interrupt

	I2C3_EV_IRQn_			= 72,	//I2C3 Event Interrupt
	I2C3_ER_IRQn_			= 73, 	//I2C3 Error Interrupt

	OTG_HS_EP1_OUT_IRQn_	= 74,	//USB OTG HS End Point 1 Out Global Interrupt
	OTG_HS_EP1_IN_IRQn_		= 75, 	//USB OTG HS End Point 1 In Global Interrupt
	OTG_HS_WKUP_IRQn_		= 76, 	//USB OTG HS Wakeup through EXTI Interrupt
	OTG_HS_IRQn_			= 77, 	//USB OTG HS Global Interrupt

	DCMI_IRQn_				= 78,	//DCMI Global Interrupt (Digital Camera Interface)
	CRYP_IRQn_				= 79,	//CRYP crypto Global Interrupt
	RNG_IRQn_				= 80, 	//RNG Global Interrupt
	FPU_IRQn_				= 81,	//FBU Global Interrupt

	IRQn_COUNT				= FPU_IRQn_ + 1 //Used for valid check
}IRQn_t;

/*
 * =====================================================================
 * 						FUNCTIONS DECLARATIONS
 * =====================================================================
 */
void NVIC_setPriorityGrouping(NVIC_PriorityGroup_t priorityGrouping);
NVIC_PriorityGroup_t NVIC_getPriorityGrouping(void);

bool NVIC_enableIRQ(IRQn_t irqNumber);
bool NVIC_getEnableIRQ(IRQn_t irqNumber, uint32_t* irqStatus);

bool NVIC_disableIRQ(IRQn_t irqNumber);
uint32_t NVIC_getPendingIRQ(IRQn_t irqNumber);

bool NVIC_setPendingIRQ(IRQn_t irqNumber);
bool NVIC_clearPendingIRQ(IRQn_t irqNumber);

uint32_t NVIC_getActiveInterrupt(IRQn_t irqNumber);
bool NVIC_setPriorityIRQ(IRQn_t irqNumber, uint32_t priority);

uint32_t NVIC_getPriorityIRQ(IRQn_t irqNumber);
uint32_t NVIC_encodePriority(NVIC_PriorityGroup_t priorityGrouping, uint32_t preemptPriority, uint32_t subPriority);

void NVIC_decodePriority(NVIC_PriorityGroup_t priorityGroup, uint32_t priority, uint32_t* const preemptPriority, uint32_t* const subPriority);
bool NVIC_changeExternalIRQHandlerAddr(IRQn_t irqNumber, void (*newIRQHandlerAddr)(void));

uint32_t NVIC_getExternalIRQHandlerAddr(IRQn_t irqNumber);
bool NVIC_changeExceptionIRQHandlerAddr(IRQn_t irqNumber, void (*newIRQHandlerAddr)(void));

uint32_t NVIC_getExceptionIRQHandlerAddr(IRQn_t irqNumber);
__attribute__((__noreturn__)) void NVIC_SystemReset(void);
bool SysTick_config(uint32_t ticks);

#endif /* INC_CORE_ARM4_H_ */














