#include <stdlib.h>

#include "LinkList.h"
#include "errno.h"

struct LinkList* linklist_create()
{
	struct LinkList* pLinklist = NULL;
	pLinklist = calloc(sizeof(struct LinkList), 1); 
	if (!pLinklist) {		
		errno=ENOMEM;
		return NULL;
	}
	return pLinklist;
}
int linklist_add(struct LinkList* ll, void* data)
{
	if (ll==NULL){
		errno=EINVAL;
		return -1;
	}
	struct LinkListNode* node = calloc(sizeof(struct LinkListNode), 1);
	if (!node) {		
		errno=ENOMEM;
		return -1;
	}
	node->data = data;
	if (ll->head == NULL) 
		ll->tail = ll->head = node;
	else  {
		ll->tail->next = node;
		ll->tail = node;
	}
	ll->count++;
	return 0;
}

int linklist_del(struct LinkList* ll, void* data, void(*free_func)(void* ))
{
	if (ll==NULL || data == NULL){
		errno=EINVAL;
		return -1;
	}
	struct LinkListNode* node = ll->head;
	struct LinkListNode* tmp = ll->head;
	while(node) {
		if (data == node->data)
			break;
		tmp = node;
		node = node->next;
	}
	if (!node)
		return 0;
	if (data != node->data) {
		errno = 0;
		return -1;
	}
	if (node == ll->head) 
		ll->head = ll->head->next;
	else 
		tmp->next = node->next;
	
	if (free_func)
		(*free_func)(node->data);
	free(node);
	ll->count--;
	return 0;
}


int linklist_del_index(struct LinkList* ll, int index, void(*free_func)(void* ))
{
	if (ll==NULL){
		errno=EINVAL;
		return -1;
	}
	if (index< 0 || index > ll->count-1) {
		errno=EINVAL;
		return -1;
	}
	int i = 0;
	struct LinkListNode* node = ll->head;
	struct LinkListNode* tmp = ll->head;
	while(node) {
		if (i >= index)
			break;
		tmp = node;
		node = node->next;
		i++;
	}
	if (!node)
		return 0;
	if (i != index) {
		errno = 0;
		return -1;
	}
	if (index == 0) 
		ll->head = ll->head->next;
	else 
		tmp->next = node->next;
	
	if (free_func)
		(*free_func)(node->data);
	free(node);
	ll->count--;
	return 0;
}

int linklist_travel(struct LinkList* ll, void(*do_function)(void* ))
{	
	if (ll==NULL || do_function == NULL){
		errno=EINVAL;
		return -1;
	}
	struct LinkListNode* node = ll->head;
	while(node) {
		(*do_function)(node->data);
		node = node->next;
	}
	return 0;
}

void* linklist_search(struct LinkList* ll, void* find_data, int(*compare)(void* ,void* ))
{	
	if (ll==NULL || compare == NULL || find_data == NULL){
		errno=EINVAL;
		return NULL;
	}
	struct LinkListNode* node = ll->head;
	while(node) {
		if ((*compare)(find_data, node->data) ==0)
			return node->data;
		node = node->next;
	}
	return NULL;
}

void* linklist_visit(struct LinkList* ll, int index)
{	
	if (ll==NULL){
		errno=EINVAL;
		return NULL;
	}
	if (index< 0 || index > ll->count-1) {
		errno=EINVAL;
		return NULL;
	}
	
	int i = 0;
	struct LinkListNode* node = ll->head;
	while(node) {
		if (i >= index)
			break;
		node = node->next;
		i++;
	}
	if (i != index || node == NULL) {
		return NULL;
	}
	
	return node->data;
}

int linklist_get_count(struct LinkList* ll)
{
	if (ll==NULL){
		errno=EINVAL;
		return -1;
	}
	return ll->count;
}


//call free_func for every node_data. free_func is allow NULL
int linklist_destroy(struct LinkList** ll, void(*free_func)(void* ))
{
	if (*ll==NULL){
		return 0;
	}
	struct LinkListNode* node = (*ll)->head;
	while(node) {
		if (free_func)
			(*free_func)(node->data);
		(*ll)->head = node->next;
		free(node);
		node = (*ll)->head;
		(*ll)->count--;
	}
	free(*ll);
	*ll = NULL;
	return 0;
}

