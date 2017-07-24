#include "Microgear.h"

Microgear *mg;
char pbuffer[MAXPAYLOADSIZE+1];
unsigned int topicprefixlen = 0;

void messageReceived(char *topic, uint8_t *payload, unsigned int length) {
     char* rtopic =  topic+topicprefixlen+1;

     if (*rtopic == '&') {
         if (strcmp(rtopic,"&present") == 0) {
             if (mg->cb_present) {
                 mg->cb_present("present",payload,length);
             }
         }
         else if (strcmp(rtopic,"&absent") == 0) {
             if (mg->cb_absent) {
                 mg->cb_absent("absent",payload,length);
             }
         }
         else if (strcmp(rtopic,"&resetendpoint") == 0) {
             #ifdef DEBUG_H
                 Serial.println("to reset endpoint");
             #endif
//             if (mg) mg->resetEndpoint();
         }
     }
     else if (*topic == '@') {
         if (strcmp(topic,"@error")==0) {
             if (mg->cb_error)  {
                 mg->cb_error("error",payload,length);
             }
         }
         else if (strcmp(topic,"@info")==0) {
             if (mg->cb_info)  {
                 mg->cb_info("info",payload,length);
             }
         }
     }
     else if (mg->cb_message) {
         mg->cb_message(topic,payload,length);
     }
}

Microgear::Microgear(Client& tcpclient ) {
  mg = this;
  this->client = &tcpclient;
}

void Microgear::init(char *key, char *secret, char *alias) {
    this->key = key;
    this->secret = secret;
    this->token = NULL;
    this->tokensecret = NULL;
    this->alias = alias;

    this->cb_connected = NULL;
    this->cb_absent = NULL;
    this->cb_present = NULL;
    this->cb_message = NULL;
    this->cb_error = NULL;
    this->cb_info = NULL;
}

int Microgear::connect(char* appid) {
    Token tokenstore;
    char *p;
    char mqtt_username[MQTT_USERNAME_SIZE+1];
    char raw_password[20];
    char mqtt_password[MQTT_PASSWORD_SIZE+1];
    char hashkey[SECRETSIZE+TOKENSECRETSIZE+2];

    AuthClient *auth = new AuthClient(*(this->client));

    this->appid = appid;
    topicprefixlen = strlen(appid)+1;

    if (this->token  == NULL) {
        if (!auth->getAccessToken(&tokenstore, this->appid, this->key, this->secret, this->alias)) {
            return -1;
        }
  }
  else {
      // if token has been manually assigned by api
      strcpy(tokenstore.token, this->token);
      strcpy(tokenstore.secret, this->tokensecret);
      strcpy(tokenstore.saddr, this->host);
      tokenstore.sport = this->port;
  }

  setTime(auth->getServerTime());

  this->mqttclient.begin(tokenstore.saddr, tokenstore.sport,*(this->client));

  if (this->token) strcpy(tokenstore.token, this->token);
  if (this->tokensecret) strcpy(tokenstore.secret, this->tokensecret);

  strrep(mqtt_username, tokenstore.token);
  p = addattr(mqtt_username+strlen(tokenstore.token), "%", this->key);
  addattr(p, "%", getTimeStr());

//  addattr(p, "%", "1498324944");
  strrep(hashkey, tokenstore.secret);
  addattr(hashkey+strlen(tokenstore.secret), "&", this->secret);

  Sha1.initHmac((uint8_t*)hashkey,strlen(hashkey));
  Sha1.HmacBase64(mqtt_password,mqtt_username);

  #ifdef DEBUG
      Serial.println("Going to connect to MQTT broker");
      Serial.println(tokenstore.token);
      Serial.println(mqtt_username);
      Serial.println(mqtt_username+strlen(tokenstore.token)+1);
      Serial.println(mqtt_password);
      Serial.println(hashkey);
  #endif

  int ret = this->mqttclient.connect(tokenstore.token, mqtt_username+strlen(tokenstore.token)+1, mqtt_password);

  delete auth;
  return ret == MQTT::SUCCESS;
}

bool Microgear::connected() {
    return this->mqttclient.connected();
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

void Microgear::on(unsigned char event, CALLBACK callback) {
    switch (event) {
        case MESSAGE :
                if (callback) cb_message = callback;
                break;
        case PRESENT :
                if (callback) cb_present = callback;
                if (connected())
                    subscribe("/&present");
                break;
        case ABSENT :
                if (callback) cb_absent = callback;
                if (connected())
                    subscribe("/&absent");
                break;
        case CONNECTED :
                if (callback) cb_connected = callback;
                break;
        case ERROR :
                if (callback) cb_error = callback;
                break;
        case INFO :
                if (callback) cb_info = callback;
                break;
    }
}

void Microgear::subscribe(char* topic) {
    char top[MAXTOPICSIZE] = "/";

    strcat(top,appid);
    strcat(top,topic);
    mqttclient.subscribe(top);
}

void Microgear::unsubscribe(char* topic) {
    char top[MAXTOPICSIZE] = "/";

    strcat(top,appid);
    strcat(top,topic);
    mqttclient.unsubscribe(top);
}

void Microgear::loop() {
  this->mqttclient.loop();
}

bool Microgear::publish(char* topic, char* message, bool retained) {
    char top[MAXTOPICSIZE] = "/";
    addattr(top+1, this->appid, topic);
    return this->mqttclient.publish(top, message, retained);
}

bool Microgear::publish(char* topic, char* message) {
    return publish(topic, message, false);
}

bool Microgear::publish(char* topic, double message, int n) {
    return publish(topic, message, n, false);
}

bool Microgear::publish(char* topic, double message, int n, bool retained) {
    char mstr[16];
    dtostrf(message,0,n,mstr);
    return publish(topic, mstr, retained);
}

bool Microgear::publish(char* topic, double message) {
    return publish(topic, message, 8, false);
}

bool Microgear::publish(char* topic, double message, bool retained) {
    return publish(topic, message, 8, retained);
}

bool Microgear::publish(char* topic, int message) {
    return publish(topic, message, 0, false);
}

bool Microgear::publish(char* topic, int message, bool retained) {
    return publish(topic, message, 0, retained);
}

bool Microgear::publish(char* topic, String message) {
    return publish(topic, message, false);
}

bool Microgear::publish(char* topic, String message, bool retained) {
    message.toCharArray(pbuffer,MAXPAYLOADSIZE-1);
    return publish(topic, pbuffer, retained);
}

void MicroGear::setAlias(char* gearalias) {
    char top[MAXTOPICSIZE];
    strcpy(top,"/@setalias/");
    strcat(top,gearalias);
    this->alias = alias;
    publish(top,"");
}

bool Microgear::chat(char* target, char* message) {
    bool result;
    char chattopic[MAXALIASSIZE+11];
    addattr(chattopic,"/gearname/",target);
    result = this->publish(chattopic, message);
    this->mqttclient.loop();
    return result;
}

bool Microgear::chat(char* topic, double message, int n) {
    bool result;
    char mstr[16];

    dtostrf(message,0,n,mstr);
    result = chat(topic, mstr);
    this->mqttclient.loop();
    return result;
}

bool Microgear::chat(char* topic, double message) {
    return chat(topic, message, 8);
}

bool Microgear::chat(char* topic, int message) {
    return chat(topic, message, 0);
}

bool Microgear::chat(char* topic, String message) {
    message.toCharArray(pbuffer,MAXPAYLOADSIZE);
    return chat(topic, pbuffer);
}
