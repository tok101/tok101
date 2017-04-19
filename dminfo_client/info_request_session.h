#ifndef _DMINFOREQUEST_SESSION_H_
#define _DMINFOREQUEST_SESSION_H_

/*��ģ�鸺��https/httpͨѶ��������������ʹ�����Ӧ����һ����������Ӧ����Ϊһ��session*/




/*�ýṹ�������̼߳��������*/
struct request_session {
	CURL *curl;
	HandleRespondFunc* handle_respong_func;
	HandleErrorFunc* handle_error_func;
	void* usercb;
	int sequence;
};

/*��ģ�鴦��session��������ʱ����ðѴ������������ߴ��������߿��԰��Լ��Ĵ�����
������װΪ�������ͣ��ڵ���ģ��ӿ�ʱ����*/
typedef int HandleErrorFunc(int errcode, void* user_cb, int sequence);

typedef int HandleRespondFunc();


int request_asyn(CURL *curl, HandleRespondFunc* handle_respond_func, HandleErrorFunc* handle_error_func�� 
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
