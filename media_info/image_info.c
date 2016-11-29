#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <libexif/exif-data.h>

#include <png.h>

#include "internel.h"
#include "image_info.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;    /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
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

int check_image_type(const char *src_img_file)
{
	FILE *src_file = NULL;
	char src_buf[9];
	int len = 0;
	int ret = 0;
	memset(src_buf, 0, sizeof(src_buf));

	src_file = fopen(src_img_file, "rb");
	if(src_file == NULL){
		printf("open src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}

	fseek(src_file,0L,SEEK_SET);
	len = fread(src_buf, sizeof(char), sizeof(src_buf)-1, src_file);
	if(len != sizeof(src_buf)-1){
		printf("read buf fail !\r\n");
		ret = -1;
		goto EXIT;
	}

	if(memcmp(src_buf, "\377\330\377", 3) == 0){
		printf("is jpeg type\r\n");
		ret = IS_JPEG_TYPE;
		goto EXIT;
	}
	else if(memcmp(src_buf, "GIF8", 4) == 0){
		printf("is gif type\r\n");
		ret = IS_GIF_TYPE;
		goto EXIT;
	}
	else if (memcmp(src_buf, "\211PNG\r\n\032\n", 8) == 0){
		printf("is png type\r\n");
		ret = IS_PNG_TYPE;
		goto EXIT;
	}
	else{
		printf("is unknow tpye: %s\r\n", src_buf);
		ret = IS_UNKONW_TYPE;
		goto EXIT;
	}
EXIT:
	if(src_file != NULL)
		fclose(src_file);
	return ret;
}


int get_jpeg_media_info(const char *src_img_file,IMAGE_MEDIA_INFO* pImageMediaInfo)
{
	SrcJpegImgInfo src_jpeg_info;
	int ret = 0;
	FILE *src_file = NULL;
    
	memset(&src_jpeg_info, 0, sizeof(SrcJpegImgInfo));
	if(src_img_file == NULL || pImageMediaInfo == NULL){
		printf("invalid argument!\r\n");
		ret = -1;
		goto EXIT;
	}
	
	src_file = fopen(src_img_file, "rb");
	if(src_file == NULL){
		printf("open src image file failed(%s)!\r\n", src_img_file);
		ret = -1;
		goto EXIT;
	}
	//printf("start decompress!\r\n");
	src_jpeg_info.dc_info.err = jpeg_std_error(&src_jpeg_info.dc_jerr);
    /* We set up the normal JPEG error routines, then override error_exit. */
    src_jpeg_info.dc_jerr.error_exit = my_error_exit;
     /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(src_jpeg_info.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
        jpeg_destroy_decompress(&src_jpeg_info.dc_info);
		ret = -1;
		goto EXIT;
    }
    
	jpeg_create_decompress(&src_jpeg_info.dc_info);
	jpeg_stdio_src(&src_jpeg_info.dc_info, src_file);
	jpeg_read_header(&src_jpeg_info.dc_info, TRUE);

	pImageMediaInfo->XResolution = src_jpeg_info.dc_info.image_width;
	pImageMediaInfo->YResolution = src_jpeg_info.dc_info.image_height;
	get_image_exif_info(src_img_file,pImageMediaInfo);


	jpeg_destroy_decompress(&src_jpeg_info.dc_info);

	return ret;
EXIT:
	if(src_file != NULL)
		fclose(src_file);
	return ret;
}


int get_png_wh(const char* name, int *m_width, int *m_height)
{
	png_infop info_ptr = NULL;             
	png_structp png_ptr = NULL;  
	int ret = 0;
	FILE* file = fopen(name, "rb");    
	if (!file) {
		LOG_E("fopen %s failed\n", name);
		ret = -1;
		goto err;
	}
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);  
	if (png_ptr == NULL){
		LOG_E("png_create_read_structfailed\n");
		ret = -1;
		goto err_file;
	}
	info_ptr = png_create_info_struct(png_ptr);      
	if (info_ptr == NULL){
		LOG_E("png_create_info_struct\n");
		ret = -1;
		goto err_struct;
	}
	setjmp(png_jmpbuf(png_ptr));               
	png_init_io(png_ptr, file);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);        

	if ((png_ptr != NULL) && (info_ptr != NULL))
	{
		*m_width = png_get_image_width(png_ptr, info_ptr);            
		*m_height = png_get_image_height(png_ptr, info_ptr);                         
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(file);

	return 0;
err_struct:
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);	//ok even if info_ptr == NULL
err_file:
	fclose(file);	
err:
	return ret;
}


int get_png_media_info(const char *src_img_file,IMAGE_MEDIA_INFO* pImageMediaInfo)
{
	int width = 0, height = 0;
	int ret = 0;
	if ( (ret = get_png_wh(src_img_file, &width, &height)) < 0) {
		ret = -1;
		goto err;
	}
	pImageMediaInfo->XResolution = width;
	pImageMediaInfo->YResolution = height;
	get_image_exif_info(src_img_file,pImageMediaInfo);
	return 0;
err:
	return ret;
}
