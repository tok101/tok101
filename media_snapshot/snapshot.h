/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#define PKG_NAME 		"lib_ffmpeg"
#define PKG_VERSION		"1.0.00"

typedef struct _OptionDmContext {
    char input_file_name[256];
	char output_file_name[256];
	#if 0
	int seek_time;//s
	int seek_level;//0-100 percent
	int width;
	int height;
	#else
	char seek_time[16];//s
	char seek_level[16];//0-100 percent
	char width[16];
	char height[16]; 
	#endif
	char log_level[16];
	char force_format[16];//force output format
	char nb_pic[16];//the picture of num for creat
} OptionDmContext;


int media_snapshot(OptionDmContext *dm_context);



#endif