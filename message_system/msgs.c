#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <unistd.h>
#include <pthread.h>
#include "mqtt.h"
#include "LinkList.h"
#include "errno.h"
#include "msgs_internel.h"
#include "my_json.h"
#include "msgs.h"

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "ExampleClientSubaa"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

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

//store  the handle function of a broadcast msg
struct BroMsgCbMap {
	char msg[32];
	void (*callback)(const char *,const char *);
};

//similar to struct BroMsgCbMap
struct ReqMsgCbMap {
	char msg[32];
	long (*callback)(const char *);
};
struct P2pMsgTask {
	int trigger;	//when received respond ,trigger will be 1
	char* msg;
	char* data;
	char* sender;
	int seq;
	int err;
};

static char g_clientid[32];
static int g_counter = 0;
static int g_init = 0;
//linklist of struct BroMsgCbMap, store all handle function of broadcast msgs
static struct LinkList* g_bromsg_cb_map_list = NULL;
//similar to g_bromsg_cb_map_list
static struct LinkList* g_reqmsg_cb_map_list = NULL;
//linklist of struct P2pMsgTask, store all task which comes from other module and need to be handle
static struct LinkList* g_p2pmsg_camein_list = NULL;
//linklist of struct P2pMsgTask, store all tasks this module sent out and waiting for respond
static struct LinkList* g_p2pmsg_sentout_list = NULL;
static MQTTClient g_client = 0;
static volatile MQTTClient_deliveryToken deliveredtoken = 0;
static pthread_t g_taskhandle_thread = 0;
static pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
static int last_lock_line = 0;
#define MSG_LOCK						\
	do {                                                            \
		int i = 0;                                              \
									\
		while (pthread_mutex_trylock(&msg_mutex)) {                   \
			if (i++ >= 100) {                               \
				LOG_E("#### {%s, %s, %d} dead lock (last: %d)####\n", \
				       __FILE__, __func__, __LINE__, last_lock_line); \
				i = 0;                                  \
			}                                               \
			usleep(100 * 1000);                             \
		}                                                       \
		last_lock_line = __LINE__;				\
	} while (0)

#define MSG_UNLOCK do{pthread_mutex_unlock(&msg_mutex);}while(0)

static int bromsg_compare(void* ptr1, void* ptr2)
{
	char* msg = (char*)ptr1;
	struct BroMsgCbMap* msg_cb_map = (struct BroMsgCbMap*)ptr2;

	if (!msg || !msg_cb_map) {
		LOG_E("msg or msg_cb_map is null\n");
		return -1;
	}		
	if (strcmp(msg, msg_cb_map->msg) == 0) {
		return 0;
	}
	return -1;
}

static int p2pmsg_compare(void* ptr1, void* ptr2)
{
	char* msg = (char*)ptr1;
	struct ReqMsgCbMap* msg_cb_map = (struct ReqMsgCbMap*)ptr2;

	if (!msg || !msg_cb_map) {
		LOG_E("msg or msg_cb_map is null\n");
		return -1;
	}		
	if (strcmp(msg, msg_cb_map->msg) == 0) {
		return 0;
	}
	return -1;
}

static struct BroMsgCbMap *search_bromsg_handler(char* msg)
{
	struct BroMsgCbMap *msg_cb_map = NULL;
	msg_cb_map = linklist_search(g_bromsg_cb_map_list, msg, bromsg_compare);
	if (!msg_cb_map) {
		LOG_I("cant find the handler of broadcast message '%s', ignore\n", msg);
		return NULL;
	}
	return msg_cb_map;
}

static struct ReqMsgCbMap *search_p2pmsg_handler(char* msg)
{
	struct ReqMsgCbMap *msg_cb_map = NULL;
	msg_cb_map = linklist_search(g_reqmsg_cb_map_list, msg, p2pmsg_compare);
	if (!msg_cb_map) {
		LOG_I("cant find the handler of request message '%s', ignore\n", msg);
		return NULL;
	}
	return msg_cb_map;
}

static void free_p2pmsg_task(void* arg) 
{
	if (!arg) {
		return ;
	}
	struct P2pMsgTask* task = (struct P2pMsgTask*)arg;
	if (task->sender)
		safe_free(task->sender);
	if (task->msg)
		safe_free(task->msg);
	if (task->data)
		safe_free(task->data);
	safe_free(task);
	return ;
}

static struct P2pMsgTask* make_p2p_msg_task(const char* sender, const char* message, const char* data, int id, int err)
{
	struct P2pMsgTask* task = calloc(1, sizeof(struct P2pMsgTask));
	if (!task) {
		LOG_E("malloc (%ld) bytes memory failed\n", sizeof(struct P2pMsgTask));
		goto err;
	}
	if (asprintf(&task->data, "%s", data) < 0) {
		LOG_E("malloc memory failed\n");
		goto err;
	}
	if (asprintf(&task->msg, "%s", message) < 0) {
		LOG_E("malloc memory failed\n");
		goto err;
	}
	if (asprintf(&task->sender, "%s", sender) < 0) {
		LOG_E("malloc memory failed\n");
		goto err;
	}
	task->seq = id;
	task->err = err;
	return task;
err:
	free_p2pmsg_task(task);
	return NULL;
}

//obtain the message from a topic
//return pointer need not be free by caller
static char* parse_msg(char* topic, int *topic_type)
{
	char* tmp = NULL;
	char* msg = NULL;
	int i = 0;
	for (i = 0; i < TOPIC_COUNT; ++i) {
		if ( (tmp = strstr(topic, topic_prefix[i])) != NULL && tmp == topic) {
			msg = topic + strlen(topic_prefix[i]);
			*topic_type = i;
			return msg;
		}
	}
	LOG_E("unexpected topic(%s)\n", topic);
	return NULL;
}

static char* ajson_to_string(JObj* json)
{
	const char *json_string = JSON_TO_STRING(json);
	if(json_string == NULL) {
		LOG_E("JSON_TO_STRING failed\n");
		goto err;
	}
	char* new_json_string = NULL;
	asprintf(&new_json_string, "%s", json_string);
	if (!new_json_string) {
		LOG_E("asprintf failed\n");
		goto err;
	}
	return new_json_string;
err:
	return NULL;
}

static char*  make_p2p_message_json(char* modulename, int id, const char* msg, const char* req_data, int err)
{	
	JObj *request_json=JSON_NEW_EMPTY_OBJECT();
	JSON_ADD_OBJECT(request_json,"sender",JSON_NEW_OBJECT(modulename,string));
	JSON_ADD_OBJECT(request_json,"id",JSON_NEW_OBJECT(id,int));
	JSON_ADD_OBJECT(request_json,"data",JSON_NEW_OBJECT(req_data,string));
	JSON_ADD_OBJECT(request_json,"msg",JSON_NEW_OBJECT(msg,string));
	JSON_ADD_OBJECT(request_json,"err",JSON_NEW_OBJECT(err,int));
	char *request_string = ajson_to_string(request_json);
	if (request_string == NULL) {
		JSON_PUT_OBJECT(request_json);
		goto err;
	}
	JSON_PUT_OBJECT(request_json);
	return request_string;
err:
	return NULL;
}

static struct P2pMsgTask* parse_json_to_p2p_task(char* json_string)
{
	JObj* json = JSON_PARSE(json_string);
	if (!json) {
		LOG_E("json is null\n");
		goto err;
	}
	JObj *header_json = JSON_GET_OBJECT(json,"sender");
	JObj *id_json = JSON_GET_OBJECT(json,"id");
	JObj *data_json = JSON_GET_OBJECT(json,"data");
	JObj *msg_json = JSON_GET_OBJECT(json,"msg");
	JObj *err_json = JSON_GET_OBJECT(json,"err");
	if (!header_json || !id_json || !data_json|| !err_json|| !err_json) {
		LOG_E("json header(%p)/id(%p)/data(%p)/err(%p)/msg(%p) is null",
				header_json,id_json,data_json,err_json, msg_json);
		goto err;
	}
	const char* sender = JSON_GET_OBJECT_VALUE(header_json, string);
	int id = JSON_GET_OBJECT_VALUE(id_json, int);
	const char* data = JSON_GET_OBJECT_VALUE(data_json, string);
	const char* msg = JSON_GET_OBJECT_VALUE(msg_json, string);
	int err1 = JSON_GET_OBJECT_VALUE(err_json, int);
	if (!sender || !data || !msg) {
		LOG_E("sender(%p)/data(%p)/msg(%p) is null\n",
				sender, data, msg);
		goto err;
	}
	
	struct P2pMsgTask* task = make_p2p_msg_task(sender, msg, data, id, err1);
	if (!task)
		goto err;

	JSON_PUT_OBJECT(json);
	return task;
err:
	if (json)
		JSON_PUT_OBJECT(json);
	return NULL;
}

static void announce_respond_arrived(int seq, char* respond_data,int err1)
{
	MSG_LOCK;
	int count = linklist_get_count(g_p2pmsg_sentout_list);
	int i = 0;
	for (; i < count; ++i) {
		struct P2pMsgTask* task = (struct P2pMsgTask*)linklist_visit(g_p2pmsg_sentout_list, i);
		if (task && task->seq == seq) {
			LOG_I("respond id(%d) has received\n", seq);
			task->trigger = 1;
			task->err = err1;
			safe_free(task->data);
			task->data = respond_data;
			break;
		}	
	}
	MSG_UNLOCK;
	return ;
}

static int handle_p2p_respond(int id ,const char* respond_data, int err1)
{	
	char* new_respond_data = NULL;
	if (err1==0) {
		if (asprintf(&new_respond_data, "%s", respond_data) < 0) {
			LOG_E("malloc (%d) bytes memory failed\n",(int)strlen(respond_data));
			goto err;
		}
	}else {
		LOG_E("receive error, reason:%s\n", respond_data);
	}
	announce_respond_arrived(id, new_respond_data, err1);
	return 0;
err:
	return -1;
}

int respond_p2p_message(char* topicName, struct P2pMsgTask* task)
{
	char* respond_json_data = NULL;
	respond_json_data = make_p2p_message_json(g_clientid, task->seq, task->msg, task->data, task->err);
	if (respond_json_data == NULL)
		goto err;
	LOG_I("respond json data:%s\n", respond_json_data);
	if (mqtt_publish_message(g_client, topicName, respond_json_data, QOS) < 0) {
		safe_free(respond_json_data);
		goto err;
	}
	safe_free(respond_json_data);
	return 0;
err:
	return -1;
}

static int handle_topic_broadcast(char* msg, char* data)
{
	struct BroMsgCbMap *msg_cb_map = NULL;
	MSG_LOCK;
	if ( (msg_cb_map = search_bromsg_handler(msg)) == NULL) {
		MSG_UNLOCK;
		goto err;
	}
	void (*callback)(const char *,const char *) = msg_cb_map->callback;
	MSG_UNLOCK;
	callback(msg, data);
err:
	return -1;
}

static int handle_topic_p2p(char* topicName, char* msg, char* json_string)
{
	struct P2pMsgTask* task = NULL;
	if ( (task = parse_json_to_p2p_task(json_string)) == NULL) {
		goto err;
	}
	
	MSG_LOCK;
	if ( linklist_add(g_p2pmsg_camein_list, task) < 0) {
		LOG_E("linklist add failed\n");
		MSG_UNLOCK;
		free_p2pmsg_task(task);
		goto err;
	}
	MSG_UNLOCK;
	
	return 0;
err:
	return -1;
}

static int handle_topic_module(char* topicName, char* msg, char* json_string)
{
	struct P2pMsgTask* task = NULL;
	if ( (task = parse_json_to_p2p_task(json_string)) == NULL)
		goto err;	
	if (handle_p2p_respond(task->seq, task->data, task->err) < 0)
		goto err;
	free_p2pmsg_task(task);
	return 0;
err:
	return -1;
}

//determine the topic according to the message
//return ptr must be free by caller
static char* make_topic(const char* msg, const char* _topic_prefix)
{
	int size = strlen(msg)+strlen(_topic_prefix) + 1;
	char* topic = calloc(size, 1);
	if (!topic) {
		LOG_E("malloc (%d) bytes memory failed\n", size);
		return NULL;
	}
	snprintf(topic, size, "%s%s", _topic_prefix, msg);
	return topic;
}
static char* make_topic_p2p_respond(char* module_name)
{
	return make_topic(module_name ,topic_prefix[TOPIC_MODULE]);
}

static int subscribe_topic(const char* broadcast_message, int topictype)
{
	char* topic = NULL;
	topic = make_topic(broadcast_message, topic_prefix[topictype]);
	if (!topic) 
		return -1;
	int ret =0;
	if ( (ret = mqtt_subscribe_topic(g_client, topic, QOS)) < 0) {
		LOG_E("subscribe topic %s failed, %d\n", topic, ret);
		goto err;
	}
	safe_free(topic);
	return 0;
err:
	safe_free(topic);
	return -1;
}

static int unsubscribe_topic(const char* topicname, int topictype)
{
	char* topic = NULL;
	topic = make_topic(topicname, topic_prefix[topictype]);
	if (!topic) 
		return -1;
	int ret =0;
	if ( (ret = MQTTClient_unsubscribe(g_client, topic)) < 0) {
		LOG_E("subscribe topic %s failed, %d\n", topic, ret);
		goto err;
	}
	safe_free(topic);
	return 0;
err:
	safe_free(topic);
	return -1;
}

//subscribe a mqtt topic for a broadcast message; each broadcast message has a corresponing topic.
static int subscribe_topic_broadmsg(const char* broadcast_message)
{
	return subscribe_topic(broadcast_message, TOPIC_BROAD);
}

//similar to subscribe_topic_broadmsg
static int subscribe_topic_request(const char* broadcast_message)
{
	return subscribe_topic(broadcast_message, TOPIC_P2P);
}

//similar to subscribe_topic_broadmsg
static int subscribe_topic_module(const char* moduleid)
{
	return subscribe_topic(moduleid, TOPIC_MODULE);
}
static int unsubscribe_topic_module(const char* moduleid)
{
	return unsubscribe_topic(moduleid, TOPIC_MODULE);
}

static int add_broadcast_cb(const char* broadcast_message,
								void (*broadcast_listener)(const char *,const char *))
{
	struct BroMsgCbMap* msg_cb_map = calloc(sizeof(struct BroMsgCbMap), 1);
	if (!msg_cb_map) {
		LOG_E("malloc (%d) bytes memory failed\n", (int)sizeof(struct BroMsgCbMap));
		goto err;
	}
	snprintf(msg_cb_map->msg, sizeof(msg_cb_map->msg), "%s", broadcast_message);
	msg_cb_map->callback = broadcast_listener;
	MSG_LOCK;
	if (linklist_add(g_bromsg_cb_map_list, msg_cb_map) < 0) {
		LOG_E("linklist_add fail, errno = %d\n", errno);
		safe_free(msg_cb_map);
		MSG_UNLOCK;
		goto err;
	}
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

static int add_request_cb(const char* messageListenerName,long (*message_listener)(const char* ))
{
	struct ReqMsgCbMap* msg_cb_map = calloc(sizeof(struct ReqMsgCbMap), 1);
	if (!msg_cb_map) {
		LOG_E("malloc (%d) bytes memory failed\n", (int)sizeof(struct ReqMsgCbMap));
		goto err;
	}
	snprintf(msg_cb_map->msg, sizeof(msg_cb_map->msg), "%s", messageListenerName);
	msg_cb_map->callback = message_listener;
	MSG_LOCK;
	if (linklist_add(g_reqmsg_cb_map_list, msg_cb_map) < 0) {
		LOG_E("linklist_add fail, errno = %d\n", errno);
		MSG_UNLOCK;
		goto err;
	}
	MSG_UNLOCK;
	return 0;
err:
	safe_free(msg_cb_map);
	return -1;
}

static int generate_client_id()
{
	int pid = getpid();
	int tid = pthread_self();
	snprintf(g_clientid, sizeof(g_clientid), "%d-%d", pid, tid);
	LOG_I("clientid = %s\n", g_clientid);
	return 0;
}

static int make_request_id()
{
	MSG_LOCK;
	int id = g_counter++;
	MSG_UNLOCK;
	return id;
}

static int function_properly()
{
	MSG_LOCK;
	if (0 == g_init) {
		LOG_E("not init yet!\n");
		MSG_UNLOCK;
		return -1;
	}
	MSG_UNLOCK;
	return 0;
}

static int wait_response(int seq, char **response, int timeout_ms)
{
	int ret = 0;
	int count = 0;
	int sleep_us = 100000;
	while(1) {
		usleep(sleep_us);
		count ++;
		MSG_LOCK;
		int i = 0;
		struct P2pMsgTask* task = NULL;
		for (; i < linklist_get_count(g_p2pmsg_sentout_list); ++i) {
			task = linklist_visit(g_p2pmsg_sentout_list,i);
			if (task && task->seq == seq) 
				break;
		}
		if (!task || task->seq != seq) {
			LOG_E("no task (id = %d) in list\n", seq);
			MSG_UNLOCK;
			ret = -1;
			break;
		}
		if (1 == task->trigger) {
			*response = task->data;
			task->data = NULL;
			if (task->err !=0 )
				ret = -1;
			linklist_del(g_p2pmsg_sentout_list, task, free_p2pmsg_task);
			MSG_UNLOCK;
			break;
		}
		if (timeout_ms > 0 && count * sleep_us >= timeout_ms*1000) {
			LOG_E("wait for respond timeout\n");
			linklist_del(g_p2pmsg_sentout_list, task, free_p2pmsg_task);
			ret = -1;
			MSG_UNLOCK;
			break;
		}
		MSG_UNLOCK;
	}
	return ret;
}

static int handle_a_request(struct P2pMsgTask *task, long (*callback)(const char *))
{
	char* resp_data = (char*)callback(task->data);
	if (!resp_data) {
		task->err = -1;
		asprintf(&resp_data, "p2p message handler return NULL");
		if (!resp_data) {
			LOG_E("malloc failed\n");
			goto err;
		}
	}
	LOG_I("respond data: %s\n", resp_data);
	safe_free(task->data);
	task->data = resp_data;
	char* topicName = make_topic_p2p_respond(task->sender);
	if (!topicName) {
		goto err;
	}
	if (respond_p2p_message(topicName, task) < 0) {
		safe_free(topicName);
		goto err;
	}
	safe_free(topicName);
	return 0;
err:
	return -1;
}

static void* request_handle(void* args)
{
	while(1) {
		usleep(100000);
		MSG_LOCK;
		if (linklist_get_count(g_p2pmsg_camein_list) == 0) {
			MSG_UNLOCK;
			continue;
		}
		MSG_UNLOCK;
		LOG_I("there are tasks need to be handled\n");
		while(1) {
			MSG_LOCK;
			if (linklist_get_count(g_p2pmsg_camein_list) <= 0) {
				LOG_I("all tasks has been handled\n");
				MSG_UNLOCK;
				break;
			}
			struct P2pMsgTask *task = linklist_visit(g_p2pmsg_camein_list, 0);
			if (!task) {
				LOG_E("linklist cannot found 0\n");
				MSG_UNLOCK;
				break;
			}
			LOG_I("handling request{sender = %s, msg = %s, data = %s, seq = %d}\n,",
					task->sender,task->msg, task->data, task->seq);
			struct ReqMsgCbMap *msg_cb_map = NULL;
			long (*callback)(const char *) = NULL;
			if ( (msg_cb_map = search_p2pmsg_handler(task->msg)) != NULL)
				callback = msg_cb_map->callback;
			struct P2pMsgTask *task_copy = make_p2p_msg_task(task->sender, task->msg, task->data, task->seq, task->err);
			if (!task_copy) {
				MSG_UNLOCK;
				continue;
			}
			if (linklist_del_index(g_p2pmsg_camein_list, 0, free_p2pmsg_task) < 0)
				LOG_E("linklist del failed, reason:%s\n", strerror(errno));
			MSG_UNLOCK;
			if (!callback) {
				LOG_E("cannot handle the task\n");
			}
			else
				handle_a_request(task_copy, callback);
			free_p2pmsg_task(task_copy);
		}
	}
	return NULL;
}

static int do_send_request(const char* msg, const char* topic, int id, const char* data)
{
	char* json_data = make_p2p_message_json(g_clientid, id, msg, data, 0);
	if (!json_data) {
		goto err;
	}
	if (mqtt_publish_message(g_client, topic, json_data, QOS) < 0) {
		safe_free(json_data);
		goto err;
	}
	safe_free(json_data);
	return 0;
err:
	return -1;
}

int add_sentout_request(int id)
{
	struct P2pMsgTask * task = make_p2p_msg_task("", "", "", id, 0);
	if (!task) {
		goto err;
	}
	MSG_LOCK;
	if (linklist_add(g_p2pmsg_sentout_list, task) < 0) {
		LOG_E("linklist_add failed\n");
		MSG_UNLOCK;
		goto err;
	}
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

static void print_brocast_msg_map(void *args)
{
	struct BroMsgCbMap* map = (struct BroMsgCbMap*)args;
	printf("{%s, %p}\n", map->msg, map->callback);
	return ;
}
static void print_p2p_msg(void *args)
{
	struct ReqMsgCbMap* map = (struct ReqMsgCbMap*)args;
	printf("{%s, %p}\n", map->msg, map->callback);
	return ;
}

static void print_p2p_msg_comein(void* args)
{
	struct P2pMsgTask* p2p = (struct P2pMsgTask*)args;
	printf("{%s,%s,%s,%d}\n", p2p->msg, p2p->data, p2p->sender, p2p->seq);
	return ;
}

static void print_p2p_msg_sentout(void* args)
{
	struct P2pMsgTask* p2p = (struct P2pMsgTask*)args;
	printf("{%s,%s,%s,%d}\n", p2p->msg, p2p->data, p2p->sender, p2p->seq);
	return ;
}

//////////////////////////////////////////////////////////////////////////////
static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	char* data = NULL;
	char* msg = NULL;
	if (!topicName  || !message) {
		LOG_E("one of arguments:topicName(%p), Message(%p) is null, msgarrvd failed\n", topicName, message);
		goto err;
	}
	LOG_I("Message arrived, topic = %s\n", topicName);
    LOG_I("   message: ");
    int i;
    char* payloadptr;
    payloadptr = message->payload;
	if (payloadptr) {
	    for(i=0; i<message->payloadlen; i++)
	    {
	        LOG_I("%c", payloadptr[i]);
	    }
    	LOG_I("\n");
	}
	
	int topic_type = 0;
	msg = parse_msg(topicName, &topic_type);
	if (!msg) {
		goto err;
	}
	int size = message->payloadlen+1;
	data = calloc(1, size);
	if (!data) {
		LOG_E("malloc (%d) bytes memory failed\n",size);
		goto err;
	}
	snprintf(data, size, "%s", (char*)message->payload);
	
	if (topic_type == TOPIC_BROAD) {
		if (handle_topic_broadcast(msg, data) < 0)
			goto err;
	}
	else if (topic_type == TOPIC_P2P) {
		if (handle_topic_p2p(topicName, msg, data) < 0) {
			goto err;
		}
	}
	else if (topic_type == TOPIC_MODULE) {
		if (handle_topic_module(topicName, msg, data) < 0) {
			goto err;
		}
	}
err:
	if (message)
		MQTTClient_freeMessage(&message);
	if (topicName)
		MQTTClient_free(topicName);
	safe_free(data);
    return 1;
}

static void connlost(void *context, char *cause)
{
    LOG_E("\nConnection lost\n");
    LOG_E("     cause: %s\n", cause);
	return;
}

static void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
	return;
}

int send_broadcast_message(const char* broadcast_message,const char *data)
{
	if (function_properly() < 0)
		goto err;
	char* topic = NULL;
	if (!broadcast_message) {
		LOG_E("broadcast_message is NULL, please check\n");
	}
	if (data == NULL)
		data = "";

	topic = make_topic(broadcast_message, topic_prefix[TOPIC_BROAD]);
	if (!topic) 
		goto err;
	if (mqtt_publish_message(g_client, topic, data, QOS) < 0) {
		safe_free(topic);
		goto err;
	}
	safe_free(topic);
	return 0;
err:
	return -1;
}

int add_broadcast_listener(const char* broadcast_message,
								void (*broadcast_listener)(const char* ,const char*))
{
	if (function_properly() < 0)
		goto err;
	if (broadcast_message == NULL || broadcast_listener == NULL) {
		LOG_E("broadcast_message(%p)/broadcast_listener(%p) is NULL, error\n", broadcast_message, broadcast_listener);
		goto err;
	}

	if (add_broadcast_cb(broadcast_message, broadcast_listener)) {
		goto err;
	}

	if (subscribe_topic_broadmsg(broadcast_message) < 0)
		goto err;

	return 0;
err:
	return -1;
}

int add_p2p_message_listener(const char* messageListenerName,long (*message_listener)(const char* ))
{
	if (function_properly() < 0)
		goto err;
	if (messageListenerName == NULL || message_listener == NULL) {
		LOG_E("broadcast_message(%p)/broadcast_listener(%p) is NULL, error\n", messageListenerName, message_listener);
		goto err;
	}

	if (add_request_cb(messageListenerName, message_listener)) {
		goto err;
	}

	if (subscribe_topic_request(messageListenerName) < 0)
		goto err;

	return 0;
err:
	return -1;
}

int show_broadcast_listener()
{
	if (function_properly() < 0)
		goto err;
	printf("broadcast listers:\n");
	MSG_LOCK;
	linklist_travel(g_bromsg_cb_map_list, print_brocast_msg_map);
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

int show_p2pmessage_listener()
{
	if (function_properly() < 0)
		goto err;
	printf("p2p message listers:\n");
	MSG_LOCK;
	linklist_travel(g_reqmsg_cb_map_list, print_p2p_msg);
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

int show_p2pmsg_comein_list()
{
	if (function_properly() < 0)
		goto err;
	printf("p2p message come in:\n");
	MSG_LOCK;
	linklist_travel(g_p2pmsg_camein_list, print_p2p_msg_comein);
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

int show_p2pmsg_sentout_list()
{
	if (function_properly() < 0)
		goto err;
	printf("p2p message send out:\n");
	MSG_LOCK;
	linklist_travel(g_p2pmsg_sentout_list, print_p2p_msg_sentout);
	MSG_UNLOCK;
	return 0;
err:
	return -1;
}

int send_p2p_message(const char* messageListenerName,const char *data,char **response, int timeout_ms)
{
	//int ret;
	if (function_properly() < 0)
		goto err;
	if (!messageListenerName) {
		LOG_E("broadcast_message is NULL, please check\n");
		goto err;
	}
	if (data == NULL)
		data = "";
	
	int id = make_request_id();
	char* topic = NULL;
	topic = make_topic(messageListenerName, topic_prefix[TOPIC_P2P]);
	if (!topic) 
		goto err;

	if (add_sentout_request(id) < 0) {
		safe_free(topic);
		goto err;
	}
	if (do_send_request(messageListenerName, topic, id, data) < 0) {
		safe_free(topic);
		goto err;
	}
	if (wait_response(id, response, timeout_ms) < 0) {
		safe_free(topic);
		goto err;
	}
	safe_free(topic);
	return 0;
err:
	return -1;
}

void msgs_destroy()
{
	MSG_LOCK;
	pthread_cancel(g_taskhandle_thread);
 	pthread_join(g_taskhandle_thread, NULL);
	bzero(&g_taskhandle_thread, sizeof(g_taskhandle_thread));
	LOG_I("mqtt_client_disconnect\n");
	unsubscribe_topic_module(g_clientid);
	mqtt_client_disconnect(&g_client);
	LOG_I("mqtt_client_disconnect over\n");
	linklist_destroy(&g_reqmsg_cb_map_list, free);
	linklist_destroy(&g_bromsg_cb_map_list, free);
	linklist_destroy(&g_p2pmsg_camein_list, free_p2pmsg_task);
	linklist_destroy(&g_p2pmsg_sentout_list, free_p2pmsg_task);
	bzero(g_clientid, sizeof(g_clientid) );
	g_init = 0;
	MSG_UNLOCK;
	return ;
}

int msgs_init()
{ 
	MSG_LOCK;
	if (g_init) {
		LOG_E("has inited\n");
		goto err;
	}
	generate_client_id();
	if ((g_bromsg_cb_map_list = linklist_create()) == NULL) {
		LOG_E("linklist_create faile errno:%d\n", errno);
		goto err;
	}
	if ((g_reqmsg_cb_map_list = linklist_create()) == NULL) {
		LOG_E("linklist_create faile errno:%d\n", errno);
		goto err_bromsg_cb_map_list;
	}
	if ((g_p2pmsg_camein_list = linklist_create()) == NULL) {
		LOG_E("linklist_create faile errno:%d\n", errno);
		goto err_reqmsg_cb_map_list;
	}

	if ((g_p2pmsg_sentout_list = linklist_create()) == NULL) {
		LOG_E("linklist_create faile errno:%d\n", errno);
		goto err_task_list;
	}
	if (mqtt_client_connect(&g_client, ADDRESS, g_clientid, connlost, msgarrvd, delivered) < 0)
		goto err_task_sent_list;

	if (subscribe_topic_module(g_clientid) < 0)
		goto err_client_conn;
	if((pthread_create(&g_taskhandle_thread, NULL, request_handle, NULL)) != 0) {
       LOG_E("pthread_create failed, reson:%s\n", strerror(errno));
	   goto err_unsub_module;
   }
	g_init = 1;
	MSG_UNLOCK;
	return 0;
err_unsub_module:
	unsubscribe_topic_module(g_clientid);
err_client_conn:	
	LOG_I("mqtt_client_disconnect\n");
	mqtt_client_disconnect(&g_client);
	LOG_I("mqtt_client_disconnect over\n");	
err_task_sent_list:
	linklist_destroy(&g_p2pmsg_sentout_list, NULL);
err_task_list:
	linklist_destroy(&g_p2pmsg_camein_list, NULL);
err_reqmsg_cb_map_list:
	linklist_destroy(&g_reqmsg_cb_map_list, NULL);
err_bromsg_cb_map_list:
	linklist_destroy(&g_bromsg_cb_map_list, NULL);
err:
	MSG_UNLOCK;
	return -1;
}

const char* get_lib_version()
{
	return LIB_VERSION;
}
