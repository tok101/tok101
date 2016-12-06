#ifndef LINKLIST_H
#define LINKLIST_H
struct LinkListNode {
	void* data;
	struct LinkListNode* next;
};

struct LinkList {
	int index;
	int count;
	struct LinkListNode* head;
	struct LinkListNode* tail;
};
struct LinkList* linklist_create();
int linklist_add(struct LinkList* ll, void* data);
int linklist_travel(struct LinkList* ll, void(*do_function)(void* ));
//data_release is allow NULL
int linklist_destroy(struct LinkList** ll, void(*data_release)(void* ));
//compare should return 0 if match
void* linklist_search(struct LinkList* ll, void* find_data, int(*compare)(void* ,void* ));
void* linklist_visit(struct LinkList* ll, int index);
int linklist_get_count(struct LinkList* ll);
int linklist_del_index(struct LinkList* ll, int index, void(*free_func)(void* ));
int linklist_del(struct LinkList* ll, void* data, void(*free_func)(void* ));
#endif
