#ifndef _CAMERA_CONTROL_H
#define _CAMERA_CONTROL_H

#include "gp-params.h"

typedef int (*PHandleFunc)(const char*, GPParams*, void*);


int read_camera_file(const char* path, GPParams* gp_params, uint64_t offset, 
		uint64_t size, unsigned char*	data);

int get_all_files(PHandleFunc func, void* func_args);

#endif
