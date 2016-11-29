#include "media_info.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int test_video_media_info(const char *path)
{
	VIDEO_MEDIA_INFO *pVideoMediaIn = NULL;
	if (get_video_media_info(path, &pVideoMediaIn) == 0) {
		printf("weight:%d height:%d bit_rate:%d codec_name:%s duration:%"PRId64"\n", 
			pVideoMediaIn->width, pVideoMediaIn->height, pVideoMediaIn->bit_rate,
			pVideoMediaIn->encode, pVideoMediaIn->duration);
		free_video_media_info(&pVideoMediaIn);
	}
	else {
		printf("can't get video media info\n");
	}
	return 0;
}

int test_audio_media_info(const char *path)
{

	AUDIO_MEDIA_INFO *pAudioMediaIn = NULL;
	if (get_audio_media_info(path, &pAudioMediaIn) == 0) {
		printf("title=%s\nartist=%s\nalbum=%s\ncodec=%s\nbit_rate=%d\nduration:%"PRId64"\n", 
			pAudioMediaIn->name, pAudioMediaIn->artist, pAudioMediaIn->album, 
			pAudioMediaIn->encode, pAudioMediaIn->bit_rate, pAudioMediaIn->duration);
		free_audio_media_info(&pAudioMediaIn);
	}
	else {
		printf("can't get audio media info\n");
	}
	return 0;
}

int test_image_media_info(const char *path)
{

	IMAGE_MEDIA_INFO *pImageMediaIn = NULL;
	if (get_image_media_info(path, &pImageMediaIn) == 0) {
		printf("ApertureValue = %s\n", pImageMediaIn->ApertureValue);
		printf("AMaxApertureValue = %s\n",pImageMediaIn->MaxApertureValue);
		printf("CameraType = %s\n",pImageMediaIn->CameraType);
		printf("ExposureBiasValue = %s\n",pImageMediaIn->ExposureBiasValue);
		printf("ExposureTime = %s\n",pImageMediaIn->ExposureTime);
		printf("Flash = %s\n",pImageMediaIn->Flash);
		printf("FocalLength = %s\n",pImageMediaIn->FocalLength);
		printf("MeteringMode = %s\n",pImageMediaIn->MeteringMode);
		printf("DateTime = %s\n",pImageMediaIn->DateTime);
		printf("Make = %s\n",pImageMediaIn->Make);
		printf("ISOSpeedRatings = %s\n",pImageMediaIn->ISOSpeedRatings);
		printf("XResolution = %d\n",pImageMediaIn->XResolution);
		printf("YResolution = %d\n",pImageMediaIn->YResolution);
		free_image_media_info(&pImageMediaIn);
	} else {
		printf("can't get image media info\n");
	}
	return 0;
}

static void usage(const char* argv[])
{
    printf("Usage:%s -t type -f file \n", argv[0]);
	printf("Option:\n");
    printf("%-5s%-15s:%s\n", "", "-t type", "set file type: v/a/i , repensent for video, audio, image");
    printf("%-5s%-15s:%s\n", "", "-f file", "set file path");
    printf("%-5s%-15s:%s\n", "", "--version", "show versioin");
    printf("%-5s%-15s:%s\n", "", "-h help", "show this help");
    printf("example: %s -t a -f 1.mp3\n", argv[0]);
    printf("\n");
}

static char g_arg_type = 0;
static const char* g_arg_path = NULL;
static int g_showversion = 0;

static int process_cmdline(int argc,const char  *argv[])
{
    int count = argc;
    int i;
    for(i=1;i<count;) {
        if(!strcmp("-t", argv[i]) && i+1 < count) {
        	g_arg_type = argv[i+1][0];
            i+=2;
            continue;
        }  
        else if(!strcmp("-f", argv[i]) && i+1 < count) {
        	g_arg_path = argv[i+1];
            i+=2;
            continue;
        }  
        else if(!strcmp("--version", argv[i])) {
			g_showversion = 1;
            i+=1;
            continue;
        } 
        else
        {
            usage(argv);
			return -1;

        }
    }
    return 0;
}

int main(int argc, const char* argv[])
{
	if (process_cmdline(argc,argv) < 0)
		return -1;
	if (g_showversion == 1)  {
		printf("%s\n", get_lib_version());
		return 0;
	}
	if (g_arg_type == 0 || !g_arg_path) {
		usage(argv);
		return -1;		
	}
	switch(g_arg_type) {
		case 'a':
			test_audio_media_info(g_arg_path);
			break;
		case 'v':
			test_video_media_info(g_arg_path);
			break;
		case 'i':
			test_image_media_info(g_arg_path);
			break;	
		default:
			usage(argv);
			break;			
	}
	return 0;
}
