
#define LOGIN_INTERFACE "dm-device-login"
#define COOKIE_FILE "/Users/zhu/CProjects/curlposttest.cookie"

static int vasprintf (char **str, const char *fmt, va_list args)
{
  int size = 0;
  va_list tmpa;
  va_copy(tmpa, args);
  size = vsnprintf(NULL, size, fmt, tmpa);
  va_end(tmpa);
  if (size < 0) { return -1; }
  *str = (char *) malloc(size + 1);
  if (NULL == *str) { return -1; }
  size = vsprintf(*str, fmt, args);
  return size;
}

static int my_asprintf (char **str, const char *fmt, ...)
{
  int size = 0;
  va_list args;
  va_start(args, fmt);
  size = vasprintf(str, fmt, args);
  va_end(args);
  return size;
}
/*
size_t receive_cb(void *buffer, size_t size, size_t nmemb, void *userp) {
	printf("%s", buffer);
	return size;
}  */

void destroy_curl(CURL *curl)
{
	if (!curl)
		return;
	return ;
}

int device_login_generate_curl(CURL **curl, char* url, char* DeviceID, char* CheckSum)
{
	int ret = 0;
	CURL *curl_tmp = NULL; 
	
	char* posturl = NULL;
	char* postfield = NULL;
	if (my_asprintf(&posturl, "%s/%s", url, LOGIN_INTERFACE) < 0) {
		LOG_E("device_login_generate_curl malloc failed\n");
		ret = -INFOSERVICE_ENOMEM;
		goto out;
	}
	if (my_asprintf(&postfield, "DeviceID=%s&CheckSum=%s", DeviceID, CheckSum) < 0) {
		LOG_E("device_login_generate_curl malloc failed\n");
		ret = -INFOSERVICE_ENOMEM;
		goto out;
	}
    curl_tmp = curl_easy_init();
	if (!curl_tmp) {
		LOG_E("curl_easy_init failed\n");
		ret = -INFOSERVICE_EUNKNOW;
		goto out;
	}
    curl_easy_setopt(curl_tmp, CURLOPT_URL, posturl);  
    curl_easy_setopt(curl_tmp, CURLOPT_POSTFIELDS, POSTFIELDS);  
    //curl_easy_setopt(curl_tmp, CURLOPT_WRITEFUNCTION, receive_cb);  
    //curl_easy_setopt(curl_tmp, CURLOPT_WRITEDATA, fptr);  
    curl_easy_setopt(curl_tmp, CURLOPT_POST, 1);  
    curl_easy_setopt(curl_tmp, CURLOPT_VERBOSE, 1);  
    //curl_easy_setopt(curl_tmp, CURLOPT_HEADER, 1);  
    //curl_easy_setopt(curl_tmp, CURLOPT_FOLLOWLOCATION, 1);  
    //curl_easy_setopt(curl_tmp, CURLOPT_COOKIEFILE, COOKIE_FILE);  
    //res = curl_easy_perform(curl_tmp);
out:
	safe_free(posturl);
	safe_free(postfield);
	*curl = curl_tmp;
	return ret;
}

