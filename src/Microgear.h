#ifndef MICROGEAR_H
#define MICROGEAR_H

#include <arduino.h>
#include <ESP8266WiFi.h>
#include "MQTTClient/MQTTClient.h"
#include "SHA1.h"

#define APPIDSIZE                  32
#define KEYSIZE                    16
#define SECRETSIZE                 32
#define TOKENSIZE                  16
#define TOKENSECRETSIZE            32
#define USERNAMESIZE               65
#define PASSWORDSIZE               28
#define REVOKECODESIZE             28
#define FLAGSIZE                   4
#define FINGERPRINTSIZE            60
#define HMACSIZE                   28
#define PUB_MSG_LEN                16
#define MAXALIASSIZE               16

#define MQTT_USERNAME_SIZE         TOKENSIZE+KEYSIZE+13
#define MQTT_PASSWORD_SIZE         33


class Microgear {
  private:
    MQTTClient mqttclient;
    Client *client;

  public:
    char *appid;
    char *key;
    char *secret;
    char *alias;

    char *token;
    char *tokensecret;
    char *host;
    uint16 port;

    Microgear(Client&);
    void init(char*, char*, char*);
    void setToken(char*, char*, char*);
    int connect(char*);
};

#endif
