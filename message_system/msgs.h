/**
 * 消息服务
 */
#ifndef _MSGS_H_
#define _MSGS_H_


#define LIB_VERSION "1.0.0"
/**
 * 关机消息事件
 */
#define BROADCAST_MESSAGE_SHUTDOWN "shutdown"
/**
 * todo
 * 下面可以继续添加可以监听的事件消息。
 */


/**
 * 消息服务初始化
 */
int msgs_init();

/**
 * 消息服务资源释放
 */
void msgs_destroy();


////////////下面是广播消息接口
/**
 * 添加需要监听的广播回调方法（接收者调用）
 * broadcast_message：广播的消息
 * broadcast_listener：事件的回调方法
 */
int add_broadcast_listener(const char* broadcast_message,void (*broadcast_listener)(const char * ,const char *));

/**
 * 发送广播消息（发布者调用）
 * broadcast_message：广播的消息
 * data:为广播的附加参数，这个根据实际业务需求而定。
 */
int send_broadcast_message(const char* broadcast_message,const char *data);




////////下面是p2p消息接口
/**
 * 设置消息处理回调（服务端调用）
 * messageListenerName：当前消息处理器的名字
 * message_listener:消息处理的回调方法,返回值是字符指针，指向消息处理的结果，注意这个指针必须是可以被free的，否则会崩溃。
 				另外因不能以char*返回，所以以long代替
 */
int add_p2p_message_listener(const char* p2p_message,long (*message_listener)(const char* data));

/**
 * 发送消息（客户端调用）
 * p2p_message：p2p消息
 * data:为p2p附加参数，这个根据实际业务需求而定。
 * 调用者必须释放response
 * timeout是毫秒数，其值小于等于0时为无限时间
 */
int send_p2p_message(const char* p2p_message,const char *data,char **response, int timeout_ms);


const char* get_lib_version();

int show_broadcast_listener();
int show_p2pmessage_listener();
int show_p2pmsg_comein_list();
int show_p2pmsg_sentout_list();

#endif


