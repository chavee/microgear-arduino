#ifndef TOKENSTORE_H
#define TOKENSTORE_H
#include "stdint.h"
#include "Arduino.h"

#define KEYSIZE                  16
#define TOKENSIZE                16
#define TOKENSECRETSIZE          32
#define ENDPOINTSIZE             200
#define REVOKECODESIZE           28

struct token_struct{
  char type;
  char key[KEYSIZE+1];
  char token[TOKENSIZE+1];
  char secret[TOKENSECRETSIZE+1];
  char saddr[ENDPOINTSIZE+1];
  uint16_t sport;
  char flag;
  char revokecode[REVOKECODESIZE+1];
  uint32_t checksum;
};
typedef struct token_struct Token;

uint32_t generateChecksum(uint8_t*, uint16_t);
bool compareChecksum(Token*);
void showAllData(uint8_t* token);
void saveChecksum(Token*);
void saveToken(Token*);
bool loadToken(Token*);
void clearToken(Token*);

#endif

