#ifndef __LIST_H__
#define __LIST_H__

#include <sys/types.h>
#include <sys/resource.h>

typedef struct procnode {
    pid_t pid;
    char *cmd;
    struct procnode *next;
} procnode;

typedef struct proclist {
    struct procnode *head;
    int length;
} proclist;

struct proclist* proclist_new();
void proclist_free(struct proclist *);
void proclist_add(struct proclist *, pid_t, char *);
void proclist_remove(struct proclist *, pid_t);
struct procnode* proclist_find(struct proclist *, pid_t);
void proclist_print(struct proclist *);

#endif // __LIST_H__
