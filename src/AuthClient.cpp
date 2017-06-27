#include "AuthClient.h"

AuthClient::AuthClient(Client& iclient) {
    client = &iclient;
    buff = NULL;
}

char* AuthClient::bufferMalloc(char **pt, unsigned int size) {
    if (*pt == NULL) {
        *pt = (char *)malloc(size);
    }
}

char* AuthClient::bufferFree(char **pt) {
    if (*pt != NULL) {
        free(*pt);
        *pt = NULL;
    }
}

char* AuthClient::setExternalBuffer(char *expt) {
    buff = expt;
    return buff;
}

int AuthClient::getAccessToken(Token *token, char* appid, char* key, char* secret, char* alias) {
    uint32_t time;

    time = getServerTime();
    setTime(time);
    #ifdef DEBUG
        Serial.printf("Server Time == %d\n", time);
    #endif
    loadToken(token);
    switch (token->type) {
        case TKTYPE_ACCESS :
                return 1;
        case TKTYPE_REQUEST :
                if (getOAuthToken(token, appid, key, secret, alias, AUTH_ACCESS_TOKEN_URI)) {
                    token->type = TKTYPE_ACCESS;
                    if (token->flag == TKFLAG_PERSIST) saveToken(token);
                    return 1;
                }
                else return 0;
        default :
                memset(token, 0, sizeof(Token));
                if (getOAuthToken(token, appid, key, secret, alias, AUTH_REQUEST_TOKEN_URI)) {
                    token->type = TKTYPE_REQUEST;
                    if (getOAuthToken(token, appid, key, secret, alias, AUTH_ACCESS_TOKEN_URI)) {
                        token->type = TKTYPE_ACCESS;
                        if (token->flag == TKFLAG_PERSIST) saveToken(token);
                        return 1;
                    }
                    else {
                        saveToken(token);
                        return 0;
                    }
                }
    }
}

int AuthClient::connectAuthServer(Client *client) {
    if (this->client->connect(AUTH_ADDRESS, atoi(AUTH_PORT))) return 1;
    else return 0;
}

long headerParseLong(char* key, int plen, char* str) {
    static long val;
    if (strncmp(str, key, strlen(key))==0) {
        if (plen > 0) *(str+strlen(key)+plen) = 0;

        val = strtol(str+strlen(key), NULL, 10);
        if (val==LONG_MIN || val==LONG_MAX) {
            return INT_INVALID;
        }
        return val;
    }
    else return INT_NULL;
}

int extract(char *str, char *key, char **vp) {
    if (memcmp(str, key, strlen(key))==0) {
        *vp = str+strlen(key);
        return strlen(str) - strlen(key);
    }
    else {
        *vp = NULL;
        return 0;
    }
}

int parseendpoint(char *endpoint, char **saddr, char **sport) {
    char *a, *p;
    *sport = NULL;
    if (endpoint!=NULL && memcmp(endpoint,"pie://",6) == 0) {
        *saddr = p = endpoint+6;
        while (*p != 0) {
            if (*p == ':') {
                *p = 0;
                *sport = p+1;
                break;
            }
            p++;
        }
        return 1;
    }
    else {
        *saddr = NULL;
        return 0;
    }
}

char* urldecode(char *str) {
    char *p, *h, d=0;
    char c[3] = {0,0,0};

    if (str == NULL) return NULL;
    p = h = str;
    while (*p != 0) {
        if (*p=='%') d=2;
        else if (d>0) {
            c[2-d] = *p;
            if (--d == 0) *(h++) = strtol(c, NULL, 16);
        }
        else *(h++) = *p;
        p++;
    }
    *h = 0;
    return str;
}

// Read http client socket, set buff as a body, return http status
int AuthClient::getHTTPResponse(Client* client, char *buff, int timeout) {
    int b, r, clen = -1;
    int httpstatus = -1;
    char *h, *p, *t;
    char header = 1;

    unsigned int stm = millis();
    while (client->available() == 0) {
        if (millis() - stm > timeout) {
            #ifdef DEBUG
                Serial.println("HTTP connection timeout..");
            #endif
            client->stop();
            return -1;
        }
    }
    t = buff;
    while (true) {
        // read atmost READ_CHUNK_SIZE from client until nothing left
        r = 0;
        while (true) {
            b = client->read();
            if (b > 0) {
                *(t++) = (char)b;
                if (++r >= READ_CHUNK_SIZE) break;
            }
            else break;
        }
        if (r==0) break;
        *t = 0;
        h = p = buff;

        if (header) {
            while (*p != 0) {
                if (*p != '\r') p++;
                else {
                    p++;
                    if (*p == '\n')  {          // if \r\n found
                        *(p-1) = *p = 0;
                        #ifdef DEBUG
                            if (*h != 0) Serial.printf("Header: %s\n",h);
                        #endif
                        if (httpstatus==-1) {
                            httpstatus = headerParseLong("HTTP/1.1 ",3, h);
                        }
                        if (clen==-1 || clen==INT_NULL || clen==INT_INVALID) {
                            clen = headerParseLong("Content-Length: ",0, h);
                        }
                        if (*h == 0) header =0; // if \r\n\r\n found, then header ends
                        h = ++p;
                    }
                }
            }
            memmove(buff, h, t-h+1);             // shift the current chunk (ended with \0) to the front
            t = buff+(t-h);
        }
    }
    if (clen >= 0) *(buff+clen)=0;
    return httpstatus;
}

uint32_t AuthClient::getServerTime() {
    #define REQUESTCMD "GET /api/time HTTP/1.1\r\nConnection: close\r\n\r\n"
    uint32_t time = 0;

    bufferMalloc(&buff, HTTP_BUFFER_SIZE);
    if (connectAuthServer(this->client)) {
        memset(buff, 0, HTTP_BUFFER_SIZE);

        if (this->client->write((uint8_t *)REQUESTCMD, STRLEN(REQUESTCMD)) < 0) {
            this->client->stop();
            bufferFree(&buff);
            return 0;
        }

        memset(buff, 0, strlen(buff));
        if (getHTTPResponse(this->client, buff, HTTP_TIMEOUT) == 200) {
            time = headerParseLong("",0,buff);
        }
    }
    this->client->stop();
    bufferFree(&buff);
    return time;
}

int AuthClient::getOAuthToken(Token *token, char* appid, char* key, char* secret, char* alias, char* uri) {
    int authclient;

    if (connectAuthServer(this->client)) {
         char *p, *r;
        char rep[3];
        char signature[HMACBASE64SIZE+1];
        char hashkey[HASKKEYSIZE+1];
        char rawsig[HMACBASE64SIZE];

        bufferMalloc(&buff, HTTP_BUFFER_SIZE);
        memset(buff, 0, HTTP_BUFFER_SIZE);

        p = addattr(buff, "POST ",uri);
        p = addattr(p, " HTTP 1.1\r\n", NULL);
        p = addattr(p, "Authorization: OAuth oauth_callback=\"scope%3D%26appid%3D", appid);
        p = addattr(p, "%26mgrev%3D", MGREV);
        p = addattr(p, "%26verifier%3D", alias);
        p = addattr(p, "\",oauth_consumer_key=\"", key);
        p = addattr(p, "\",oauth_nonce=\"", "sdjkjdf8");
        p = addattr(p, "\",oauth_signature_method=\"HMAC-SHA1\",oauth_timestamp=\"",getTimeStr());
        if (*token->token) {    // if already have a request token
            p = addattr(p, "\",oauth_token=\"", token->token);
            p = addattr(p, "\",oauth_verifier=\"", alias);
        }
        p = addattr(p, "\",oauth_version=\"1.0\"",NULL);

        if (this->client->write((uint8_t *)buff, strlen(buff)) < 0) {
            Serial.println("... socket send failed\r\n");
            this->client->stop();
            bufferFree(&buff);
            return 0;
        }
        memmove(buff+(34+STRLEN(AUTH_ADDRESS)), buff+42, HTTP_BUFFER_SIZE-(42+STRLEN(AUTH_ADDRESS)));
        p = addattr(buff , "POST&http%3A%2F%2F", NULL);
        p = addattr(p, AUTH_ADDRESS, "%3A");
        p = addattr(p, AUTH_PORT,NULL);
        r = uri;
        for (r=uri; *r!=0; r++) {
            switch(*r) {
                case '/' :  memcpy(p,"%2F",3);
                            p=p+3;
                            break;
                default  :  *p++ = *r;
            }
        }
        *p++='&';
        r = p;
        while (*p != 0) {
            switch (*r) {
                case '"'  : r++;                // skip "
                            break;
                case ',': case '%': case '=':   // change , to & and encode special characters
                            memcpy(rep, *r==','?"%26":*r=='%'?"%25":"%3D", 3);
                            memmove(p+2, p, HTTP_BUFFER_SIZE-(p-buff)-2);
                            memcpy(p,rep,3);
                            r+=3; p+=3;
                            break;
                case '\0' : *p = 0;             // end the process
                            break;
                default   : *p++ = *r++;        // otherwise just copy
                            break;
            }
        }
        memset(hashkey, 0, HASKKEYSIZE);
        memcpy(hashkey, secret, SECRETSIZE);
        strcat(hashkey,"&");
        strcat(hashkey,token->secret);

        #ifdef DEBUG
            Serial.printf("Hash key:\n%s\n",hashkey);
            Serial.printf("Signature base string:\n%s\n",buff);
        #endif

        memset(signature, 0, HMACBASE64SIZE+1);
        Sha1.initHmac((uint8_t*)hashkey,strlen(hashkey));
        Sha1.HmacBase64(signature,buff);
        memset(buff, 0, HTTP_BUFFER_SIZE);
        memcpy(buff, ",oauth_signature=\"",18);
        memcpy(buff+18, signature, HMACBASE64SIZE);
        memcpy(buff+18+28, "\"\r\n", 3);

        p = buff+18;
        r = p;
        while (*p != 0) {

            switch (*r) {
                case '+'  : memcpy(rep, "%2B", 3); break;
                case '='  : memcpy(rep, "%3D", 3); break;
                case '/'  : memcpy(rep, "%2F", 3); break;
                default   : memset(rep, 0, 1); break;
            }
            if (*rep != 0) {
                memmove(p+2, p, HTTP_BUFFER_SIZE-(p-buff)-2);
                r+=3;
                memcpy(p,rep,3);
                p+=3;
            }
            else if (*r == 0) {
                *p = 0;
            }
            else {
                *p++ = *r++;
            }
        }

        #ifdef DEBUG
            Serial.printf("oauth_signature = %s\n",buff+18);
        #endif

        this->client->write((uint8_t *)buff, strlen(buff));

        memset(buff, 0, HTTP_BUFFER_SIZE);
        p = addattr(buff, "Host: ", AUTH_ADDRESS);
        p = addattr(p, ":", AUTH_PORT);
        p = addattr(p, "\r\nConnection: close\r\nUser-Agent: E8R\r\n\r\n", NULL);

        if (this->client->write((uint8_t *)buff, strlen(buff)) < 0) {
            Serial.println("... socket send failed\r\n");
            this->client->stop();
            bufferFree(&buff);
            return 0;
        }
    }
    else {
        this->client->stop();
    }

    char *h, *p, *t;
    int httpstatus = -1;
    char *oauth_token = NULL;
    char *oauth_token_secret = NULL;
    char *flag = NULL;
    char *endpoint = NULL;
    char *saddr = NULL;
    char *sport = NULL;

    httpstatus = getHTTPResponse(this->client, buff, HTTP_TIMEOUT);

    #ifdef DEBUG
        Serial.printf("\n");
        Serial.printf("Attribute: http status = %d\n",httpstatus);
        Serial.printf("Body: %s\n",buff);
    #endif

    t = h = p = buff;
    while (*h != 0) {
        if (*p != '&' && *p) p++;
        else {
            if (*p==0) t=NULL;  // if the end of the body
            *p = 0;
            if (!oauth_token) extract(h,"oauth_token=", &oauth_token);
            if (!oauth_token_secret) extract(h,"oauth_token_secret=", &oauth_token_secret);
            if (!flag) extract(h,"flag=", &flag);
            if (!endpoint) {
                extract(h,"endpoint=", &endpoint);
            }
            h = t?++p:p;
        }
    }

    strrep(token->token, oauth_token);
    strrep(token->secret, oauth_token_secret);
    if (endpoint) {
        urldecode(endpoint);
        parseendpoint(endpoint, &saddr, &sport);
        strrep(token->saddr, saddr);
        token->sport = strtol(sport, NULL, 10);
    }
    else {
        *(token->saddr) = 0;
        token->sport = 0;
    }
    token->flag = flag?*flag:0;

    #ifdef DEBUG
        Serial.printf("oauth_token == %s\n",token->token);
        Serial.printf("oauth_token_secret == %s\n",token->secret);
        Serial.printf("flag == %s\n",flag);
        Serial.printf("saddr = %s\n",saddr);
        Serial.printf("sport = %s\n",sport);
    #endif

    bufferFree(&buff);
    this->client->stop();
    return 1;
}
