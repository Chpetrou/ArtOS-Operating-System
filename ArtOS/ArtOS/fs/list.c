#include <list.h>

/*Implementation of a double linked list in C for ArtOS*/

//Push into head in list
void list_push_head( struct list *list, struct list_node *node )
{
	node->next = list->head;
	node->prev = 0;
	node->priority = 0;

	if(list->head) list->head->prev = node;
       	list->head = node;
	if(!list->tail) list->tail = node;
	node->list = list;
}

//Push into tail in list
void list_push_tail( struct list *list, struct list_node *node )
{
	node->prev = list->tail;
	node->next = 0;
	node->priority = 0;

	if(list->tail) list->tail->next = node;
	list->tail = node;
	if(!list->head) list->head = node;
	node->list = list;
}

//Push with priority in list
void list_push_priority( struct list *list, struct list_node *node, int pri )
{
	struct list_node *n;
	int i=0;

	if(!list->head) {
		list_push_head(list,node);
		return;
	}

	for(n=list->head;n;n=n->next) {
		if(pri>n->priority || i>5000) {
			node->next = n;
			node->prev = n->prev;
			node->priority = pri;

			if(n->prev) {
				n->prev->next = node;
			} else {
				list->head = node;
			}
			n->prev = node;
			node->list = list;
			return;
		}			
		i++;
	}

	list_push_tail(list,node);
}

//Pop from head in list
struct list_node * list_pop_head( struct list *list )
{
	struct list_node *result = list->head;

	if(list->head) {
		list->head = list->head->next;
		if(list->head) list->head->prev = 0;
		if(!list->head) list->tail = 0;

		result->next = result->prev = 0;
		result->list = 0;
	}

	return result;
}

//Pop from tail in list
struct list_node * list_pop_tail( struct list *list )
{
	struct list_node *result = list->tail;

	if(list->tail) {
		list->tail = list->tail->prev;
		if(list->tail) list->tail->next = 0;
		if(!list->tail) list->head = 0;

		result->next = result->prev = 0;
		result->list = 0;
	}

	return result;
}

//Remove a list
void list_remove( struct list_node *node )
{
	if(!node->list) return;

	if(node->list->head==node) {
		list_pop_head(node->list);
		return;
	}

	if(node->list->tail==node) {
		list_pop_tail(node->list);
		return;
	}

	node->next->prev = node->prev;
	node->prev->next = node->next;

	node->next = node->prev = 0;
	node->list = 0;
}
