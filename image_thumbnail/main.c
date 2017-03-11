/*
 * =============================================================================
 *
 *	Filename:  main.c
 *
 *	Description:  thumbnail test main
 *
 *	Version:  1.0.00
 *	Created:  2016/11/1 12:00
 *	Revision:  none
 *	Compiler:  gcc
 *
 *	Author:  Neil
 *	Organization:  China
 *
 * =============================================================================
 */
#include <sys/time.h>
#include "resize_img.h"
#include <stdlib.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void sighandle(int signo)
{
    void *array[10];
    int size = 0;
	fprintf(stderr, "signal segment\n");
    size = backtrace(array, 10);//maybe stuck, fix it
    backtrace_symbols_fd(array, size,STDERR_FILENO);
    exit(0);
}
extern char *optarg;
extern int optind, opterr, optopt;

void test_by_size_default(const char *src_name)
{
	unsigned char quality = 60;
	ImgReduceRequest *request_array[6] = {0};
	char *p = NULL;
	char tmp_file_name[256];
	char tmp_path[256];
	ImgReduceRequest request_middle, request_small;	
	
	if(src_name == NULL){
		printf("src_name is null\r\n");
		return;
	}

	memset(&request_middle, 0, sizeof(ImgReduceRequest));
	memset(&request_small, 0, sizeof(ImgReduceRequest));
	//printf("src_name: %s\n", src_name);
	p = strrchr(src_name, '/');
	if(p != NULL){
		memset(tmp_path, 0, sizeof(tmp_path));
		memcpy(tmp_path, src_name, p-src_name);
		p++;
		memset(tmp_file_name, 0, sizeof(tmp_file_name));
		memcpy(tmp_file_name, p, strlen(p));
		//printf("tmp_path: %s, tmp_file_name: %s\n", tmp_path, tmp_file_name);
		sprintf(request_middle.img_name, "%s/middle_%s", tmp_path, tmp_file_name);
		sprintf(request_small.img_name, "%s/small_%s", tmp_path, tmp_file_name);
	}
	else{
		sprintf(request_middle.img_name, "middle_%s", src_name);
		sprintf(request_small.img_name, "small_%s", src_name);
	}
	//printf("request_middle.img_name: %s, request_small.img_name: %s\n", request_middle.img_name, request_small.img_name);
#if 0
	ImgReduceRequest request_big;
	memset(&request_big, 0, sizeof(ImgReduceRequest));
	request_big.width = 1152;
	request_big.height = 765;
	request_big.is_accurate = 1;
	request_big.scale = 0;
	request_big.quality = quality;
	sprintf(request_big.img_name, "big_%s", src_name);
#endif
	request_middle.width = 640;
	request_middle.height = 480;
	request_middle.is_accurate = 1;
	request_middle.scale = 0;
	request_middle.quality = quality;

	request_small.width = 320;
	request_small.height = 240;
	request_small.is_accurate = 1;
	request_small.scale = 0;
	request_small.quality = quality;
	

	#if 0
	request_array[0] = &request_big;
	request_array[1] = &request_middle;
	request_array[2] = &request_small;
	#endif
	request_array[0] = &request_middle;
	request_array[1] = &request_small;

	reduce_image(src_name, &request_array[0]);
	
	return ;
}


void test_by_size(unsigned int width, unsigned int height, unsigned char quality, const char *src_name, const char *dst_name)
{
	ImgReduceRequest *request_array[2] = {0};
	ImgReduceRequest request_size;	
	
	if(src_name == NULL || dst_name == NULL){
		printf("src_name or dst_name is null\r\n");
		return;
	}

	memset(&request_size, 0, sizeof(ImgReduceRequest));
	request_size.width = width;
	request_size.height = height;
	request_size.is_accurate = 1;
	request_size.scale = 0;
	request_size.quality = quality;
	//sprintf(request_size.img_name, "size_%s", src_name);
	sprintf(request_size.img_name, "%s", dst_name);

	request_array[0] = &request_size;

	reduce_image(src_name, &request_array[0]);
	
	return ;
}


void test_by_scale(const char *src_name)
{
	ImgReduceRequest request_array[4];
	ImgReduceRequest *p[5] = {0};

	request_array[0].scale = 4;
	request_array[0].quality = 80;
	strncpy(request_array[0].img_name, "4_1.jpg", sizeof(request_array[0].img_name));

	request_array[1].scale = 6;
	request_array[1].quality = 80;
	request_array[1].is_accurate = 1;
	strncpy(request_array[1].img_name, "6_1.jpg", sizeof(request_array[1].img_name));

	request_array[2].scale = 8;
	request_array[2].quality = 80;
	strncpy(request_array[2].img_name, "8_1.jpg", sizeof(request_array[2].img_name));

	request_array[3].scale = 10;
	request_array[3].quality = 80;
	request_array[3].is_accurate = 1;
	strncpy(request_array[3].img_name, "10_1.jpg", sizeof(request_array[3].img_name));

	uint8_t i = 0;
	for(i = 0; i < 4; ++i)
	{
		p[i] = &request_array[i];
	}

	reduce_image(src_name, &p[0]);
	return ;
}


void usage()
{
	printf("Usage: thumbnail_test [-f file] [-w width] [-h height] [-q quality]  [-o file] [-help] [astv]\n");
	printf("Option:\n");
	printf("\t -f file\t\t: the source file\n");
	printf("\t -a \t\t\t: creat middle picture and thumbnail by default\n");
	printf("\t -w width \t\t: width of the thumbnail\n");		
	printf("\t -h height \t\t: height of the thumbnail\n");
	printf("\t -q quality \t\t: quality of the thumbnail,default 60\n");
	printf("\t -s \t\t\t: creat thumbnail for the file by size\n");
	printf("\t -o file\t\t: the output filename\n");
	printf("\t -t \t\t\t: show time gap\n");	
	printf("\t -v \t\t\t: show version number\n");
	printf("\t -help \t\t\t: show help list\n");
	return ;
}
#if 0
int main(int argc, char **argv)
{
   int result = 1;

   if (argc == 3)
   {
	  png_image image;

	  /* Only the image structure version number needs to be set. */
	  memset(&image, 0, sizeof image);
	  image.version = PNG_IMAGE_VERSION;

	  if (png_image_begin_read_from_file(&image, argv[1]))
	  {
		 png_bytep buffer;

		 /* Change this to try different formats!  If you set a colormap format
		  * then you must also supply a colormap below.
		  */
		 image.format = PNG_FORMAT_RGBA;

		 buffer = malloc(PNG_IMAGE_SIZE(image));

		 if (buffer != NULL)
		 {
			if (png_image_finish_read(&image, NULL/*background*/, buffer,
			   0/*row_stride*/, NULL/*colormap for PNG_FORMAT_FLAG_COLORMAP */))
			{
				image.height = image.width = 150;
			   if (png_image_write_to_file(&image, argv[2],
				  0/*convert_to_8bit*/, buffer, 0/*row_stride*/,
				  NULL/*colormap*/))
				  result = 0;

			   else
				  fprintf(stderr, "pngtopng: write %s: %s\n", argv[2],
					  image.message);

			   free(buffer);
			}

			else
			{
			   fprintf(stderr, "pngtopng: read %s: %s\n", argv[1],
				   image.message);

			   /* This is the only place where a 'free' is required; libpng does
				* the cleanup on error and success, but in this case we couldn't
				* complete the read because of running out of memory.
				*/
			   png_image_free(&image);
			}
		 }

		 else
			fprintf(stderr, "pngtopng: out of memory: %lu bytes\n",
			   (unsigned long)PNG_IMAGE_SIZE(image));
	  }

	  else
		 /* Failed to read the first argument: */
		 fprintf(stderr, "pngtopng: %s: %s\n", argv[1], image.message);
   }

   else
	  /* Wrong number of arguments */
	  fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");

   return result;
}
#endif
#if 1


int main(int argc, char **argv)
{
	printf("flag 4\n");
	signal(SIGSEGV, sighandle);
	int ret = 0;
	int ch;
	int test_auto_flag = 0;
	int version_flag = 0;
	int time_show_flag = 0;
	int test_size_flag = 0;
	char *src_file_name = NULL;
	char *dst_file_name = NULL;
	unsigned int width = 0, height = 0;
	unsigned char quality = 30;
	struct timeval start_tnow, end_tnow;
	unsigned int tv_sec_t;
	unsigned int tv_usec_t;

	if(argc <= 1){
		usage();
		return -1;
	}

	while((ch = getopt(argc,argv,"f:aw:h:q:so:tv"))!= -1)
	{
		switch(ch)
		{
			case 'f':
				src_file_name = strdup(optarg);
				break;
			case 'a':
				test_auto_flag = 1;			
				break;
			case 'w':
				width = atoi(optarg);
				break;
			case 'h':
				height = atoi(optarg);
				break;
			case 'q':
				quality = atoi(optarg);
				break;
			case 's':
				test_size_flag = 1;
				break;
			case 'o':
				dst_file_name = strdup(optarg);
				break;
			case 't':
				time_show_flag = 1;
				break;
			case 'v':
				version_flag = 1;
				break;
			default:
				//printf("invalid argument\n");
				break;
		}
	}

	gettimeofday(&start_tnow, NULL);
	//printf("start time: %08d.%06d\n", (unsigned int)start_tnow.tv_sec, (unsigned int)start_tnow.tv_usec);	
	if(test_auto_flag == 1 && src_file_name != NULL){		
		printf("creat thunbnail by default\n");
		test_by_size_default(src_file_name);
	}
	else if(test_size_flag == 1 && width != 0 && height != 0 && quality != 0 && src_file_name != NULL && dst_file_name){
		printf("creat thunbnail by size\n");
		test_by_size(width, height, quality, src_file_name, dst_file_name);
	}
	else if(version_flag){
		printf("Version: %s/%s\n", PKG_NAME, PKG_VERSION);
	}	
	else{
		usage();
	}
	gettimeofday(&end_tnow, NULL);
	//printf("end time: %08d.%06d\n", (unsigned int)end_tnow.tv_sec, (unsigned int)end_tnow.tv_usec);

	if(time_show_flag){
		if((unsigned int)end_tnow.tv_usec > (unsigned int)start_tnow.tv_usec){
			tv_sec_t = (unsigned int)end_tnow.tv_sec - (unsigned int)start_tnow.tv_sec;
			tv_usec_t = (unsigned int)end_tnow.tv_usec - (unsigned int)start_tnow.tv_usec;
		}
		else{
			tv_sec_t = (unsigned int)end_tnow.tv_sec - (unsigned int)start_tnow.tv_sec - 1;
			tv_usec_t = (unsigned int)end_tnow.tv_usec - (unsigned int)start_tnow.tv_usec + 1000000;
		}
		printf("takes time: %08d.%06d\n", tv_sec_t, tv_usec_t);
	}
		
	if(src_file_name != NULL){
		free(src_file_name);
	}

	if(dst_file_name != NULL){
		free(dst_file_name);
	}

	return 0;
}
#endif
