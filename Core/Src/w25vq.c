#include "w25vq.h"

/**
 * !wp koruma pinini hardware olarak kontrol edilip edilmediğini anlamak için bir takım register olayları var onun dışınd
 * !wp: veri yazma sırasında HIGH olarak ayarla
 */
extern QSPI_HandleTypeDef hqspi;	///< Quad SPI HAL Instance

typedef struct Qspi_variable_{
	uint8_t qe;
	uint8_t we;
}Qspi_variable;

//#define AUTOPOOL
#define RESET

Qspi_variable qspi_data;

uint8_t QSPI_Run(void){
#ifdef TDEBUG
	uint8_t read_data[SECTOR_BYTE];
	uint8_t write_data[SECTOR_BYTE];
	uint16_t test_number, test_addr = 0;

	for(uint32_t i=0;i<SECTOR_BYTE;i++){
		write_data[i] = i & 0xff;
	}

	if(QSPI_Init()!=HAL_OK){
		uprintf("\r\nErr init");
		return HAL_ERROR;
	}

#ifndef DEV
	uint16_t devid = QSPI_ReadDevID();
	uprintf("\r\n dev-> 0x%X", devid);
#endif

#ifdef STATUS
	uint8_t sr = 0;
	QSPI_ReadStatusReg(&sr, 1, 1);
	uprintf("\r\n sr[1]-> %d", sr);
	QSPI_ReadStatusReg(&sr, 2, 1);
	uprintf("\r\n sr[2]-> %d", sr);
	QSPI_ReadStatusReg(&sr, 3, 1);
	uprintf("\r\n sr[3]-> %d", sr);
	QSPI_ReadStatusReg(&sr, 4, 1);
	uprintf("\r\n sr[4]-> %d", sr);
	QSPI_ReadStatusReg(&sr, 5, 1);
	uprintf("\r\n sr[5]-> %d", sr);
#endif


#define ERASE;

#ifdef ERASE
	QSPI_EraseChip();
#define loop
#endif
#ifndef loop
	QSPI_QEEnable();
	if(QSPI_EnableMemoryMappedMode() !=HAL_OK){
		return HAL_ERROR + 1;
	}
	QSPI_AutoPollingMemReady();
#endif

	while(1){
#ifdef loop
		HAL_Delay(1);
		  //write
		write_data[0] = test_number;
		if(QSPI_Write(test_addr, write_data, 1) != HAL_OK){
		  uprintf("\r\nErr %ld", test_addr);
		  while(1);
		}
		//read
		if(test_addr %15 == 0){
		  QSPI_Read(test_addr, read_data, 1);
		}
		uprintf("[%ld] Read-> %d\r\n", test_addr, read_data[0]);
		test_addr = ((test_addr+1) & (PAGE_SIZE - 1));
		test_number = ((read_data[0]+1) & 0xff);
#else
		  uprintf("\r\naddr:%ld ", test_addr);
		  memcpy(read_data, ((uint8_t *)0x90000000 + test_addr), 1);
		  uprintf("[%ld] Read-> %d\r\n", test_addr, read_data[0]);
		  HAL_Delay(10);
		  test_addr += 1;
#endif
	}
#endif
	return HAL_OK;
}

/**********************************LIB START************************************************************/

QSPI_CommandTypeDef QSPI_SetCom(uint32_t Instruction) {
	QSPI_CommandTypeDef sCommand;
	memset(&sCommand, 0, sizeof(QSPI_CommandTypeDef));
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DdrMode = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.InstructionMode = qspi_data.qe ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
	sCommand.AddressMode = QSPI_ADDRESS_NONE;
	sCommand.DataMode = QSPI_DATA_NONE;

	sCommand.NbData = 0;
	sCommand.Address = 0;
	sCommand.DummyCycles = 0;
	sCommand.Instruction = Instruction;
	return sCommand;
}

void QSPI_Delay(uint32_t us){
	for (uint32_t temp = 0; temp < us; temp++) {
		__NOP();
	}
}

uint8_t QSPI_IsBusy(void) {
#ifdef AUTOPOOL
	QSPI_AutoPollingMemReady();
#else
	uint8_t reg1[1], is_busy = 1;
	while(is_busy){
	  QSPI_ReadStatusReg(reg1, 1, 1);
	  is_busy = ((reg1[0]) & 0x01);
	  QSPI_Delay(1000);
	}
#endif
	return HAL_OK;
}

uint8_t QSPI_Cmd(uint32_t cmd){
	QSPI_CommandTypeDef sCommand = QSPI_SetCom(cmd);
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

uint8_t QSPI_AutoPollingMemReady(void){
	QSPI_CommandTypeDef com = QSPI_SetCom(0x05);
	QSPI_AutoPollingTypeDef sConfig;

	sConfig.Match = 0x00;
	sConfig.Mask = 0x01;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x50;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_DISABLE;

	com.DataMode = qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
	com.NbData = 1;
	if (HAL_QSPI_AutoPolling(&hqspi, &com, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
  return FLASH_OK;
}

uint8_t QSPI_WriteEnable(void) {

	if (QSPI_Cmd(0x06) != HAL_OK) {
		return HAL_ERROR;
	}
#ifdef AUTOPOOL
	QSPI_AutoPollingTypeDef sConfig;
	QSPI_CommandTypeDef com = QSPI_SetCom(0x05);//read st reg

	//waiting for enable
	sConfig.Match = 0x02;
	sConfig.Mask = 0x02;
	sConfig.MatchMode = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval = 0x20;
	sConfig.AutomaticStop = QSPI_AUTOMATIC_STOP_DISABLE;

	com.DataMode = qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
	com.NbData = 1;
	if (HAL_QSPI_AutoPolling(&hqspi, &com, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
#else
	uint8_t reg_data[1];
	while(!(reg_data[0] & 0x02)){
		QSPI_ReadStatusReg(reg_data, 1, 1);
		QSPI_Delay(1000);
	}
#endif
	return HAL_OK;
}

uint8_t QSPI_Reset(void) {
	memset(&qspi_data, 0, sizeof(Qspi_variable));
	uint32_t temp = 0;
	//enable reset
	if (QSPI_Cmd(0x66) != HAL_OK) {
		return HAL_ERROR;
	}
	//delay
	for (temp = 0; temp < 0x2f; temp++) {
		__NOP();
	}
	//reset
	if(QSPI_Cmd(0x99) !=HAL_OK){
		return HAL_ERROR;
	}
	return HAL_OK;
}


void QSPI_Gpio_mode(uint8_t mode, uint32_t pin, GPIO_TypeDef *gpio){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = mode ? GPIO_MODE_INPUT : GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

uint8_t QSPI_Init(void) {
	uint8_t wr[1] = {0x00}, status = FLASH_OK;
#ifdef WPHOLD
    QSPI_Gpio_mode(1, GPIO_PIN_2, GPIOE);
    QSPI_Gpio_mode(1, GPIO_PIN_6, GPIOF);
#endif
#ifdef RESET
	if(QSPI_Reset()!=HAL_OK){
		return HAL_ERROR;
	}
#endif
	QSPI_QEDisable();
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
	QSPI_Cmd(0XAB);
	QSPI_WriteEnable();
	status = QSPI_WriteStatusReg(wr, 1, 1);//reg1 set 0x00
	QSPI_Delay(100);
	QSPI_ReadDevID();
	return status;
}

uint8_t QSPI_ReadStatusReg(uint8_t *reg_data, uint8_t reg_num, uint8_t size) {
	uint32_t addr[7] = {0x00, 0x05, 0x35, 0x15, 0x48, 0x94, 0x9F};
	QSPI_CommandTypeDef sCommand = QSPI_SetCom(addr[reg_num]);

	sCommand.DataMode = qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
	sCommand.NbData = size;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, reg_data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}

uint8_t QSPI_WriteStatusReg(uint8_t *reg_data, uint8_t reg_num, uint8_t size) {
	uint32_t addr[6] = {0x00, 0x01, 0x31, 0x11, 0x38};
	QSPI_CommandTypeDef com = QSPI_SetCom(addr[reg_num]);
	com.DataMode = qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;
	com.NbData = size;

	if (QSPI_IsBusy() != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Transmit(&hqspi, reg_data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}


uint8_t QSPI_EnableMemoryMappedMode(void) {
    QSPI_CommandTypeDef com = QSPI_SetCom(0xEB);
    	com.InstructionMode = QSPI_INSTRUCTION_4_LINES;
		com.AddressMode = QSPI_ADDRESS_4_LINES;
		com.DataMode = QSPI_DATA_4_LINES;
		com.DummyCycles = 2;

    QSPI_MemoryMappedTypeDef sMemMappedCfg;
    	sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

	if (HAL_QSPI_MemoryMapped(&hqspi, &com, &sMemMappedCfg) != HAL_OK) {
		return HAL_ERROR;
	}
    return HAL_OK;
}

uint16_t QSPI_ReadDevID(void){
	uint8_t reg_data[3];
	QSPI_ReadStatusReg(reg_data, 6, 3);
    return (reg_data[0]<<8) | reg_data[1];
}

uint8_t QSPI_QEEnable(){
	uint8_t status = FLASH_OK;
	QSPI_WriteEnable();
	QSPI_Cmd(0X38);
	uint8_t wr[1] = {0x02};
	status = QSPI_WriteStatusReg(wr, 2, 1);
	QSPI_IsBusy();
	qspi_data.qe = 1;
	return status;
}

uint8_t QSPI_QEDisable(){
	qspi_data.qe = 1;
	uint8_t status = FLASH_OK;
	status = QSPI_Cmd(0xFF);
	qspi_data.qe = 0;
	return status;
}
uint8_t QSPI_EnableMap(){
	QSPI_QEEnable();
	if(QSPI_EnableMemoryMappedMode() !=HAL_OK){
		return HAL_ERROR + 1;
	}
	QSPI_AutoPollingMemReady();
	return HAL_OK;
}

uint8_t QSPI_EraseSector(uint32_t sector) {
	if(sector >	SECTOR_SIZE){
		return FLASH_ERR_PARAM;
	}
	//enable
	if(QSPI_WriteEnable()!= HAL_OK) {
		return HAL_ERROR;
	}
	//resume
	if(QSPI_Cmd(0x7A)!= HAL_OK) {
		return HAL_ERROR;
	}
	//erase sector
	QSPI_CommandTypeDef sCommand = QSPI_SetCom(0x20);
	sCommand.AddressMode = qspi_data.qe ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_1_LINE;
	sCommand.Address = sector * SECTOR_BYTE;
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	//waiting finish
	QSPI_IsBusy();
	return HAL_OK;
}

uint8_t QSPI_EraseBlock(uint32_t block){
	if(block >	BLOCK_SIZE){
		return FLASH_ERR_PARAM;
	}
	//enable
	if(QSPI_WriteEnable()!= HAL_OK) {
		return HAL_ERROR;
	}
	//resume
	if(QSPI_Cmd(0x7A)!= HAL_OK) {
		return HAL_ERROR;
	}
	//erase sector
	QSPI_CommandTypeDef sCommand = QSPI_SetCom(0xD8);
	sCommand.AddressMode = qspi_data.qe ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_1_LINE;
	sCommand.Address = block * BLOCK_BYTE;
	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}

	//waiting finish
	QSPI_IsBusy();
	return HAL_OK;
}

uint8_t QSPI_EraseChip(){
  //enable
  QSPI_WriteEnable();
  //erase
  if(QSPI_Cmd(0xC7)!= HAL_OK) {
	return HAL_ERROR;
  }
  //waiting
  QSPI_IsBusy();
  return HAL_OK;
}


uint8_t QSPI_Write(uint32_t address, uint8_t *data, uint32_t size){
	int32_t page_byte = size;
	uint32_t loop_size = 0;

	if(size > (PAGE_SIZE * PAGE_BYTE)){
		return FLASH_ERR_PARAM;
	}
	//write
	uint32_t cmd = qspi_data.qe ? 0x32 : 0x02;
	QSPI_CommandTypeDef sCommand = QSPI_SetCom(cmd);
	sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.AddressMode =  qspi_data.qe ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_1_LINE;
	sCommand.DataMode =  qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;

	//write map
	while(page_byte > 0){

		if(!(loop_size % 0x100))
			QSPI_WriteEnable();

		size = ((page_byte>PAGE_BYTE) ? PAGE_BYTE : page_byte);
		sCommand.Address = (address);
		sCommand.NbData = size;

		if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}
		if (HAL_QSPI_Transmit(&hqspi, ((void *)data)+(loop_size), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
			return HAL_ERROR;
		}

		loop_size +=PAGE_BYTE;
		address	  +=PAGE_BYTE;
		page_byte -=PAGE_BYTE;

		QSPI_IsBusy();
	}

  return HAL_OK;
}

uint8_t QSPI_Read(uint32_t address, uint8_t *data, uint32_t size){
	uint32_t cmd = qspi_data.qe ? 0xEB : 0x03;
    QSPI_CommandTypeDef sCommand = QSPI_SetCom(cmd);
    sCommand.AddressSize = QSPI_ADDRESS_24_BITS;
	sCommand.AddressMode =  qspi_data.qe ? QSPI_ADDRESS_4_LINES : QSPI_ADDRESS_1_LINE;
	sCommand.DataMode =  qspi_data.qe ? QSPI_DATA_4_LINES : QSPI_DATA_1_LINE;;
	sCommand.NbData = size;
	sCommand.Address =  address;
	sCommand.DummyCycles =  qspi_data.qe ? 2 : 0;

	if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_QSPI_Receive(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
		return HAL_ERROR;
	}
	return HAL_OK;
}
