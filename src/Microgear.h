#ifndef MICROGEAR_H
#define MICROGEAR_H

#include <arduino.h>
#include <ESP8266WiFi.h>
#include "MQTTClient/MQTTClient.h"
#include "SHA1.h"
#include "func.h"
#include "TokenStore.h"
#include "config.h"

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

#define MAXTOPICSIZE               128
#define MAXPAYLOADSIZE             128


#define MQTT_USERNAME_SIZE         TOKENSIZE+KEYSIZE+13
#define MQTT_PASSWORD_SIZE         33

/* Event Type */
#define MESSAGE                    1
#define PRESENT                    2
#define ABSENT                     3
#define CONNECTED                  4
#define ERROR          		         5
#define INFO                  	   6

typedef void (* CALLBACK)(char*, uint8_t*, unsigned int);

class Microgear {
  private:
    MQTTClient mqttclient;
    Client *client;
    Token tokenstore;

  public:
    char *appid;
    char *key;
    char *secret;
    char *alias;

    char *token;
    char *tokensecret;
    char *host;
    uint16 port;

    CALLBACK cb_connected;
    CALLBACK cb_absent;
    CALLBACK cb_present;
    CALLBACK cb_message;
    CALLBACK cb_error;
    CALLBACK cb_info;

    Microgear(Client&);
    void init(char*, char*, char*);
    void setToken(char*, char*, char*);
    int connect(char*);
    void loop();
    bool connected();

		bool publish(char*, char*);
		bool publish(char*, char*, bool);
    bool publish(char*, double);
		bool publish(char*, double, bool);
		bool publish(char*, double, int);
		bool publish(char*, double, int, bool);
		bool publish(char*, int);
		bool publish(char*, int, bool);
		bool publish(char*, String);
		bool publish(char*, String, bool);
		bool publish(char*, String, String);

    void subscribe(char*);
		void unsubscribe(char*);

    bool chat(char*, char*);
		bool chat(char*, int);
		bool chat(char*, double);
		bool chat(char*, double, int);
		bool chat(char*, String);

    void on(unsigned char event, CALLBACK);

};

typedef Microgear MicroGear;
#endif
