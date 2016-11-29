﻿#include "libavformat/avformat.h"
#include "media_info.h"
#include <libexif/exif-data.h>

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
    
	#if 0
//request:	pp_dest must be NULL
//if `max_size` is less than the len of `p_src`+1, just malloc `max_size`.in case p_src is not a string pointer and may malloc a large size of mem.
static int anstrcpy(char** pp_dest, const char* p_src, int max_size)
{
	if(*pp_dest != NULL) {
		LOG_E("anstrcpy error: target pointer has been alloc\n");
		return -1;
	}
	if (p_src == NULL) {
		LOG_E("src is NULL\n");
		return 0;		
	}
	int size = strlen(p_src) + 1;
	size = size>max_size?max_size:size;
	*pp_dest = malloc(size);
	if (*pp_dest == NULL) {
		LOG_E("malloc failed\n");
		return -1;
	}
	snprintf(*pp_dest, size, "%s", p_src);
	return 0;
}
#endif

void ffmpeg_release_info(AVFormatContext **ppFormatCtx, AVCodecContext **ppCodecCtx)
{
	if (*ppCodecCtx) {
		avcodec_close(*ppCodecCtx);
		*ppCodecCtx = NULL;
	}
	if (*ppFormatCtx) {
		avformat_close_input(ppFormatCtx);	
		*ppFormatCtx = NULL;
	}
	return ;
}

//obtain the first stream of 'type' from a file 
int ffmpeg_obtain_info(const char* path, AVFormatContext **ppFormatCtx, AVCodecContext **ppCodecCtx, AVCodec **ppCodec, int type)
{
	int i=0, stream=0;
	av_register_all();
	if( avformat_open_input(ppFormatCtx, path, NULL, NULL) != 0 ) {
		LOG_E("avformat_open_input failed\n");
		return -1;
	}
	if( avformat_find_stream_info(*ppFormatCtx, NULL ) < 0 ){
		LOG_E("avformat_find_stream_info failed\n");
		return -1;
	}
	//av_dump_format(*ppFormatCtx, -1, path, 0);
	stream = -1;
	for( i = 0; i < (*ppFormatCtx)->nb_streams; i++ )
		if( (*ppFormatCtx)->streams[i]->codec->codec_type == type) {
			stream = i;
			break;
		}
 
	if( stream == -1 ) {
		LOG_E("no stream found\n");
		return -1;
	} 
	*ppCodecCtx = (*ppFormatCtx)->streams[stream]->codec;
	*ppCodec = avcodec_find_decoder((*ppCodecCtx)->codec_id);
	if (*ppCodec == NULL) {
		LOG_E("avcodec_find_decoder return NULL\n");
		return -1;
	}
	return 0;
}


static char* fmtctx_get_metadata(AVDictionary *m, char* tagname)
{
	if (!tagname)
		return NULL;
	AVDictionaryEntry *tag = NULL;		
	tag = av_dict_get(m, tagname, NULL, 0);
	if(tag)
	    return tag->value;
	else
		return NULL;
}


int get_audio_media_info(const char *path,AUDIO_MEDIA_INFO** ppAudioMediaInfo)
{	
	AVFormatContext *pFormatCtx = NULL;
	int 			ret=0;
	AVCodecContext	*pCodecCtx = NULL;
	AVCodec 		*pCodec = NULL;

	if (ffmpeg_obtain_info(path, &pFormatCtx, &pCodecCtx, &pCodec, AVMEDIA_TYPE_AUDIO) < 0) {
		ret = -1;
		goto err;
	}

	char* name = fmtctx_get_metadata(pFormatCtx->metadata, "title");
	char* artist = fmtctx_get_metadata(pFormatCtx->metadata, "artist");
	char* album = fmtctx_get_metadata(pFormatCtx->metadata, "album");
	const char* encode = pCodec->name;
	int bit_rate = pFormatCtx->bit_rate;
	int64_t duration = pFormatCtx->duration;

	//fill media info
	*ppAudioMediaInfo = calloc(1, sizeof(AUDIO_MEDIA_INFO));
	if (!*ppAudioMediaInfo) {
		LOG_E("malloc failed\n");
		ret = -1;
		goto err;
	}
	const char* tmp = NULL;
	tmp = name?name:"unknow";
	snprintf((*ppAudioMediaInfo)->name, sizeof((*ppAudioMediaInfo)->name), "%s", tmp);
	tmp = artist?artist:"unknow";
	snprintf((*ppAudioMediaInfo)->artist, sizeof((*ppAudioMediaInfo)->artist), "%s", tmp);
	tmp = album?album:"unknow";
	snprintf((*ppAudioMediaInfo)->album, sizeof((*ppAudioMediaInfo)->album), "%s", tmp);
	tmp = encode?encode:"unknow";
	snprintf((*ppAudioMediaInfo)->encode, sizeof((*ppAudioMediaInfo)->encode), "%s", tmp);
	
	(*ppAudioMediaInfo)->bit_rate = bit_rate;
	(*ppAudioMediaInfo)->duration = duration;
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	return 0;
err:
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	free_audio_media_info(ppAudioMediaInfo);
	return ret;
}

int get_video_media_info(const char *path,VIDEO_MEDIA_INFO** ppVideoMediaInfo)
{	
	AVFormatContext *pFormatCtx = NULL;
	int             ret=0;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;

	if (ffmpeg_obtain_info(path, &pFormatCtx, &pCodecCtx, &pCodec, AVMEDIA_TYPE_VIDEO) < 0) {
		ret = -1;
		goto err;
	}

	//fill media info
	*ppVideoMediaInfo = calloc(1, sizeof(VIDEO_MEDIA_INFO));
	if (!*ppVideoMediaInfo) {
		LOG_E("malloc failed\n");
		ret = -1;
		goto err;
	}
	(*ppVideoMediaInfo)->width = pCodecCtx->width;
	(*ppVideoMediaInfo)->height = pCodecCtx->height;
	(*ppVideoMediaInfo)->bit_rate = pFormatCtx->bit_rate;
	const char* tmp;
	tmp = pCodec->name?pCodec->name:"unknow";
	snprintf((*ppVideoMediaInfo)->encode, sizeof((*ppVideoMediaInfo)->encode), "%s", tmp);
	
	(*ppVideoMediaInfo)->duration = pFormatCtx->duration;
	
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	return 0;
err:
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	free_video_media_info(ppVideoMediaInfo);
	return ret;
}

//judge if tags match,if so copy and return   
#define JUDGE_AND_FILL_EXIF(entry, ptr, tag, tag2) { \
	if (strcmp(tag, #tag2) == 0) { \
		exif_entry_get_value((entry), (ptr)->tag2, sizeof((ptr)->tag2)); \
		return ; \
	} \
}

static void content_foreach_func(ExifEntry *entry, void *callback_data)
{
  #if 0
  char buf[2000];
  printf("    Entry %p: %s (%s)\n"
	 "      Size, Comps: %d, %d\n"
	 "      Value: %s\n", 
	 entry,
	 exif_tag_get_name(entry->tag),
	 exif_format_get_name(entry->format),
	 entry->size,
	 (int)(entry->components),
	 exif_entry_get_value(entry, buf, sizeof(buf)));
#endif
  IMAGE_MEDIA_INFO* pImageMediaInfo = (IMAGE_MEDIA_INFO*)callback_data;
  const char* tag = exif_tag_get_name(entry->tag);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, ApertureValue);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, MaxApertureValue);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, CameraType);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, ExposureBiasValue);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, ExposureTime);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, Flash);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, FocalLength);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, MeteringMode);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, DateTime);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, Make);
  JUDGE_AND_FILL_EXIF(entry, pImageMediaInfo, tag, ISOSpeedRatings);
  return;
}

static void data_foreach_func(ExifContent *content, void *callback_data)
{
  exif_content_foreach_entry(content, content_foreach_func, callback_data);
  return;
}

int get_image_exif_info(const char *path,IMAGE_MEDIA_INFO* pImageMediaInfo)
{
	ExifData *d = NULL;
	if ( (d = exif_data_new_from_file(path)) == NULL) {
		//possibly the file does not has exif info.
		LOG_I("exif_data_new_from_file failed, path=%s\n", path);
		goto err;
	}
	exif_data_foreach_content(d, data_foreach_func, pImageMediaInfo);
	exif_data_unref(d);
	return 0;
err:
	exif_data_unref(d);	
	//ignore error even if error happen
	return 0;
}

#if 0
static int get_image_normal_info(const char *path,IMAGE_MEDIA_INFO** ppImageMediaInfo)
{
	AVFormatContext *pFormatCtx = NULL;
	int             ret=0;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;

	if (ffmpeg_obtain_info(path, &pFormatCtx, &pCodecCtx, &pCodec, AVMEDIA_TYPE_VIDEO) < 0) {
		ret = -1;
		goto err;
	}

	//fill media info
	*ppImageMediaInfo = calloc(1, sizeof(IMAGE_MEDIA_INFO));
	if (!*ppImageMediaInfo) {
		LOG_E("malloc failed\n");
		ret = -1;
		goto err;
	}
	(*ppImageMediaInfo)->XResolution = pCodecCtx->width;
	(*ppImageMediaInfo)->YResolution = pCodecCtx->height;
	
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	return 0;
err:
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	free_image_media_info(ppImageMediaInfo);
	return ret;
	
}
#endif
//the file type that has exif data
enum EXIF_FILE_POSTFIX {
	EXIF_FILE_JPG = 0,
	EXIF_FILE_JPEG,
	EXIF_FILE_TIF,
	EXIF_FILE_TIFF,
	EXIF_FILE_COUNT,
};

static char* exif_file_postfix[EXIF_FILE_COUNT] = {
	[EXIF_FILE_JPG] = "jpg",
	[EXIF_FILE_JPEG] = "jpeg",
	[EXIF_FILE_TIF] = "tif",
	[EXIF_FILE_TIFF] = "tiff",
};

static int file_has_exif_info(const char* path)
{
	int i = 0;
	char* p = strrchr(path, '.');
	if (!p) {
		LOG_E("%s is not a image file\n", path);
		return 0;
	}
	for (i = 0; i < EXIF_FILE_COUNT; ++i) {
		if (strcasecmp(p+1, exif_file_postfix[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

int get_image_media_info(const char *path,IMAGE_MEDIA_INFO** ppImageMediaInfo)
{
	AVFormatContext *pFormatCtx = NULL;
	int             ret=0;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;

	if (ffmpeg_obtain_info(path, &pFormatCtx, &pCodecCtx, &pCodec, AVMEDIA_TYPE_VIDEO) < 0) {
		ret = -1;
		goto err;
	}

	//fill media info
	*ppImageMediaInfo = calloc(1, sizeof(IMAGE_MEDIA_INFO));
	if (!*ppImageMediaInfo) {
		LOG_E("malloc failed\n");
		ret = -1;
		goto err;
	}
	(*ppImageMediaInfo)->XResolution = pCodecCtx->width;
	(*ppImageMediaInfo)->YResolution = pCodecCtx->height;
	
	if (file_has_exif_info(path) == 1) {
		get_image_exif_info(path,*ppImageMediaInfo);
	}
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	return 0;
err:
	ffmpeg_release_info(&pFormatCtx, &pCodecCtx);
	free_image_media_info(ppImageMediaInfo);
	return ret;
}



//ppVideoMediaInfo will become NULL
void free_video_media_info(VIDEO_MEDIA_INFO** ppVideoMediaInfo)
{
//	if (*ppVideoMediaInfo)
//		safe_free((*ppVideoMediaInfo)->encode);
	safe_free((*ppVideoMediaInfo));
	return;
}

//ppAudioMediaInfo will become NULL
void free_audio_media_info(AUDIO_MEDIA_INFO** ppAudioMediaInfo)
{
//	if (*ppAudioMediaInfo) {
//		safe_free((*ppAudioMediaInfo)->name);
//	}
	safe_free((*ppAudioMediaInfo));
	return;
}

void free_image_media_info(IMAGE_MEDIA_INFO** ppImageMediaInfo)
{
	safe_free((*ppImageMediaInfo));
	return;
}

char* get_lib_version()
{
	return LIB_VERSION;
}

