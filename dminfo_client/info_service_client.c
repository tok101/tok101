#include "info_service_client.h"

struct info_service g_info_service = {
	.mutex = PTHREAD_MUTEX_INITIALIZER;
};
#define SERVICELOCK pthread_mutex_lock(&g_info_service.mutex)
#define SERVICEUNLOCK pthread_mutex_unlock(&g_info_service.mutex)

int info_service_init(char* ip)
{
	int ret = 0;
	SERVICELOCK;
	if (g_info_service.init_flag > 0) {
		LOG_W("info service has been init, break\n");
		ret = 0;
		goto out;
	}
	if (!ip ) {
		LOG_E("some arguments are null, ip(%p)\n", ip);
		ret = -INFOSERVICE_EINVAL;
		goto out;
	}
	if ( (ret = snprintf(g_info_service.addr, SERVERADDRSIZE, "%s", ip)) >= SERVERADDRSIZE) {
		LOG_E("ip is too long, ip len(%d), mem space(%d), ip(%s)\n", ret, SERVERADDRSIZE, ip);
		ret = -INFOSERVICE_EOVERFLOW;
		goto out;
	}
	g_info_service.init_flag = 1;
out:
	SERVICEUNLOCK;
	return ret;	
}
static int generate_sequence()
{
	return g_info_service.seq++;
}

static int send_login_request(char* DeviceID, char* CheckSum)
{
	int len = 0;
	char* buffer = generate_login_request(&len);
	int fd = 0;
	fd = get_socket_fd();
	
	send_out_request(fd, buffer, len);
	return 0;
}

static wait_for_login_resp(void *arg)
{
	
	return 0;
}

static int wait_for_login_resp_async(int fd, login_callback* cb, int sequence)
{ 
	int ret;    
	pthread_t thread1;
	memset(&thread1, 0, sizeof(thread1)); 
	if((ret = pthread_create(&thread1, NULL, , NULL)) != 0)
	return 0;
}

int device_login(char* DeviceID, char* CheckSum, login_callback* cb, int* sequence)
{
	int ret = 0;
	if (!DeviceID || !CheckSum || !cb) {
		LOG_E("some arguments are null, deviceid(%p),checksum(%p),callback(%p), device_login failed.\n", 
				DeviceID, CheckSum, cb);
		return -INFOSERVICE_EINVAL;
	}
	SERVICELOCK;
	if ( g_info_service.init_flag < 0) {
		ret = -INFOSERVICE_EUNINIT;
		goto out;
	}
	*sequence = generate_sequence();
	int fd = send_login_request(DeviceID, CheckSum);
	wait_for_login_resp_async(fd, cb, *sequence);	
out:
	SERVICEUNLOCK;
	return ret;
}

