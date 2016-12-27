#ifndef _INTERNEL_H
#define _INTERNEL_H

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define LOG_E(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_I(fmt, args...) fprintf(stderr, fmt, ##args); 
#define LOG_W(fmt, args...) fprintf(stderr, fmt, ##args); 
#else
#define LOG_E(fmt, args...)
#define LOG_I(fmt, args...)
#define LOG_W(fmt, args...)
#endif

#define safe_free(p) do{\
    if((p) != NULL)\
    {\
        free((p));\
        (p) = NULL;\
    }\
    }while(0)

	
	//if result < 0, goto label
#define CK00(result, label)	\
	do{ \
	 if ((result) < 0) \
	  goto label; \
	}while(0)
		
	//if result == NULL0, goto label
#define CK01(result, label)	\
	do{ \
	 if ((result) == NULL) \
	  goto label; \
	}while(0)
	
	enum EntryType {
		ET_FILE = 0,
		ET_FOLDER,
	};

#endif
