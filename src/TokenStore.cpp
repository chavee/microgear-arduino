#include "TokenStore.h"
#define DEBUG

uint32_t MEGA_ADDR = 0;
#ifdef ARDUINO_ESP8266_NODEMCU
  uint32_t ESP_FLASH_SEC = ((_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE) + 1;
#endif

void saveFunc(Token *token){
  #ifdef ARDUINO_ESP8266_NODEMCU
    spi_flash_write(ESP_FLASH_SEC * SPI_FLASH_SEC_SIZE, (uint32_t *)token, sizeof(Token));
    Serial.println("SAVING : This is ESP8266");
  #endif

  #ifdef __AVR_ATmega2560__
    //Serial.println(EEPROM.put(MEGA_ADDR, *token));
    Serial.print("EEPORM USAGE SIZE : ");
    Serial.println(EEPROM_writeAnything(MEGA_ADDR, *token));
    Serial.println("SAVING : This is Mega2560");
  #endif
}
void eraseFunc(){
  #ifdef ARDUINO_ESP8266_NODEMCU
    spi_flash_erase_sector(ESP_FLASH_SEC);
    Serial.println("ERASING : This is ESP8266");
  #endif

  #ifdef __AVR_ATmega2560__
    for (int i = 0 ; i < EEPROM.length() ; i++) {
        EEPROM.write(i, 0);
    }
    Serial.println("ERASING : This is Mega2560");
  #endif
}

void readFunc(Token *token){
  EEPROM_readAnything(MEGA_ADDR, *token);
}

void saveToken(Token *token) {
    eraseFunc();
    saveChecksum(token);
    #ifdef DEBUG
      Serial.println("========================saveToken Function=======================");
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
    saveFunc(token);
}

bool loadToken(Token *token) {
    readFunc(token);
    #ifdef DEBUG
      Serial.println("========================loadToken Function========================");
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
    eraseFunc();
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
