
#ifndef __LIST_H
#define __LIST_H

/* Self file is from Linux Kernel (include/linux/list.h)
* and modified by simply removing hardware prefetching of list items.
* Here by copyright, credits attributed to wherever they belong.
* Kulesh Shanmugasundaram (kulesh [squiggly] isis.poly.edu)
*/

/*
* Simple doubly linked list implementation.
*
* Some of the internal functions (※__xxx§) are useful when
* manipulating whole lists rather than single entries, as
* sometimes we already know the next/prev entries and we can
* generate better code by using them directly rather than
* using the generic single-entry routines.
*/

struct list_head
{
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}
/*
* Insert a New entry between two known consecutive entries.
*
* Self is only for internal list manipulation where we know
* the prev/next entries already!
*/
static void __list_add(struct list_head *New, struct list_head *prev, struct list_head *next)
{
    next->prev = New;
    New->next = next;
    New->prev = prev;
    prev->next = New;
}

/**
* list_add 每 add a New entry
* @New: New entry to be added
* @head: list head to add it after
*
* Insert a New entry after the specified head.
* Self is good for implementing stacks.
*/
static void list_add(struct list_head *New, struct list_head *head)
{
    __list_add(New, head, head->next);
}

/**
* list_add_tail 每 add a New entry
* @New: New entry to be added
* @head: list head to add it before
*
* Insert a New entry before the specified head.
* Self is useful for implementing queues.
*/
static void list_add_tail(struct list_head *New, struct list_head *head)
{
    __list_add(New, head->prev, head);
}

/*
* Delete a list entry by making the prev/next entries
* point to each other.
*
* Self is only for internal list manipulation where we know
* the prev/next entries already!
*/
static void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
* list_del 每 deletes entry from list.
* @entry: the element to delete from the list.
* Note: list_empty on entry does not return true after this, the entry is in an undefined state.
*/
static void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (struct list_head *)0;
    entry->prev = (struct list_head *)0;
}

/**
* list_del_init 每 deletes entry from list and reinitialize it.
* @entry: the element to delete from the list.
*/
static void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
* list_move 每 delete from one list and add as another＊s head
* @list: the entry to move
* @head: the head that will precede our entry
*/
static void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
* list_move_tail 每 delete from one list and add as another＊s tail
* @list: the entry to move
* @head: the head that will follow our entry
*/
static void list_move_tail(struct list_head *list,
struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/**
* list_empty 每 tests whether a list is empty
* @head: the list to test.
*/
static int list_empty(struct list_head *head)
{
    return head->next == head;
}

static void __list_splice(struct list_head *list, struct list_head *head)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;
    struct list_head *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}

/**
* list_splice 每 join two lists
* @list: the New list to add.
* @head: the place to add it in the first list.
*/
static void list_splice(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head);
}

/**
* list_splice_init 每 join two lists and reinitialise the emptied list.
* @list: the New list to add.
* @head: the place to add it in the first list.
*
* The list at @list is reinitialised
*/
static void list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head);
        INIT_LIST_HEAD(list);
    }
}

/**
* list_entry 每 get the struct for this entry
* @ptr:    the &struct list_head pointer.
* @type:    the type of the struct this is embedded in.
* @member:    the name of the list_struct within the struct.
*/
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
* list_for_each    -    iterate over a list
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); \
        pos = pos->next)
/**
* list_for_each_prev    -    iterate over a list backwards
* @pos:    the &struct list_head to use as a loop counter.
* @head:    the head for your list.
*/
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); \
        pos = pos->prev)

/**
* list_for_each_safe    -    iterate over a list safe against removal of list entry
* @pos:    the &struct list_head to use as a loop counter.
* @n:        another &struct list_head to use as temporary storage
* @head:    the head for your list.
*/
#define list_for_each_safe(type, pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head);\
        pos = n, n = pos->next)


/**
* list_for_each_entry    -    iterate over list of given type
* @pos:    the type * to use as a loop counter.
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/
#define list_for_each_entry(type, pos, head, member)                \
    for (pos = list_entry((head)->next, type, member); \
        &pos->member != (head);                                  \
            pos = list_entry(pos->member.next, type, member))

/**
* list_for_each_entry_safe 每 iterate over list of given type safe against removal of list entry
* @pos:    the type * to use as a loop counter.
* @n:        another type * to use as temporary storage
* @head:    the head for your list.
* @member:    the name of the list_struct within the struct.
*/
#define list_for_each_entry_safe(type, pos, n, head, member)            \
for (pos = list_entry((head)->next, type, member),    \
    n = list_entry(pos->member.next, type, member);    \
        &pos->member != (head);                     \
            pos = n, n = list_entry(n->member.next, type, member))

#endif