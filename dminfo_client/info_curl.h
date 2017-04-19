#ifndef _DMINFOCURL_H_
#define _DMINFOCURL_H_

int device_login_generate_curl(CURL **curl, char* url, char* DeviceID, char* CheckSum);

#define SAFE_DESTROY_CURL(p) do{\
    if((p) != NULL)\
    {\
        destroy_curl((p));\
        (p) = NULL;\
    }\
    }while(0)
#endif

#endif
