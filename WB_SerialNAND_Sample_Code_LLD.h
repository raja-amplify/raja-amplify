/**
  ******************************************************************************
  * @file    /WB_SerialNAND_Sample_Code_LLD.h
  * @author  Winbond FAE Steam Lin
  * @version V1.0.4
  * @date    11-November-2017
  * @brief   This code provide the low level hardware initial and basic Serial operate function based on STM32F205.
  *            
  * COPYRIGHT 2015 Winbond.
*/ 

// STM32F2 header and SPI hard ware driver. they are not include in the reference code.
/*
#include "stm32f2xx.h"
#include "WB_SPI_Code.h"
*/

/* program function */
uint8_t Program_verify(uint8_t* buff1, uint8_t* buff2, uint32_t count);
void WB_Serial_NAND_Pageprogram_Pattern(uint8_t addh, uint8_t addl, uint8_t* program_buffer, uint32_t count);
void WB_Serial_NAND_Pageprogram_Pattern_QuadInput(uint8_t addh, uint8_t addl, uint8_t* program_buffer, uint32_t count);
void WB_Serial_NAND_Program_Excute(uint8_t addh, uint8_t addl);

/* status check */
uint8_t WB_Check_Embedded_ECC();
uint8_t WB_Check_Program_Erase_Fail_Flag();
uint8_t WB_Read_Serial_NAND_StatusRegister(uint8_t sr_sel);
uint8_t WB_Read_Serial_NAND_ECCRegister(uint8_t sr_sel);
void WB_Serial_NAND_ReadyBusy_Check();
uint32_t WB_NAND_Read_JEDEC_ID();
uint8_t WB_Serial_NAND_bad_block_check(uint32_t page_address);
void WB_Serial_NAND_LUT_Read(uint16_t* LBA, uint16_t* PBA);

/* Stack function for W25M series */
void WB_Die_Select(uint8_t select_die);

/* status set */
void WB_Enable_Embedded_ECC();
void WB_Disable_Embedded_ECC();
void WB_Enable_Buffer_mode();
void WB_Disable_Buffer_mode();
void WB_Serial_NAND_StatusRegister_Write_SR1(uint8_t SR1);
void WB_Serial_NAND_StatusRegister_Write_SR2(uint8_t SR2);
void WB_Serial_NAND_StatusRegister_Write_ECC_Register(uint8_t SR);
void WB_NAND_Reset();
void WB_Protect();
void WB_Unprotect();
void WB_Serial_NAND_LUT_Set(uint16_t LBA, uint16_t PBA);

/* erase function */
void WB_Serial_NAND_BlockErase(uint8_t PA_H, uint8_t PA_L);

/* read function */
void WB_Serial_NAND_PageDataRead(uint8_t PA_H, uint8_t PA_L);
void WB_Serial_NAND_Normal_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count);
void WB_Serial_NAND_QuadIO_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count);
void WB_Serial_NAND_QuadIO_Read_4Byte(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count);
void WB_Serial_NAND_QuadOutput_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count);
void WB_Serial_NAND_QuadOutput_Read_4Byte(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count);
void WB_Serial_NAND_Continuous_Normal_Read(uint8_t* buff, uint32_t count);

/* Hardware Control */
void WB_CS_LOW();
void WB_CS_HIGH();
uint8_t SPIin(uint8_t DI);
uint8_t QSPIin(uint8_t DI);
void PCB_SPI_IO_Init();
void PCB_SPI_Bus_Init();