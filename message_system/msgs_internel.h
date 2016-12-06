#ifndef MSGS_INTERNEL_H
#define MSGS_INTERNEL_H

//to indicate a bradcast topic or request topic
enum topic_type {
	TOPIC_BROAD = 0,
	TOPIC_P2P,
	TOPIC_MODULE,//each module has its own topic, p2p message respond will send back to this topic
	TOPIC_COUNT,
};
static char* topic_prefix[TOPIC_COUNT] = {
	[TOPIC_BROAD] = "BroCa_",
	[TOPIC_P2P] = "P2p_",
	[TOPIC_MODULE] = "Modu_",
};

#endif
