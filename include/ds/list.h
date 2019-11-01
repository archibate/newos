#ifndef _LIST_H
#define _LIST_H 1

// Get container_of, NULL.
#include <stddef.h>

// Invalid pointer after node removed from list
#define LIST_HOLE ((void *)-1)

struct list_head
{
	struct list_node *first;
};

#define LIST_INIT { .first = NULL }

struct list_node
{
	struct list_node *next, **pprev;
};

static void list_insert_before(
		struct list_node *node,
		struct list_node *next)
{
	*next->pprev = node;
	node->pprev = next->pprev;
	node->next = next;
	next->pprev = &node->next;
}

static void list_insert_after(
		struct list_node *node,
		struct list_node *prev)
{
	node->next = prev->next;
	if (prev->next)
		prev->next->pprev = &node->next;
	prev->next = node;
	node->pprev = &prev->next;
}

static inline void list_insert_head(
		struct list_node *node,
		struct list_head *head)
{
	node->next = head->first;
	node->pprev = &head->first;
	head->first = node;
}

static void list_insert_tail(
		struct list_node *node,
		struct list_head *head)
{
	struct list_node *tail;
	if (!head->first)
		return list_insert_head(node, head);
	for (tail = head->first; tail->next; tail = tail->next);
	list_insert_after(node, tail);
}

static void list_insert_before_ex(
		struct list_node *node,
		struct list_head *head,
		struct list_node *pos)
{
	if (!pos)
		list_insert_tail(node, head);
	else
		list_insert_before(node, pos);
}

static void list_insert_after_ex(
		struct list_node *node,
		struct list_head *head,
		struct list_node *pos)
{
	if (!pos)
		list_insert_tail(node, head);
	else
		list_insert_after(node, pos);
}

static inline void __list_remove(
		struct list_node *node)
{
	*node->pprev = node->next;
	node->pprev = LIST_HOLE;
}

static inline void list_remove(
		struct list_node *node)
{
	__list_remove(node);
	node->next = LIST_HOLE;
}

#define list_entry(node, type, list) container_of(node, type, list)
#define list_next(entry, list) list_entry((entry)->list.next, typeof(*(entry)), list)
#define list_foreach(entry, head, list, ...) \
	for (__VA_ARGS__ (entry) = list_entry((head)->first, typeof((*entry)), list); \
			&(entry)->list; (entry) = list_next(entry, list))
#define list_remove_foreach(entry, head, list, ...) \
	for ((entry) = list_entry((head)->first, typeof((*entry)), list); \
			&(entry)->list; list_remove(&(entry)->list), \
			(entry) = list_entry((head)->first, typeof((*entry)), list))

#endif
