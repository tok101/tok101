#include "mqtt.h"
static MQTTClient g_client;

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT Examples"
#define REQUESTTOPIC       "RequestTopic"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}


int main(int argc, char* argv[])
{
	if (mqtt_client_connect(&g_client, ADDRESS, CLIENTID, connlost, msgarrvd, delivered) < 0)
		goto err;
	if (mqtt_subscribe_topic(g_client, TOPIC, QOS) < 0) {
		mqtt_client_disconnect(&g_client);
		goto err;
	}
	if (mqtt_subscribe_topic(g_client, REQUESTTOPIC, QOS) < 0) {
		mqtt_client_disconnect(&g_client);
		goto err;
	}
	while(1);
	return 0;
err:
	return -1;
}

