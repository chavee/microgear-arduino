#include "TokenStore.h"
#define DEBUG
//IN spi_flash.h define SPI_FLASH_SEC_SIZE = 4096
#define SPI_FLASH_SEC_SIZE 4096
extern "C" uint32_t _SPIFFS_end;
SPIFlash flash;
//IN spi_flash.h define _SPIFFS_end = 0x405FB000
uint32_t ESP_FLASH_SEC = ((0x405FB000 - 0x40200000) / SPI_FLASH_SEC_SIZE) + 1;



void saveToken(Token *token) {
    //spi_flash_erase_sector(ESP_FLASH_SEC);
    //spi_flash_write(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, sizeof(Token));
    flash.eraseSector(ESP_FLASH_SEC);
    flash.writeAnything(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, true);
    #ifdef DEBUG
      Serial.println("saveToken Function");
      Serial.println(token->type);
      Serial.println(token->key);
      Serial.println(token->token);
      Serial.println(token->secret);
      Serial.println(token->saddr);
      Serial.println(token->sport);
      Serial.println(token->flag);
      Serial.println(token->revokecode);
      Serial.println(token->checksum);
    #endif
    saveChecksum(token);
}

bool loadToken(Token *token) {
    //spi_flash_read(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, sizeof(Token));
    flash.readAnything(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, token, true);
    #ifdef DEBUG
      Serial.println("loadToken Function");
      Serial.println(token->type);
      Serial.println(token->key);
      Serial.println(token->token);
      Serial.println(token->secret);
      Serial.println(token->saddr);
      Serial.println(token->sport);
      Serial.println(token->flag);
      Serial.println(token->revokecode);
      Serial.println(token->checksum);
      #endif
    if(compareChecksum(token)){
      Serial.println("TOKEN LOADING STATUS : TRUE");
      return true;
    }
    else {
        memset(token, 0, sizeof(Token));
        Serial.println("TOKEN LOADING STATUS : FALSE");
        false;
    }
}

void clearToken(Token *token) {
    memset(token, 0, sizeof(Token));
    flash.eraseSector(ESP_FLASH_SEC);
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
