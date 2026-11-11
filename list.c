#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "list.h"

struct proclist* proclist_new() {
    // TODO: allocate a new proclist on the heap, initialize head to NULL
    // and length to 0, and return a pointer to it
    return NULL;
}

void proclist_free(struct proclist *head) {
    // TODO: free every procnode in the list (including each node's cmd string),
    // then free the proclist struct itself
}

void proclist_add(struct proclist* head, pid_t pid, char *cmd) {
    // TODO: allocate a new procnode, copy cmd to the heap with strdup,
    // set the pid, prepend the node to the list, and increment head->length
}

struct procnode* proclist_find(struct proclist* head, pid_t pid) {
    // TODO: walk the list; return a pointer to the procnode whose pid matches,
    // or NULL if no such node exists
    return NULL;
}

void proclist_remove(struct proclist *head, pid_t pid) {
    // TODO: find the node with the given pid, unlink it from the list,
    // free its cmd string, free the node, and decrement head->length.
    // Handle the special case where the node to remove is at the head.
    // Do nothing if the list is empty or the pid is not found.
}

void proclist_print(struct proclist *head) {
    // TODO: print "Processes currently active: " followed by each node's
    // pid and cmd (one per line).  If the list is empty, print "none"
    // on the same line as the header.
}
