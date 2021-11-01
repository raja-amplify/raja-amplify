/**
  ******************************************************************************
  * @file    /WB_SerialNAND_Sample_Code_LLD.c
  * @author  Winbond FAE Steam Lin
  * @version V1.0.4
  * @date    11-November-2017
  * @brief   This code provide the basic Serial NAND Flash application cover ONFI NAND Flash working behavior based on STM32F205 
  *            
  * COPYRIGHT 2015 Winbond.
*/ 

#include <stdlib.h>
#include "WB_SerialNAND_Sample_Code_LLD.h"

#define Page_size 0x800			// 2048 bytes per page
#define Spare_size 0x40			// 64 bytes per page spare area
#define Spare_size_128 0x80		// 128 bytes per page spare area
#define Block_size 0x40			// 64 pages	per block
#define Block_count 0x400		// 1024 blocks per 1G NAND
#define status_ok 0x00
#define ID_error 0x01
#define Read_error 0x02
#define Erase_error 0x03
#define Program_error 0x04
#define Bad_block_count_over 0x05

uint32_t Chip_size = Page_size*Block_size*Block_count;

// Address data mapping follow W25N01GV datasheet, user could modify according to the controller memory mapping method

#define ADDR_Col_CA15_CA8(ADD)		(uint8_t)(((ADD) & 0x0F00) >> 8)
#define ADDR_Col_CA7_CA0(ADD)		(uint8_t)((ADD) & 0xFF)
#define ADDR_Row_PA15_PA8(ADD)		(uint8_t)(((ADD) & 0xFF00000) >> 20)
#define ADDR_Row_PA7_PA0(ADD)		(uint8_t)(((ADD) & 0xFF000) >> 12)

unsigned int Winbond_SPINAND_ID_Table[] = {
	0xEFAA21, // W25N01GV
	0xEFBA21, // W25N01GW
	0xEFAA22, // W25N02KV
	0xEFAB21, // W25M02GV
	0xEFAA23, // W25N04KV
	0xEFBC21, // W25N01JW
};

unsigned int Table_size = sizeof(Winbond_SPINAND_ID_Table)/sizeof(Winbond_SPINAND_ID_Table[0]);

void main(){
	Initial_Serial_NAND();
	Check_JEDEC_ID();
	//WB_Serial_NAND_BBM();		// Winbond embedded LUT and BBM function
	Check_Serial_NAND_Read_Write();
	Die_Select_Read_Sample();
}

void Die_Select_Read_Sample(){

	WB_Die_Select(0x00);	// Select default die
	WB_Die_Select(0x01);	// Select the other 1G NAND Flash die
	WB_Die_Select(0x00);	// Select default die
	
	unsigned int read_page_address;	// Page address, for 2G would have 0x20000 pages
	
	read_page_address = 0;
	
	if(read_page_address < 10000){
		WB_Die_Select(0x00);
		WB_Serial_NAND_PageDataRead(read_page_address/0x100, read_page_address%0x100);
		}
	else{
		WB_Die_Select(0x01);
		WB_Serial_NAND_PageDataRead(read_page_address/0x100, read_page_address%0x100);
		}
}

void Continuous_Read_Sample(){
	uint8_t read_buf[Chip_size];	// read whole Flash data
	WB_Disable_Buffer_mode(); // Enter continuous read mode
	WB_Serial_NAND_PageDataRead(0x00, 0x00);	// Load paga data address at 0x00
	WB_Serial_NAND_Continuous_Normal_Read(read_buf, Chip_size);
}

void Initial_Serial_NAND(){
	PCB_SPI_Bus_Init(); 			// Initial MOSI, MISO and CLK 
	PCB_SPI_IO_Init();				// Initial CS, WP, CLK
	PCB_Power_on();					// Flash power on function prototype
	WB_NAND_Reset();				// Reset instruction, Serial NAND enter in buffer mode
	//WB_Disable_Embedded_ECC();	// After power on, Winbond Serial NAND ECC-E default = 1. Set ECC-E = 0 to disable embedded ECC		
}

uint8_t Check_JEDEC_ID(){
	uint32_t JEDEC_ID;
	JEDEC_ID = WB_NAND_Read_JEDEC_ID();	// Read JEDEC_ID
	for(unsigned int i = 0; i < Table_size; i++){
		if(JEDEC_ID == Winbond_SPINAND_ID_Table[i]){
			return status_ok;
			}
	}
	return ID_error;
}

uint8_t Check_Serial_NAND_Quad_Read_Write(){
	uint32_t memory_offset = 0; 																	// Flash Address to access
	uint8_t read_buffer[Page_size];
	uint8_t program_buffer[Page_size];
	uint8_t EPR_status;					// Check Erase, program, read status
	uint32_t i;
	
	/* APP code for quad read/write
	
	/*Do un-protect */
	WB_Unprotect();
	
	/*Winbond Serial NAND Flash default ECC-enable
	//WB_Enable_Embedded_ECC();
	//WB_Disable_Embedded_ECC();
	
	/*Do block erase*/
	//WB_Serial_NAND_BlockErase( ADDR_Row_PA15_PA8(memory_offset), ADDR_Row_PA7_PA0(memory_offset) ); // Do Erase
	//if(WB_Serial_NAND_bad_block_check(memory_offset))		// check bad block mark before erase
	//	return;
	WB_Serial_NAND_BlockErase(0, 0); // Do Erase at 0x00 for example
	EPR_status = WB_Check_Program_Erase_Fail_Flag();
	if(EPR_status!=0){
		return Erase_error;
		}	
	memset(program_buffer, 0x55, sizeof(uint8_t)*Page_size);		// Set memory data for program verify
	
	/*Do 1 block program*/
	for( i = 0; i < Block_size; i++){
		WB_Serial_NAND_Pageprogram_Pattern_QuadInput(0, 0, program_buffer, Page_size);	// program all page data = 0x55
		//WB_Serial_NAND_Program_Excute( ADDR_Row_PA15_PA8(memory_offset+ i*(Page_size+Spare_size)), ADDR_Row_PA7_PA0(memory_offset+ i*(Page_size+Spare_size)));
		WB_Serial_NAND_Program_Excute(0, i); 	// Do program at page 0x00 to 0x40
		EPR_status = WB_Check_Program_Erase_Fail_Flag();
		if(EPR_status!=0){
			return Program_error;
			}
		}
	
	/*Do 1 block program verify*/
	for( i = 0; i < Block_size; i++){
		//WB_SPI_Serial_PageDataRead( ADDR_Row_PA15_PA8(memory_offset+ i*(Page_size+Spare_size)), ADDR_Row_PA7_PA0(memory_offset+ i*(Page_size+Spare_size)));
		WB_Serial_NAND_PageDataRead(0, 0);		// Read verify
		WB_Serial_NAND_QuadIO_Read(0, 0, read_buffer, Page_size);
		EPR_status = WB_Check_Embedded_ECC_Flag();
		if(EPR_status != 0x00 || EPR_status != 0x01){
			return Read_error;					// Check ECC status and return fail if (ECC-1, ECC0) != (0,0) or != (0,1)
			}
		if(Program_verify(read_buffer, program_buffer, Page_size)){
			return Read_error;
			}
		}	
	return status_ok;
}

uint8_t Check_Serial_NAND_Read_Write(){
	uint32_t memory_offset = 0; 																	// Flash Address to access
	uint8_t read_buffer[Page_size];
	uint8_t program_buffer[Page_size];
	uint8_t EPR_status;					// Check Erase, program, read status
	uint32_t i;
	
	/*Do un-protect */
	WB_Unprotect();
	
	/*Winbond Serial NAND Flash default ECC-enable
	//WB_Enable_Embedded_ECC();
	//WB_Disable_Embedded_ECC();
	
	/*Do block erase*/
	//WB_Serial_NAND_BlockErase( ADDR_Row_PA15_PA8(memory_offset), ADDR_Row_PA7_PA0(memory_offset) ); // Do Erase
	WB_Serial_NAND_BlockErase(0, 0); // Do Erase at 0x00 for example
	EPR_status = WB_Check_Program_Erase_Fail_Flag();
	if(EPR_status!=0){
		return Erase_error;
		}	
	memset(program_buffer, 0x55, sizeof(uint8_t)*Page_size);		// Set memory data for program verify
	
	/*Do 1 block program*/
	for( i = 0; i < Block_size; i++){
		WB_Serial_NAND_Pageprogram_Pattern(0, 0, program_buffer, Page_size);	// program all page data = 0x55
		//WB_Serial_NAND_Program_Excute( ADDR_Row_PA15_PA8(memory_offset+ i*(Page_size+Spare_size)), ADDR_Row_PA7_PA0(memory_offset+ i*(Page_size+Spare_size)));
		WB_Serial_NAND_Program_Excute(0, i); 	// Do program at page 0x00 to 0x40
		EPR_status = WB_Check_Program_Erase_Fail_Flag();
		if(EPR_status!=0){
			return Program_error;
			}
		}
	
	/*Do 1 block program verify*/
	for( i = 0; i < Block_size; i++){
		//WB_SPI_Serial_PageDataRead( ADDR_Row_PA15_PA8(memory_offset+ i*(Page_size+Spare_size)), ADDR_Row_PA7_PA0(memory_offset+ i*(Page_size+Spare_size)));
		WB_Serial_NAND_PageDataRead(0, 0);		// Read verify
		WB_Serial_NAND_Normal_Read(0, 0, read_buffer, Page_size);
		EPR_status = WB_Check_Embedded_ECC_Flag();
		if(EPR_status != 0x00 || EPR_status != 0x01){
			return Read_error;					// Check ECC status and return fail if (ECC-1, ECC0) != (0,0) or != (0,1)
			}
		if(Program_verify(read_buffer, program_buffer, Page_size)){
			return Read_error;
			}
		}	
	return status_ok;
}

uint8_t WB_Serial_NAND_BBM(){
  /* embedded BBM and LUT is work on both BUF = 0 & BUF = 1 mode */
  uint16_t i;
  uint8_t bad_block_limit = 20;				// 1Gb NAND spec bad block count limitation is 20
  uint16_t bbt_list[20];					// assume LBA is bad block
  uint8_t bbt_count = 0;					// bad block count
  uint16_t RBA = 0x3FF;						// Replace the bad block start from the last block, for 1Gb NAND, last block address is 0x3FF

  /* Scan serial NAND bad block and set up LUT to use embedded BBM */
  
  for( i = 0; i < 0x400; i++){
    if(WB_Serial_NAND_bad_block_check(i*0x40)){
      if(bbt_count == bad_block_limit){
        return Bad_block_count_over;	// Over spec
        }
      if(i == RBA){
        bbt_list[bbt_count] = i;
        bbt_count++;
        return status_ok;					// last bad block is found, no enough block can be replaced.
        }
      bbt_list[bbt_count] = i;
      bbt_count++;
      while(WB_Serial_NAND_bad_block_check(RBA*0x40)){
      if(bbt_count == bad_block_limit){
        return Bad_block_count_over;	// 1Gb NAND spec bad block count limitation is 20
        }	
      bbt_list[bbt_count] = RBA;
      bbt_count++;
      RBA--;
      if(i == RBA){
        return status_ok;				// last bad block is found, no enough block can be replaced.
        }
      }
      WB_Serial_NAND_LUT_Set(i, RBA);			// Use good block replace bad block
      RBA--;
      }
    else{
      if(i == RBA){
        return status_ok;				// bad block scan reach the end.
        }
      }
    }
  return status_ok;
  }      
