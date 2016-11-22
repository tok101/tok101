#include "mqtt.h"
#include <string.h>
#define LOG_E(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_I(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_W(fmt, args...) fprintf(stderr, fmt, ##args); 

#define safe_free(p) do{\
    if((p) != NULL)\
    {\
        free((p));\
        (p) = NULL;\
    }\
    }while(0)

int mqtt_subscribe_topic(MQTTClient client, const char* topic,int qos)
{
	
    return MQTTClient_subscribe(client, topic, qos);
	
}
	
int mqtt_publish_message(MQTTClient client, const char* topic,const char* message, int qos)
{
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    pubmsg.payload = (void*)message;
    pubmsg.payloadlen = strlen(message);
    pubmsg.qos = qos;
    pubmsg.retained = 0;
    return MQTTClient_publishMessage(client, topic, &pubmsg, &token);
}
int mqtt_client_connect(MQTTClient* pClient, const char* address, const char* clientid , MQTTClient_connectionLost* connlost,
														MQTTClient_messageArrived* msgarrvd, MQTTClient_deliveryComplete* delivered)
{
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;
//	  int ch;

	MQTTClient_create(pClient, address, clientid,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;

	MQTTClient_setCallbacks(*pClient, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(*pClient, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		LOG_E("Failed to connect, return code %d\n", rc);
		MQTTClient_destroy(pClient);
		return -1;		 
	}
	return rc;
}


int mqtt_client_disconnect(MQTTClient* pClient)
{
	if (!*pClient)
		return 0;
    MQTTClient_disconnect(*pClient, 10000);
    MQTTClient_destroy(pClient);
	return 0;
}

