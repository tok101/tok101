#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include "snapshot.h"

extern char *optarg;
extern int optind, opterr, optopt;

void usage()
{
	printf("Usage: thumbnail_test [-f file] [-p percent] [-m second] [-o file] [-help] [astv]\n");
	printf("Option:\n");
	printf("\t -y type\t\t: the source file is music or video,\"video\":video ;\"music\":music\n");
	printf("\t -f file\t\t: the source file\n");
	printf("\t -o file\t\t: the output filename\n");
	printf("\t -p percent\t\t: seek percent of source file\n");
	printf("\t -m second\t\t: seek second of source file\n");
	printf("\t -t \t\t\t: show time gap\n");	
	printf("\t -v \t\t\t: show version number\n");
	printf("\t -help \t\t\t: show help list\n");
	return ;
}




int main(int argc, char **argv)
{
	int ret = 0;	
	int ch;
	int version_flag = 0;
	int time_show_flag = 0;
	int src_type = 0;//0:video ;1:music
	
	struct timeval start_tnow, end_tnow;
	unsigned int tv_sec_t;
	unsigned int tv_usec_t;

	if(argc <= 1){
		usage();
		return -1;
	}
	
	OptionDmContext dm_context;
	memset(&dm_context, 0, sizeof(OptionDmContext));
	
	//default
	
	
	while((ch = getopt(argc,argv,"f:o:p:m:w:h:tvy:"))!= -1)
	{
		switch(ch)
		{
			case 'y':
				if(NULL != strstr(optarg, "music")){
					printf("is music\n");
					src_type = 1;
				}
				else{
					printf("is video\n");
					src_type = 0;
				}
				break;
			case 'f':
				dm_context.input_file_name = optarg;
				break;
			case 'o':
				dm_context.output_file_name = optarg;
				break;
			//case 'a':
			//	test_auto_flag = 1;			
			//	break;
			//case 's':
			//	test_size_flag = 1;
			//	break;
			case 'p':
				memset(dm_context.seek_proportion, 0, sizeof(dm_context.seek_proportion));
				strcpy(dm_context.seek_proportion, optarg);
				break;
			case 'm':
				memset(dm_context.seek_time, 0, sizeof(dm_context.seek_time));
				strcpy(dm_context.seek_time, optarg);
				break;
			case 'w':
				//strcpy(dm_context.width, optarg);
				break;
			case 'h':
				//strcpy(dm_context.height, optarg);
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

	if(src_type){
		
	}
	else{
		if(!strlen(dm_context.seek_proportion))
			sprintf(dm_context.seek_proportion, "20");
		//sprintf(dm_context.nb_pic, "0.001");
	}
	//sprintf(dm_context.force_format, "image2");
	//sprintf(dm_context.log_level, "1");

	gettimeofday(&start_tnow, NULL);
	if(dm_context.input_file_name && dm_context.output_file_name)
	{
	    ret = media_snapshot(&dm_context);
		if(ret < 0){
			printf("media_snapshot fail!!!\n");	
			return -1;
		}
		else{
			printf("media_snapshot success!!!\n");
		}
	}
	else if(version_flag){
		printf("Version: %s/%s\n", PKG_NAME, PKG_VERSION);
	}
	else{
		usage();
	}
	gettimeofday(&end_tnow, NULL);
	
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
	
	return 0;
}

