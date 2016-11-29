#ifndef MEDIA_TABLE_H
#define MEDIA_TABLE_H

#include "db_base.h"

#define MAX_ENCODE_TYPE_SIZE    8

typedef struct  VideoMediaInfoTable
{
	uint32_t index;
	uint16_t width;		// in pixels
	uint16_t height;	// in pixels
	uint32_t bitrate; // in bps 
	char encode_type[MAX_ENCODE_TYPE_SIZE];
	uint32_t duration_time; // in seconds
}video_info_t;


#define MAX_VENDOR_NAME_SIZE        64
#define MAX_CAMERA_TYPE_NAME_SIZE   64
#define MAX_APERTURE_VALUE_SIZE     16

typedef struct PhotoMediaInfoTable
{
	uint32_t index;
	uint16_t width;
	uint16_t height;
	uint64_t photo_time;
	char vendor_name[MAX_VENDOR_NAME_SIZE];
	char camera_type[MAX_CAMERA_TYPE_NAME_SIZE];
	char aperture_value[MAX_APERTURE_VALUE_SIZE];
	uint16_t exposure_time;
	uint32_t iso_value;
	uint32_t exposure_bias_val;
	uint32_t focal_length;
	uint32_t max_apeture_val;
	uint8_t meter_mode;
	uint8_t flashlight_on;
}image_info_t;


#define MAX_SONG_NAME_SIZE      128
#define MAX_ALBUM_NAME_SIZE     128
#define MAX_ARTIST_NAME_SIZE    128

typedef struct AudioMediaInfoTable
{
	uint32_t index;
	uint32_t duration_time; // in seconds
	uint32_t bitrate; // in bps
	char song_name[MAX_SONG_NAME_SIZE];
	char album_name[MAX_ALBUM_NAME_SIZE];
	char artist_name[MAX_ARTIST_NAME_SIZE];
	char encode_type[MAX_ENCODE_TYPE_SIZE];	
}audio_info_t;


typedef union
{
    video_info_t video_info;
	audio_info_t audio_info;
	image_info_t image_info;    
}media_info_t;

error_t load_audio_update_cmd(sqlite3 *database,audio_info_t *paudio);

error_t load_image_update_cmd(sqlite3 *database,image_info_t *pimage);

error_t load_video_update_cmd(sqlite3 *database,video_info_t *pvideo);

#endif
