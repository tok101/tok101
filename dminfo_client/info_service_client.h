#ifndef _INFOSERVICECLIENT_H_
#define _INFOSERVICECLIENT_H_

#define INFOSERVICE_EKNOW 1
#define INFOSERVICE_EINVAL 2
#define INFOSERVICE_ENOMEM 3
#define INFOSERVICE_EOVERFLOW 4
#define INFOSERVICE_EUNINIT 5

#define SERVERADDRSIZE 256
struct info_service {
	char addr[SERVERADDRSIZE];
	int init_flag;
	int seq;//sequence number for request
	pthread_mutex_t mutex;
};

int info_service_init(struct info_service* is);
typedef login_callback(unsigned int sequence, int err_code, char* err_msg, char* token, unsigned long long expiretime);
int device_login(char* DeviceID, char* CheckSum, login_callback* cb, int* sequence);


#endif
