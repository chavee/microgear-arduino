#ifndef AUTHCLIENT_H
#define AUTHCLIENT_H

#include <arduino.h>
#include <limits.h>
#include <Client.h>
#include "TokenStore.h"
#include "MgTime.h"
#include "SHA1.h"
#include "func.h"
#include "config.h"

#define MGREV "E8A1c"

#define REQUESTTOKEN					1
#define ACCESSTOKEN						2

#define KEYSIZE                    		16
#define SECRETSIZE                 		32

#define HMACBASE64SIZE					28
#define HASKKEYSIZE				   		SECRETSIZE+TOKENSECRETSIZE+1

#define HTTP_BUFFER_SIZE     			640
#define READ_CHUNK_SIZE                 HTTP_BUFFER_SIZE-1

#define HTTP_TIMEOUT                    5000

#define INT_NULL						INT_MIN
#define INT_INVALID                     INT_MIN+1

#define AUTH_ADDRESS 			      "ga.netpie.io"
#define AUTH_PORT				       "8080"
#define AUTH_REQUEST_TOKEN_URI  "/api/rtoken"
#define AUTH_ACCESS_TOKEN_URI   "/api/atoken"

#define INT_NULL						INT_MIN
#define INT_INVALID                     INT_MIN+1

#define TKTYPE_REQUEST					65
#define TKTYPE_ACCESS					66

#define TKFLAG_SESSION                  'S'
#define TKFLAG_PERSIST                  'P'

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))


class AuthClient {
    public:
      AuthClient(Client&);
      uint32_t getServerTime();
      int getAccessToken(Token*, char*, char*, char*, char*);
      char* bufferMalloc(char**, unsigned int);
      char* bufferFree(char**);
      char* buff;
      char* setExternalBuffer(char *expt);

    private:
      Client* client;
      int connectAuthServer(Client*);
      int getHTTPResponse(Client*, char*, int);
      int getOAuthToken(Token *token, char* appid, char* key, char* secret, char* alias, char* uri);

};
#endif
