#include "TokenStore.h"
#include "Arduino.h"
extern "C" {
  #include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_end;

uint32_t ESP_FLASH_SEC = ((_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE) + 1;

void saveToken(Token *token) {
    spi_flash_erase_sector(ESP_FLASH_SEC);
    spi_flash_write(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, sizeof(Token));
    saveChecksum(token);
}

bool loadToken(Token *token) {
    spi_flash_read(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, sizeof(Token));
    if(compareChecksum(token)){
      return true;
    }
    else {
        memset(token, 0, sizeof(Token));
        false;
    }
}

void clearToken(Token *token) {
    memset(token, 0, sizeof(Token));
    spi_flash_erase_sector(ESP_FLASH_SEC);
}

uint32_t generateChecksum(uint8_t *token, uint16_t data_len) {
    uint32_t checksum = 0;
    while(data_len--) {
        checksum += *(token);
        token++;
    }
    return checksum;
}

void saveChecksum(Token *token) {
    uint16_t data_len = sizeof(Token) - sizeof(token->checksum);
    token->checksum = generateChecksum((uint8_t*)token, data_len);
}

bool compareChecksum(Token *token) {
    uint16_t data_len =  sizeof(Token) - sizeof(token->checksum);
    if(token->checksum == generateChecksum((uint8_t*)token, data_len))
        return true;
    else return false;
}
