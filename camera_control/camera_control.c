#include <stdio.h>
#include "gp-params.h"
#include "internel.h"
#include "camera_control.h"
#include <inttypes.h>

#include <string.h>
#include <stdlib.h>


static char* make_new_path(const char* folder, const char* filename)
{
	char* new_path = NULL;
	int len = strlen(folder)+strlen(filename)+5;
	CK01 (new_path = (char*)malloc(len), err_maloc_newpath);
	if (folder[strlen(folder)-1] == '/')
		snprintf(new_path, len, "%s%s", folder, filename);
	else
		snprintf(new_path, len, "%s/%s", folder, filename);
	return new_path;
err_maloc_newpath:
	LOG_E("out of memeory");
	return NULL;
}
static int try_get_camera_file(GPParams* gp_params, const char* folder, const char* filename)
{
	int ret = 0;
    CameraFile *file = NULL;
	if ( (ret = gp_file_new (&file)) < 0) {
		LOG_E("gp_file_new failed %d\n", ret);
		return -1;
	}
    if ( (ret = gp_camera_file_get (gp_params->camera, folder, filename, GP_FILE_TYPE_NORMAL,
			  file, gp_params->context)) < 0) {
		LOG_I("gp_camera_file_get: %s\n", gp_port_result_as_string(ret));
	}
	gp_file_unref (file);
	return ret;
}

static int foreach_entry(char* folder, GPParams* gp_params, enum EntryType type, PHandleFunc func, void* args)
{
	CameraList *list = NULL;
	int ret = 0, count = 0;
	CK00 (ret = gp_list_new (&list), err_newlist);
	if (type == ET_FOLDER)
		CK00 (ret = gp_camera_folder_list_folders (gp_params->camera, folder, list, gp_params->context), err_listfolder);
	else if (type == ET_FILE)
		CK00 (ret = gp_camera_folder_list_files (gp_params->camera, folder, list, gp_params->context), err_listfiles);
	else {
		LOG_E("unknow type :%d\n", type);
		goto release_list;
	}
	CK00 (count = ret = gp_list_count (list), err_listcount);

	int i = 0;
	for (i = 0; i < count; ++i) {
		const char* tmp_name = NULL;
		CK00 (ret= gp_list_get_name (list, i, &tmp_name), err_listgetname);
		if (type == ET_FILE && try_get_camera_file(gp_params, folder, tmp_name) < 0) {
			continue;
		}
		char* new_folder = NULL;
		CK01 (new_folder = make_new_path(folder,tmp_name), err_makpath);
		func(new_folder, gp_params, args);
		safe_free(new_folder);
	}
release_list:
	if (list)
		gp_list_free (list);
out:
	return ret;

err_newlist:
	LOG_E("gp_list_new error,retcode = %d", ret);
	ret = -1;
	goto out;
err_listfolder:
	LOG_E("gp_camera_folder_list_folders error,retcode = %d", ret);
	ret = -1;
	goto release_list;
err_listfiles:
	LOG_E("gp_camera_folder_list_files error,retcode = %d", ret);
	ret = -1;
	goto release_list;
err_listcount:
	LOG_E("gp_list_count error,retcode = %d", ret);
	ret = -1;
	goto release_list;
err_listgetname:
	LOG_E("gp_list_get_name error,retcode = %d", ret);
	ret = -1;
	goto release_list;
err_makpath:
	ret = -1;
	goto release_list;	
}

struct FuncArgu {
	PHandleFunc func;
	void* func_args;
};

static int get_file_recursive(char* path, GPParams* gp_params, struct FuncArgu* args)
{
	foreach_entry(path, gp_params, ET_FILE, args->func, NULL);
	foreach_entry(path, gp_params, ET_FOLDER, (PHandleFunc)get_file_recursive, args);
	return 0;
}

int cam_detect_camera (GPParams *gp_params)
{
	int x, count;
    CameraList *list;
    const char *name = NULL, *value = NULL;
	int ret = 0;
	
	_get_portinfo_list (gp_params);
	if  ( (ret = gp_list_new (&list)) < 0)
		goto err_no;
    gp_abilities_list_detect (gp_params_abilities_list(gp_params), gp_params->portinfo_list, list, gp_params->context);


    if ((ret = count = gp_list_count (list)) < 0)
		goto err_list;

    LOG_I(("%-30s %-16s\n"), ("Model"), ("Port"));
    LOG_I(("----------------------------------------------------------\n"));
    for (x = 0; x < count; x++) {
            (gp_list_get_name  (list, x, &name), list);
            (gp_list_get_value (list, x, &value), list);
            LOG_I(("%-30s %-16s\n"), name, value);
    }
	gp_list_free (list);
	
    return count;
err_list:
	gp_list_free (list);
err_no:
	return ret;
}

int get_all_files(PHandleFunc func, void* func_args)
{
	int ret = 0;
	if (!func) {
		LOG_E("func is null\n");
		ret = -1;
		goto out;
	}
	GPParams gp_params = {0};
	
	gp_params_init (&gp_params, NULL);
	if ( (ret = cam_detect_camera(&gp_params)) < 0)
		goto err_cam_detect;
	if (ret == 0)
		goto no_camera_detect;
	struct FuncArgu args = {func, func_args};
	get_file_recursive("/", &gp_params, &args);
	
release_param:
	gp_params_exit(&gp_params);
out:
	return ret;

no_camera_detect:
	LOG_W("no camera detected\n");
	goto release_param;
err_cam_detect:
	ret = -1;
	goto release_param;
}

static int split_path_folder(const char* path, char **folder_O, char** name_O)
{
	int	ret = 0;
	char* tmp;
	char* folder = NULL;
	if ( (folder = strdup(path)) == NULL) {
		LOG_E("strdup folder failed\n");
		ret = -1;
		goto err_no;
	}
	if ( (tmp = strrchr(folder, '/')) == NULL) {
		LOG_E("invailed path = %s\n", folder);
		ret = -1;
		safe_free(folder);
		goto err_no;
	}
	*tmp = '\0';
	char* name = NULL;
	if ( (name = strdup(tmp+1)) == NULL) {
		LOG_E("strdup name failed\n");
		ret = -1;
		safe_free(folder);
		goto err_no;
	}
	*folder_O = folder;
	*name_O = name;
err_no:
	return ret;
}

int read_camera_file(const char* path, GPParams* gp_params, uint64_t offset, 
		uint64_t size, unsigned char*	data)
{
	int ret = 0;
	CK01 (path, err_paranul_no);
	CK01 (data, err_paranul_no);
	char* folder = NULL;
	char* name = NULL;
		
	LOG_I("get file info from camera,file_path = %s,offset = %"PRIu64",size_to_read = %"PRIu64"\n", path, offset, size);

	CK00 (ret = split_path_folder(path, &folder, &name), err_no);
	CK00 (ret = gp_camera_file_read(gp_params->camera, folder, name, GP_FILE_TYPE_NORMAL, offset, 
									(char*)data, &size, gp_params->context), err_path);
	
	safe_free(folder);
	safe_free(name);
	return size;
	
err_paranul_no:
	LOG_E("parameter is null");
	goto err_no;	
err_path:
	safe_free(folder);
	safe_free(name);
err_no:
	return -1;
}


