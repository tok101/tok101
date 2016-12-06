/**
 * ��Ϣ����
 */
#ifndef _MSGS_H_
#define _MSGS_H_


#define LIB_VERSION "1.0.0"
/**
 * �ػ���Ϣ�¼�
 */
#define BROADCAST_MESSAGE_SHUTDOWN "shutdown"
/**
 * todo
 * ������Լ�����ӿ��Լ������¼���Ϣ��
 */


/**
 * ��Ϣ�����ʼ��
 */
int msgs_init();

/**
 * ��Ϣ������Դ�ͷ�
 */
void msgs_destroy();


////////////�����ǹ㲥��Ϣ�ӿ�
/**
 * �����Ҫ�����Ĺ㲥�ص������������ߵ��ã�
 * broadcast_message���㲥����Ϣ
 * broadcast_listener���¼��Ļص�����
 */
int add_broadcast_listener(const char* broadcast_message,void (*broadcast_listener)(const char * ,const char *));

/**
 * ���͹㲥��Ϣ�������ߵ��ã�
 * broadcast_message���㲥����Ϣ
 * data:Ϊ�㲥�ĸ��Ӳ������������ʵ��ҵ�����������
 */
int send_broadcast_message(const char* broadcast_message,const char *data);




////////������p2p��Ϣ�ӿ�
/**
 * ������Ϣ����ص�������˵��ã�
 * messageListenerName����ǰ��Ϣ������������
 * message_listener:��Ϣ����Ļص�����,����ֵ���ַ�ָ�룬ָ����Ϣ����Ľ����ע�����ָ������ǿ��Ա�free�ģ�����������
 				����������char*���أ�������long����
 */
int add_p2p_message_listener(const char* p2p_message,long (*message_listener)(const char* data));

/**
 * ������Ϣ���ͻ��˵��ã�
 * p2p_message��p2p��Ϣ
 * data:Ϊp2p���Ӳ������������ʵ��ҵ�����������
 * �����߱����ͷ�response
 * timeout�Ǻ���������ֵС�ڵ���0ʱΪ����ʱ��
 */
int send_p2p_message(const char* p2p_message,const char *data,char **response, int timeout_ms);


const char* get_lib_version();

int show_broadcast_listener();
int show_p2pmessage_listener();
int show_p2pmsg_comein_list();
int show_p2pmsg_sentout_list();

#endif


