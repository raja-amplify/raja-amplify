/**
  ******************************************************************************
  * @file    /WB_SerialNAND_Sample_Code_LLD.c
  * @author  Winbond FAE Steam Lin
  * @version V1.0.10
  * @date    11-November-2017
  * @brief   This code provide the low level hardware initial and basic Serial operate function based on STM32F205.
  *            
  * COPYRIGHT 2015 Winbond.
*/ 
#include "WB_SerialNAND_Sample_Code_LLD.h"

/********************
Function: Serial NAND continuous read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_Continuous_Normal_Read(uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0x03); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = SPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: W25M series (SPISTACK) die select 
Argument: 
	Select_die
		0x00  (power on default states)
		0x01
return:
*********************/
void WB_Die_Select(uint8_t select_die){

  /*	reference code to avoid incorrect select die ID input
  switch(select_die){
	  case 0x00:
		break;
	  case 0x01:
		break;
	  default:
		return;
  }
  */	
  
  WB_CS_LOW();
  SPIin(0xC2);		// Software Die Select
  SPIin(select_die);
  WB_CS_HIGH();
  
  return;
}

/********************
Function: Serial NAND BBM Set LUT
Argument: 
  LBA                   Logical block address
  PBA                   Physical block address
  * LBA or PBA          0       1       2
  * Page address        0       0x40   0x80
  return:
*********************/
void WB_Serial_NAND_LUT_Set(uint16_t LBA, uint16_t PBA){ 
  
  WB_CS_LOW();
  SPIin(0x06);
  WB_CS_HIGH();
  
  WB_CS_LOW();
  SPIin(0xA1);
  SPIin((LBA/0x100));
  SPIin((LBA%0x100));
  SPIin((PBA/0x100));
  SPIin((PBA%0x100));
  WB_CS_HIGH();
}

/********************
Function: Serial NAND LUT read
Argument: 
  uint16_t LBA[20];             Logical block address
  uint16_t PBA[20];             Physical block address 
  For Winbond 1Gb NAND, LBA[9:0] & PBA [9:0] are effective block address. LBA[15:14] is used for additional information
return:
*********************/
void WB_Serial_NAND_LUT_Read(uint16_t* LBA, uint16_t* PBA){
  uint16_t i, buf1, buf2;  
  WB_CS_LOW();
  SPIin(0xA5);
  SPIin(0x00);                  // Dummy
  for(i = 0; i < 20; i++){
    buf1 = SPIin(0x00);
    buf2 = SPIin(0x00);
    *(LBA+i) = (buf1 << 8) | buf2;
    buf1 = SPIin(0x00);
    buf2 = SPIin(0x00);
    *(PBA+i) = (buf1 << 8) | buf2;
  }
  WB_CS_HIGH();
  return;
}

/********************\
Function: Serial NAND Bad block mark check
Argument: 
return:
1: Check block is bad block.
0: Check block is not bad block.
update: V.1.0.8 // correct the bad block mark address
*********************/
uint8_t WB_Serial_NAND_bad_block_check(uint32_t page_address){
  uint8_t *read_buf;
  uint8_t EPR_status;
  WB_Serial_NAND_PageDataRead(page_address/0x100, page_address%0x100);                  // Read the first page of a block
  
  /* if build-in ECC algorithm enable
  EPR_status = WB_Check_Embedded_ECC_Flag();
  if(EPR_status > 1){
	  return 1;															// Check ECC status and return fail if (ECC-1, ECC0) = (1,0) or (1, 1)
		  }
  */
  
  WB_Serial_NAND_Normal_Read(0x8, 0x0, read_buf, 1);					// Read bad block mark at 0x800 update at v.1.0.8
  if(*read_buf != 0xFF){												// update at v.1.0.7
    return 1;
    }
	
  return 0;
}



/********************
Function: Program data verify, works as the same as memcmp
return:
0: no mismatch
1: mismatch
*********************/
uint8_t Program_verify(uint8_t* buff1, uint8_t* buff2, uint32_t count){
  uint32_t i = 0;
  for( i = 0; i < count; i++){
    if( *(buff1+i) != *(buff2+i)){
      return 1;
      }
    }
  return 0;
}

/********************
Function: Serial NAND page program
Argument: 
addh, addl: input address
program_buffer: input data
count: program count
return:
*********************/
void WB_Serial_NAND_Pageprogram_Pattern(uint8_t addh, uint8_t addl, uint8_t* program_buffer, uint32_t count){  
  uint32_t i = 0;  
  
  WB_CS_LOW();
  SPIin(0x06);
  WB_CS_HIGH();
  
  WB_CS_LOW();
  SPIin(0x02);
  SPIin(addh);
  SPIin(addl); 
  for(i = 0; i < count; i++){
    SPIin(*(program_buffer+i));
  }
  WB_CS_HIGH();    

  return;
}

/********************
Function: Serial NAND page quad program
Argument: 
addh, addl: input address
program_buffer: input data
count: program count
return:
*********************/
void WB_Serial_NAND_Pageprogram_Pattern_QuadInput(uint8_t addh, uint8_t addl, uint8_t* program_buffer, uint32_t count){  
  uint32_t i = 0;  
  
  WB_CS_LOW();
  SPIin(0x06);
  WB_CS_HIGH();
  
  WB_CS_LOW();
  SPIin(0x32);
  SPIin(addh);
  SPIin(addl); 
  for(i = 0; i < count; i++){
    QSPIin(*(program_buffer+i));
  }
  WB_CS_HIGH();    

  return;
}

/********************
Function: Serial NAND page program
Argument: 
addh, addl: input address
pattern: program data
count: program count
return: ready busy count
*********************/
void WB_Serial_NAND_Program_Excute(uint8_t addh, uint8_t addl){  
  
  WB_CS_LOW();
  SPIin(0x10);
  SPIin(0x00); // dummy
  SPIin(addh);
  SPIin(addl); 
  WB_CS_HIGH();    
  WB_Serial_NAND_ReadyBusy_Check();
  
  return;
}

/********************
Function: Do whole Flash protect
Argument: 
*********************/
void WB_Protect(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(1); 	// Read status register 2
  SR|=0x7C;										// Enable ECC-E bit
  WB_Serial_NAND_StatusRegister_Write_SR1(SR);
  return;
}

/********************
Function: Do whole Flash unprotect
Argument: 
*********************/
void WB_Unprotect(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(1); 	// Read status register 2
  SR&=0x83;										// Enable ECC-E bit
  WB_Serial_NAND_StatusRegister_Write_SR1(SR);
  return;
}

/********************
Function: Check ECC-E status
Argument: 
Comment: Change function name at V1.0.3
*********************/
uint8_t WB_Check_Embedded_ECC_Enable(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(2);  // Read status register 2
  return (SR&0x10)>>4;
}

/********************
Function: Check P-FAIL\E-FAIL status
Argument: 
Comment: Modify this function at V1.0.3
*********************/
uint8_t WB_Check_Program_Erase_Fail_Flag(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(3); 	// Read status register 3
  return (SR&0x0C)>>2;							// Check P-Fail, E-Fail bit								
}

/********************
Function: Check ECC-1, ECC-0 status
Argument: 
Comment: Add this function at V1.0.3
*********************/
uint8_t WB_Check_Embedded_ECC_Flag(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(3); 	// Read status register 3
  return (SR&0x30)>>4;							// Check ECC-1, ECC0 bit								
}

/********************
Function: Enable embedded ECC
Argument: 
*********************/
void WB_Enable_Embedded_ECC(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(2); 	// Read status register 2
  SR|=0x10;										// Enable ECC-E bit
  WB_Serial_NAND_StatusRegister_Write_SR2(SR);
  return;
}

/********************
Function: Disable embedded ECC
Argument: 
*********************/
void WB_Disable_Embedded_ECC(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(2); 	// Read status register 2
  SR&= 0xEF;									// Disable ECC-E bit
  WB_Serial_NAND_StatusRegister_Write_SR2(SR);
  return;
}

/********************
Function: Enable buffer mode
Argument: 
*********************/
void WB_Enable_Buffer_mode(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(2); 	// Read status register 2
  SR|=0x08;										// Enable BUF bit
  WB_Serial_NAND_StatusRegister_Write_SR2(SR);
  return;
}

/********************
Function: Disable buffer mode
Argument: 
*********************/
void WB_Disable_Buffer_mode(){
  uint8_t SR;
  SR = WB_Read_Serial_NAND_StatusRegister(2); 	// Read status register 2
  SR&= 0xF7;									// Disable BUF bit
  WB_Serial_NAND_StatusRegister_Write_SR2(SR);
  return;
}


/********************
Function: Serial NAND Write Status register 1 
Argument: 
SR1: program SR1 data
*********************/
void WB_Serial_NAND_StatusRegister_Write_SR1(uint8_t SR1){
  WB_CS_LOW();
  SPIin(0x01);
  SPIin(0xA0);
  SPIin(SR1);
  WB_CS_HIGH();
  WB_Serial_NAND_ReadyBusy_Check();
  return;
}

/********************
Function: Serial NAND Write Status register 2 
Argument: 
SR2: program SR2 data
*********************/
void WB_Serial_NAND_StatusRegister_Write_SR2(uint8_t SR2){  

  WB_CS_LOW();
  SPIin(0x01);
  SPIin(0xB0);
  SPIin(SR2);
  WB_CS_HIGH();
  WB_Serial_NAND_ReadyBusy_Check();
  return;
}

/********************
Function: Serial NAND Write ECC register
Argument: 
SR: program ECC register value 
*********************/
void WB_Serial_NAND_StatusRegister_Write_ECC_Register(uint8_t SR){  

  WB_CS_LOW();
  SPIin(0x01);
  SPIin(0x10);
  SPIin(SR);
  WB_CS_HIGH();
  WB_Serial_NAND_ReadyBusy_Check();
  return;
}

/********************
Function: Serial NAND Block erase
Argument:
PA_H, PA_L: Page address
return:
*********************/
void WB_Serial_NAND_BlockErase(uint8_t PA_H, uint8_t PA_L){  

  WB_CS_LOW();
  SPIin(0x06);
  WB_CS_HIGH();
  
  WB_CS_LOW();
  SPIin(0xD8);
  SPIin(0x00); // dummy
  SPIin(PA_H);
  SPIin(PA_L);  
  WB_CS_HIGH();  
  
  WB_Serial_NAND_ReadyBusy_Check();
}

/********************
Function: Serial NAND Status register read
Argument: 
sr_sel: select register
return: status register value
*********************/
uint8_t WB_Read_Serial_NAND_StatusRegister(uint8_t sr_sel){
  uint8_t SR = 0;  // status register data
  switch(sr_sel){
  case 0x01:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0xA0); // SR1
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x02:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0xB0); // SR2
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x03:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0xC0); // SR3
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  default:
    SR = 0xFF;
    break;
  }    
  return SR;
}

/********************
Function: Serial NAND Status Extend internal ECC register read
Argument: 
sr_sel: select register
return: status register value
*********************/
uint8_t WB_Read_Serial_NAND_ECCRegister(uint8_t sr_sel){
  uint8_t SR = 0;  // status register data
  switch(sr_sel){
  case 0x01:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0x10); // SR1
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x02:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0x20); // SR2
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x03:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0x30); // SR3
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x04:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0x40); // SR3
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  case 0x05:
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0x50); // SR3
    SR = SPIin(0x00);
    WB_CS_HIGH();
    break;
  default:
    SR = 0xFF;
    break;
  }    
  return SR;
}

/********************
Function: SPINAND page data read
Argument: 
PA_H, page address
PA_L, page address
return:
*********************/
void WB_Serial_NAND_PageDataRead(uint8_t PA_H, uint8_t PA_L){  
  WB_CS_LOW();
  SPIin(0x13); // 
  SPIin(0x00); // dummy
  SPIin(PA_H); // Page address
  SPIin(PA_L); // Page address
  WB_CS_HIGH();  
  WB_Serial_NAND_ReadyBusy_Check(); // Need to wait for the data transfer.
  return;
}

/********************
Function: Serial NAND Normal read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_Normal_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0x03);
  SPIin(addh);
  SPIin(addl);
  SPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = SPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: Serial NAND Quad IO read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_QuadIO_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0xEB);
  QSPIin(addh);
  QSPIin(addl);
  QSPIin(0x00); // dummy
  QSPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = QSPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: Serial NAND Quad Output read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_QuadOutput_Read(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0x6B);
  SPIin(addh);
  SPIin(addl);
  SPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = QSPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: Serial NAND Quad Output with 4 byte address read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_QuadOutput_Read_4Byte(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0x6C);
  SPIin(addh);
  SPIin(addl);
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = QSPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: Serial NAND Quad IO with 4 byte address read to buffer
Argument: 
addh~addl: Read data address
count: read count
return:
*********************/
void WB_Serial_NAND_QuadIO_Read_4Byte(uint8_t addh, uint8_t addl, uint8_t* buff, uint32_t count){  
  uint32_t i = 0;
  WB_CS_LOW();
  SPIin(0xEC);
  QSPIin(addh);
  QSPIin(addl);
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  SPIin(0x00); // dummy
  for( i = 0; i < count; i++){
    *(buff+i) = QSPIin(0x00);
  }    
  WB_CS_HIGH();  
  return;
}

/********************
Function: Serial NAND reset command
Argument:
return:
*********************/
void WB_NAND_Reset(){
  WB_CS_LOW();
  SPIin(0xFF);
  WB_CS_HIGH();
  WB_Serial_NAND_ReadyBusy_Check();
}


/********************
Function: SPI NAND Ready busy check
Argument: 
return: 
*********************/
void WB_Serial_NAND_ReadyBusy_Check(){
  uint8_t SR = 0xFF;  
  while(SR&0x3 != 0x00){
    WB_CS_LOW();
    SPIin(0x05);
    SPIin(0xC0);
    SR = SPIin(0x00);
    WB_CS_HIGH();
  }
  return;
}

/********************
Function: read JEDEC ID
Argument: cs sel
return: JEDEC ID
*********************/
uint32_t WB_NAND_Read_JEDEC_ID(){
  uint32_t JEDECID = 0;
  WB_CS_LOW();
  SPIin(0x9F);
  SPIin(0x00); // dummy
  JEDECID += SPIin(0x00);
  JEDECID <<= 8;
  JEDECID += SPIin(0x00);
  JEDECID <<= 8;
  JEDECID += SPIin(0x00);
  WB_CS_HIGH();
  return JEDECID;
}

/********************
Function: CS select low
Argument:
return: 
*********************/
void WB_CS_LOW(){
  GPIO_ResetBits(GPIOB, GPIO_Pin_9);      // Use GPIOB for test, please modify it to fit the EVB
}

/********************
Function: CS select high
Argument:
return: 
*********************/
void WB_CS_HIGH(){
  GPIO_SetBits(GPIOB, GPIO_Pin_9);    // Use GPIOB for test, please modify it to fit the EVB
}

/********************
Function: SPI hardware command send
Argument: DI: DI data
return: DO data
*********************/
uint8_t SPIin(uint8_t DI){
  SPI1->DR = DI; // Write controller SPI bus register.
  while((SPI1->SR &SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
  return SPI1->DR;  
}

/********************
Function: QSPI hardware command send
Argument: DI: DI data
return: DO data
*********************/
uint8_t QSPIin(uint8_t DI){
  QSPI1->DR = DI; // Write controller SPI bus register.
  while((QSPI1->SR &SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
  return QSPI1->DR;  
}

/********************
Function: STM32 official IO bus initial.
Argument:
return: 
Comment:
Modify the IO port to meet the controller SPI bus.
*********************/
/*
void PCB_SPI_IO_Init(){
  GPIO_InitTypeDef   GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 |GPIO_Pin_3 | GPIO_Pin_4; // CS, WP, Hold
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;        
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // CS, WP, Hold default keep high
  GPIO_SetBits(GPIOA, GPIO_Pin_0);
  GPIO_SetBits(GPIOA, GPIO_Pin_3);
  GPIO_SetBits(GPIOA, GPIO_Pin_4);
 }
*/

/********************
Function: STM32 official SPI bus initial.
Argument:
return: 
Comment:
Modify the IO port to meet the controller SPI bus.
*********************/
/*
void PCB_SPI_Bus_Init(){
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
  GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; // MISO, MOSI, CLK  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

  // SPI Config 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //SPI_BaudRatePrescaler_0;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStructure);
    
  // SPI enable 
  SPI_Cmd(SPI1, ENABLE);  
}
*/

