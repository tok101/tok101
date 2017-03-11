#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <libexif/exif-loader.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libexif/exif-data.h"
#include "exif.h"

int creat_exif_thumbnail(const char *src_file, char *thumbnail_file)
{
    int rc = 0;
    ExifLoader *l;

    if (src_file == NULL || thumbnail_file == NULL) {
        printf("Extracts a thumbnail from the given EXIF image.\n");
        return -1;
    }

    /* Create an ExifLoader object to manage the EXIF loading process */
    l = exif_loader_new();
    if (l) {
        ExifData *ed;

        /* Load the EXIF data from the image file */
        exif_loader_write_file(l, src_file);

        /* Get a pointer to the EXIF data */
        ed = exif_loader_get_data(l);

		/* The loader is no longer needed--free it */
        exif_loader_unref(l);
		l = NULL;
        if (ed) {
	    /* Make sure the image had a thumbnail before trying to write it */
            if (ed->data && ed->size) {
                FILE *thumb;

                thumb = fopen(thumbnail_file, "wb");
                if (thumb) {
		    		/* Write the thumbnail image to the file */
                    fwrite(ed->data, 1, ed->size, thumb);
                    fclose(thumb);
                    printf("Wrote thumbnail to %s\n", thumbnail_file);
                    rc = 0;
                } else {
                    printf("Could not create file %s\n", thumbnail_file);
                    rc = -1;
                }
            } else {
                printf("No EXIF thumbnail in file %s\n", thumbnail_file);
                rc = 1;
            }
	    	/* Free the EXIF data */
            exif_data_unref(ed);
        }
    }
    return rc;
}

image_rotate_info all_image_rotate_info[]=
{
	{Rotate_Multiple, ""},
	{Rotate_Top_Left, "Top-left"},
	{Rotate_Top_Right, "Top-right"},
	{Rotate_Bottom_Right, "Bottom-right"},
	{Rotate_Bottom_Left, "Bottom-left"},
	{Rotate_Left_Top, "Left-top"},
	{Rotate_Right_Top, "Right-top"},
	{Rotate_Right_Bottom, "Right-bottom"},
	{Rotate_Left_Bottom, "Left-bottom"},
	{RoTate_None, "none"},
};

#define IMAGE_ROTATE_INFO_NUM (sizeof(all_image_rotate_info)/sizeof(all_image_rotate_info[0]))

int get_exif_rotate_type(const char *src_file, int image_ifd_type)
{
	ExifData *d = NULL;
	ExifContent *ifd = NULL;
	ExifEntry *entry;
	int i = 0, j = 0;
	int rotate_type = 0;  
	char buf[2000];

	if(image_ifd_type > EXIF_IFD_COUNT){
		printf("error idf type: %d\n", image_ifd_type);
		return -1;
	}
	
	d = exif_data_new_from_file(src_file);
	if(d == NULL){
		printf("get exif error\n");
		return -1;
	}
	ifd = d->ifd[image_ifd_type];
	//printf("count: %d\n", ifd->count);

	for(i = 0; i < ifd->count && !rotate_type; i++){
		entry = ifd->entries[i];	
		if(entry->tag == EXIF_TAG_ORIENTATION){
			memset(buf, 0, sizeof(buf));
			printf("%s: %s\n", exif_tag_get_name_in_ifd(entry->tag, image_ifd_type), exif_entry_get_value(entry, buf, sizeof(buf)));
			for(j = 0; j < IMAGE_ROTATE_INFO_NUM; j++){
				memset(buf, 0, sizeof(buf));
				if(!strcmp(all_image_rotate_info[j].name, exif_entry_get_value(entry, buf, sizeof(buf)))){
					rotate_type = all_image_rotate_info[j].type;
					break;
				}
			}
		}

		#if 0
		memset(buf, 0, sizeof(buf));
		entry = ifd->entries[i];		  
		exif_entry_get_value(entry, buf, sizeof(buf));
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
	}
	exif_data_unref(d);
	return rotate_type;
}

#define OUTPUTLINESIZE 256
static int execute_process(char *cmd)
{
    FILE *pipefd = NULL;
    char buf[OUTPUTLINESIZE + 1] = {0};

	printf("execute process(%s)\n", cmd);

    pipefd = popen(cmd, "r");
    if(pipefd == NULL) {
        fprintf(stderr, "%s popen faild, errno(%d), error(%s)\n", __FUNCTION__, errno, strerror(errno));
		return -1;
    }

    while(fgets(buf, OUTPUTLINESIZE, pipefd)!=NULL) {
		fprintf(stdout, "%s", buf);
    }

    int ret = pclose(pipefd); 
	if (ret == -1 || !WIFEXITED(ret) || WEXITSTATUS(ret) != 0) {
		if (ret == -1)
        	fprintf(stderr, "pclose failed, error(%s)\n", strerror(errno) );
		else if(!WIFEXITED(ret))
        	fprintf(stderr, "process(%s) was unabled to exit normally\n", cmd);
		else
        	fprintf(stderr, "process(%s) returned errno(%d)\n", cmd, WEXITSTATUS(ret));
        return -1;
    }
	return 0;
}


int rotate_jpeg_image(const char *src_file, int degree_to_rotate)
{
	printf("start rotate image, path(%s), rotate(%d)\n", src_file, degree_to_rotate);
	char* cmd = NULL;
	asprintf(&cmd, "jpegtran -rotate %d -outfile %s %s", degree_to_rotate, src_file, src_file);
	if (!cmd) {
		fprintf(stderr, "%s asprintf failed, errno(%d), error(%s)\n", __FUNCTION__, errno, strerror(errno) );
		return -1;
	}
	int ret = execute_process(cmd);
	free(cmd);
	return ret ;
}

