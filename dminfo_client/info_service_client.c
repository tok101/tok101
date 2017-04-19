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


int handle_device_login_session_error( int errcode, struct request_session* session)
{
	login_callback* cb = session->usercb;
	cb(session->sequence, errcode, -1, "", NULL, 0);
	return 0;
}


int device_login(char* DeviceID, char* CheckSum, login_callback* usercb, int* sequence)
{
	int ret = 0;
	if (!DeviceID || !CheckSum || !usercb) {
		LOG_E("some arguments are null, deviceid(%p),checksum(%p),callback(%p), device_login failed.\n", 
				DeviceID, CheckSum, usercb);
		return -INFOSERVICE_EINVAL;
	}
	SERVICELOCK;
	if ( g_info_service.init_flag < 0) {
		ret = -INFOSERVICE_EUNINIT;
		goto out;
	}
	*sequence = generate_sequence();
	CURL *curl = NULL;
	if ( (ret = device_login_generate_curl(&curl, g_info_service.addr, DeviceID, CheckSum)) < 0)
		goto out;
	struct request_session* session = NULL;
	if ( (ret = create_request_session(&session, curl, &handle_device_login_respond, 
										&handle_device_login_session_error, usercb, *sequence)) < 0) {
		SAFE_DESTROY_CURL(curl);
		goto out;
	}
	if ( (ret = request_asyn(session)) < 0) {
		SAFE_DESTROY_SESSION(session);
		goto out;
	}
out:
	SERVICEUNLOCK;
	return ret;
}

int device_login_sync(char* DeviceID, char* CheckSum, login_callback* usercb, int* sequence)
{
	device_login_async();
	return 0;
}

