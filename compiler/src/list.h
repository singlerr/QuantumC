#ifndef _LIST_H_
#define _LIST_H_

struct list_head
{
    struct list_head *next, *prev;
};

#define container_of(ptr, type, member) ({             \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); \
})

#define LIST_HEAD_INIT(name) {&(name), &(name)}
#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr)  \
    do                       \
    {                        \
        (ptr)->next = (ptr); \
        (ptr)->prev = (ptr); \
    } while (0)
#define list_add(new, head)                                    \
    do                                                         \
    {                                                          \
        struct list_head *_new = (new), *_prev = (head)->prev; \
        _prev->next = _new;                                    \
        _new->next = (head);                                   \
        _new->prev = _prev;                                    \
        (head)->prev = _new;                                   \
    } while (0)

#define list_del(node)                     \
    do                                     \
    {                                      \
        (node)->next->prev = (node)->prev; \
        (node)->prev->next = (node)->next; \
        (node)->next = NULL;               \
        (node)->prev = NULL;               \
    } while (0)

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each_entry(pos, head, member)                 \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head);                               \
         pos = list_entry(pos->member.next, typeof(*pos), member))

#endif