#ifndef MQTT_H
#define MQTT_H

#include "MQTTClient.h"

int mqtt_subscribe_topic(MQTTClient client, const char* topic,int qos);
int mqtt_client_connect(MQTTClient* , const char* , const char* , MQTTClient_connectionLost* ,
														MQTTClient_messageArrived* , MQTTClient_deliveryComplete* );
int mqtt_client_disconnect(MQTTClient* pClient);
int mqtt_publish_message(MQTTClient client, const char* topic,const char* message, int qos);
#endif
