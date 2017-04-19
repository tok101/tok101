

static void* request(void *args)
{
	pthread_detach(pthread_self());
	struct request_session* session = (struct request_session*)args;
	
	int res = curl_easy_perform(session->curl);
	if (res < 0) {
		LOG_E("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		session->handle_error_func(-INFOSERVICE_EUNKNOW, session->usercb, session->sequence);
		goto out;
	}
	safe_free(session);
out:
	return NULL;
}

int create_request_session(struct request_session** session, CURL *curl, 
	HandleRespondFunc* handle_respond_func, HandleErrorFunc* handle_error_func, void* usercb, int sequence)
{
	struct request_session* session_tmp = malloc(sizeof(struct request_session) );
	if (!session_tmp) {
		LOG_E("malloc request arguments session_tmp failed\n");
		return -INFOSERVICE_ENOMEM;
	}
	session_tmp->curl = curl;
	session_tmp->handle_respond_func = handle_respond_func;
	session_tmp->handle_error_func = handle_error_func;
	session_tmp->usercb = usercb;
	session_tmp->sequence = sequence;

	*session = session_tmp;
	return 0;
}

int destroy_request_session(struct request_session* session)
{
	if (!session)
		return 0;
	SAFE_DESTROY_CURL(session->curl);
	safe_free(session);
	return 0;
}

int request_asyn(struct request_session* session)
{
	pthread_t thread1;
	memset(&thread1, 0, sizeof(thread1));
	if(pthread_create(&thread1, NULL, request, session) != 0) {
		LOG_E("pthread_create failed, can not create request thread, error(%s)\n", strerror(errno));
		return -INFOSERVICE_EUNKNOW;
	}
	return 0;
}

