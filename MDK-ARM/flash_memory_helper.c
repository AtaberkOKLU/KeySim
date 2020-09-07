void FlashWrite(uint32_t address, uint32_t data){
	uint32_t PAGEError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Page = 255;
	EraseInitStruct.NbPages = 1;
	EraseInitStruct.Banks	= FLASH_BANK_1;

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
		HAL_FLASH_GetError();

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address, data);
	HAL_FLASH_Lock();
}