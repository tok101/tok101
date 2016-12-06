#include "msgs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#define c_CharColor_Green "\033[1;32m"
#define c_CharColor_Red "\033[1;31m"
#define c_Print_Ctrl_Off "\033[0m"

#define PRINTF_GREEN(fmt, args...) fprintf(stderr, c_CharColor_Green""fmt""c_Print_Ctrl_Off, ##args)
#define PRINTF_RED(fmt, args...) fprintf(stderr, c_CharColor_Red""fmt""c_Print_Ctrl_Off, ##args)


void sighand(int signo)
{
	void *array[10];
	int size = 0;
	size = backtrace(array, 10);
	printf("Call Trace:\n");
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(0);
}

long msg1_cb(const char *data)
{
	PRINTF_GREEN("message handler msg1_cb is called,data = %s\n", data);
	char* data1 = malloc(100);
	strcpy(data1, "1111");
	return (long)data1;
}

void broadcast_listener0(const char * broadcast_message,const char *data)
{
	PRINTF_GREEN("message:%s is broadcasted, data = %s\n", broadcast_message, data);
	return ;
}
void broadcast_listener1(const char * broadcast_message,const char *data)
{
	PRINTF_GREEN("message:%s is broadcasted, data = %s\n", broadcast_message, data);
	return ;
}
void broadcast_listener2(const char * broadcast_message,const char *data)
{
	PRINTF_GREEN("message:%s is broadcasted, data = %s\n", broadcast_message, data);
	return ;
}
void broadcast_listener3(const char * broadcast_message,const char *data)
{
	PRINTF_GREEN("message:%s is broadcasted, data = %s\n", broadcast_message, data);
	return ;
}

long p2pmsg_listener0(const char *data)
{
	char* retval = "p2p message p0";
	PRINTF_GREEN("p2p message p0 has receive, data = %s, return %s\n", data, retval);
	char* data1 = malloc(100);
	strcpy(data1, retval);
	return (long)data1;
}
long p2pmsg_listener1(const char *data)
{
	char* retval = "p2p message p1";
	PRINTF_GREEN("p2p message p1 has receive, data = %s, return %s\n", data, retval);
	char* data1 = malloc(100);
	strcpy(data1, retval);
	return (long)data1;
}
long p2pmsg_listener2(const char *data)
{
	char* retval = "p2p message p2";
	PRINTF_GREEN("p2p message p2 has receive, data = %s, return %s\n", data, retval);
	char* data1 = malloc(100);
	strcpy(data1, retval);
	return (long)data1;
}
long p2pmsg_listener3(const char *data)
{
	char* retval = "p2p message p3";
	PRINTF_GREEN("p2p message p3 has receive, data = %s, return %s\n", data, retval);
	char* data1 = malloc(100);
	strcpy(data1, retval);
	return (long)data1;
}




static int test_choice(void)
{
    printf("\n\n");
    printf("0.exit\n");
    printf("1.test get_lib_version\n");
    printf("2.test add_broadcast_listener\n");
    printf("3.test send_broadcast_message\n");
    printf("4.test add_p2p_message_listener\n");
    printf("5.test send_p2p_message\n");
    printf("6.test show all handlers and tasks\n");
    printf("choose:[0-6]\n");
	char c[10] = {};
	fgets(c, 9, stdin);
	return c[0];
}

int test_lib_version()
{
	PRINTF_GREEN("version:%s\n", LIB_VERSION);
	return 0;
}
int test_add_broadcast_listener()
{
	int ret = 0;
	while(1) {
	    printf("input broadcast message[b0-b3]:\n");
		char c[10] = {};
		fgets(c, 9, stdin);
		c[strlen(c)-1] = '\0';
		if (strcmp(c, "b0") == 0)  {
			ret = add_broadcast_listener("b0", broadcast_listener0);
			break;
		}
		else if (strcmp(c, "b1") == 0) {
			ret = add_broadcast_listener("b1", broadcast_listener1);
			break;
		}
		else if (strcmp(c, "b2") == 0) {
			ret = add_broadcast_listener("b2", broadcast_listener2);
			break;
		}
		else if (strcmp(c, "b3") == 0) {
			ret = add_broadcast_listener("b3", broadcast_listener3);
			break;
		}
		else
			PRINTF_RED("invalid message\n");
	}
		
	if (ret < 0)
		PRINTF_RED("add_broadcast_listener error, test failed\n");
	else
		PRINTF_GREEN("add_broadcast_listener succeed\n");
	return 0;
}
int test_send_broadcast_message()
{
    printf("input broadcast message:\n");
	char msg[100] = {};
	scanf("%s", msg);
    printf("input broadcast data:\n");
	char data[1024] = {};
	scanf("%s", data);
	PRINTF_GREEN("testing send_broadcast_message\n");
	if (send_broadcast_message(msg, data) < 0)
		PRINTF_RED("send_broadcast_message error, test failed\n");
	return 0;
}
int test_p2p_message_listener()
{

	int ret = 0;
	while(1) {
	    printf("input p2p message[p0-p3]:\n");
		char c[10] = {};
		fgets(c, 9, stdin);
		c[strlen(c)-1] = '\0';
		if (strcmp(c, "p0") == 0)  {
			ret = add_p2p_message_listener("p0", p2pmsg_listener0);
			break;
		}
		else if (strcmp(c, "p1") == 0) {
			ret = add_p2p_message_listener("p1", p2pmsg_listener1);
			break;
		}
		else if (strcmp(c, "p2") == 0) {
			ret = add_p2p_message_listener("p2", p2pmsg_listener2);
			break;
		}
		else if (strcmp(c, "p3") == 0) {
			ret = add_p2p_message_listener("p3", p2pmsg_listener3);
			break;
		}
		else
			PRINTF_RED("invalid message\n");
	}
	
	if (ret < 0)
		PRINTF_RED("add_p2p_message_listener error, test failed\n");
	else
		PRINTF_GREEN("add_p2p_message_listener succeed\n");
	return 0;
}

int create_p2p_msg_thread(char* msg, char* data)
{
	return 0;
}

int test_send_p2p_message()
{
	printf("input broadcast message:\n");
	char msg[100] = {};
	scanf("%s", msg);
	printf("input broadcast data:\n");
	char data[1024] = {};
	scanf("%s", data);
	PRINTF_GREEN("testing send_p2p_message\n");
	char *respond = NULL;
	if (send_p2p_message(msg, data, &respond, 10000) < 0)
		PRINTF_RED("send_p2p_message_listener error, test failed\n");
	else
		PRINTF_GREEN("receive respond:%s\n", respond);
	free(respond);
	return 0;
}



int main(int argc, char* argv[])
{
	signal(SIGSEGV, sighand);
	if (msgs_init() < 0){
		printf("init failed\n");
		return -1;
	}
	while(1) {
		char c = test_choice();
		switch(c) {
			case '0':
				goto out;
			case '1':
				test_lib_version();
				break;
			case '2':
				test_add_broadcast_listener();
				break;
			case '3':
				test_send_broadcast_message();
				break;
			case '4':
				test_p2p_message_listener();
				break;
			case '5':
				test_send_p2p_message();
				break;
			case '6':
				show_broadcast_listener();
				show_p2pmessage_listener();
				show_p2pmsg_comein_list();
				show_p2pmsg_sentout_list();
				break;
			case '\n':
				break;
			default:
				PRINTF_RED("invalid choose\n");
				break;
		}
	}
out:
	msgs_destroy();
	return 0;
}

