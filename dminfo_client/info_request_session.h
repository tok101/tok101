#ifndef _DMINFOREQUEST_SESSION_H_
#define _DMINFOREQUEST_SESSION_H_

/*此模块负责https/http通讯，包括发出请求和处理响应。把一次请求及其响应定义为一个session*/




/*该结构体用于线程间参数传递*/
struct request_session {
	CURL *curl;
	HandleRespondFunc* handle_respong_func;
	HandleErrorFunc* handle_error_func;
	void* usercb;
	int sequence;
};

/*当模块处理session发生错误时，最好把错误反馈给调用者处理。调用者可以把自己的错误处理
函数封装为这种类型，在调用模块接口时传入*/
typedef int HandleErrorFunc(int errcode, void* user_cb, int sequence);

typedef int HandleRespondFunc();


int request_asyn(CURL *curl, HandleRespondFunc* handle_respond_func, HandleErrorFunc* handle_error_func， 
					void* usercb, int sequence);



#define SAFE_DESTROY_SESSION(p) do{\
    if((p) != NULL)\
    {\
        destroy_request_session((p));\
        (p) = NULL;\
    }\
    }while(0)
#endif


#endif
