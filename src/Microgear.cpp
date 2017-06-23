#include "Microgear.h"

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming --> ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.print(payload);
  Serial.println();
}

Microgear::Microgear(Client& tcpclient ) {
  this->client = &tcpclient;
}

void Microgear::init(char *key, char *secret, char *alias) {
    this->key = key;
    this->secret = secret;
    this->token = NULL;
    this->tokensecret = NULL;
    this->alias = alias;
/*
    mg->cb_connected = NULL;
    mg->cb_absent = NULL;
    mg->cb_present = NULL;
    mg->cb_message = NULL;
    mg->cb_error = NULL;
    mg->cb_info = NULL;
*/
}


int Microgear::connect(char*) {
  char mqtt_username[MQTT_USERNAME_SIZE+1];
  char raw_password[20];
  char mqtt_password[MQTT_PASSWORD_SIZE+1];
  char hashkey[SECRETSIZE+TOKENSECRETSIZE+2];

  this->mqttclient.begin(this->host, this->port,*(this->client));




  return 0;
}

void Microgear::setToken(char* token, char* tokensecret, char* endpoint) {
  this->token = token;
  this->tokensecret = tokensecret;

  if (endpoint != NULL) {
    char *p;
    for (p = endpoint; *p!='\0' && *p!=':' ;p++);
    if (*p == ':') {
        *p = '\0';
        this->host = endpoint;
        this->port = atoi(p+1);
    }
  }
}
