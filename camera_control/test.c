#include <stdio.h>
#include "camera_control.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

int write_to_file(const char *path, unsigned char* buf, int write_len)
{
	FILE *fd = NULL;
	if (NULL == (fd = fopen(path, "a+"))) {
		fprintf(stderr, "open %s to write error, error:%s\n", path, strerror(errno) );
		return -1;
	}
	if (fwrite(buf, write_len, 1, fd) != 1){
		fprintf(stderr, "fwrite error, error:%s\n", strerror(errno) );
		fclose(fd);
		return -1;
	}
	fflush(fd);
	fclose(fd);
	return 0;
}

char* create_filename(const char* path)
{
	char* tmp = NULL;
	if ( (tmp = strrchr(path, '/')) == NULL) {
		fprintf(stderr, "path not good\n");
		return NULL;
	}
	return tmp+1;
}

#define BUFFER_SIZE 512*1024
int handle_file_func(const char* path, GPParams* gp_params, void* args)
{
	int ret = 0;
	unsigned char* buf = NULL;
	const char* outfile = create_filename(path);
	unlink(outfile);
	if (!outfile) {
		ret = -1;
		goto out;
	}
	if ((buf = calloc(1, BUFFER_SIZE)) == NULL) {
		fprintf(stderr, "malloc failed\n");
		ret = -1;
		goto out;
	}
	int offset = 0;
	int size = BUFFER_SIZE;
		
	while(1) {
		ret = read_camera_file(path, gp_params, offset, size, buf);
		offset += ret;
		write_to_file(outfile, buf, ret);
		if (ret < size)
			break;	
	}
	free(buf);
out:
	return ret;
}

int main()
{
	get_all_files(handle_file_func, NULL);
	return 0;
}


