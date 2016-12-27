#include "gp-params.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "internel.h"

/** gp_params_init
 * @param envp: The third char ** parameter of the main() function
 */

int
gp_params_init (GPParams *p, char **envp)
{
	if (!p) {
		fprintf (stderr, "param is null\n");		
		return -1;
	}

	memset (p, 0, sizeof (GPParams));

	p->folder = strdup ("/");
	if (!p->folder) {
		fprintf (stderr, "Not enough memory.\n");
		return -1;
	}

	if (gp_camera_new (&p->camera) < 0) {
		fprintf (stderr, "gp_camera_new error.\n");
		goto release_folder;
	};

	p->cols = 79;
	p->flags = FLAGS_RECURSE;

	/* Create a context. Report progress only if users will see it. */
	if ( (p->context = gp_context_new ()) == NULL) {
		fprintf (stderr, "gp_context_new error.\n");
		goto release_camera;
	}
/*
	gp_context_set_cancel_func    (p->context, ctx_cancel_func,  p);
	gp_context_set_error_func     (p->context, ctx_error_func,   p);
	gp_context_set_status_func    (p->context, ctx_status_func,  p);
	gp_context_set_message_func   (p->context, ctx_message_func, p);
	if (isatty (STDOUT_FILENO))
		gp_context_set_progress_funcs (p->context,
			ctx_progress_start_func, ctx_progress_update_func,
			ctx_progress_stop_func, p);
*/
	p->_abilities_list = NULL;
	p->debug_func_id = -1;
	p->envp = envp;
	return 0;
release_camera:
	gp_camera_unref (p->camera);
	p->camera = NULL;
release_folder:
	safe_free(p->folder);
	return -1;	
}


CameraAbilitiesList *
gp_params_abilities_list (GPParams *p)
{
	/* If p == NULL, the behaviour of this function is as undefined as
	 * the expression p->abilities_list would have been. */
	if (p->_abilities_list == NULL) {
		gp_abilities_list_new (&p->_abilities_list);
		gp_abilities_list_load (p->_abilities_list, p->context);
	}
	return p->_abilities_list;
}


void
gp_params_exit (GPParams *p)
{
	if (!p)
		return;

	if (p->_abilities_list)
		gp_abilities_list_free (p->_abilities_list);
	if (p->camera)
		gp_camera_unref (p->camera);
	if (p->folder)
		free (p->folder);
	if (p->filename)
		free (p->filename);
	if (p->context)
		gp_context_unref (p->context);
	if (p->hook_script)
		free (p->hook_script);
	if (p->portinfo_list)
		gp_port_info_list_free (p->portinfo_list);
	memset (p, 0, sizeof (GPParams));
}

void
_get_portinfo_list (GPParams *p) {
	int count, result;
	GPPortInfoList *list = NULL;

	if (p->portinfo_list)
		return;

	if (gp_port_info_list_new (&list) < GP_OK)
		return;
	result = gp_port_info_list_load (list);
	if (result < 0) {
		gp_port_info_list_free (list);
		return;
	}
	count = gp_port_info_list_count (list);
	if (count < 0) {
		gp_port_info_list_free (list);
		return;
	}
	p->portinfo_list = list;
	return;
}


