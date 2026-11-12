#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "list.h"

struct proclist* proclist_new() {
    proclist list;
    list.head = NULL;
    list.length = 0;
    proclist *l = malloc(sizeof(proclist));
    *l = list;
    return l;
}

void proclist_free(struct proclist *head) {
    procnode *curr = head -> head;
    procnode *next;
    while(curr != NULL){
        free(curr -> cmd);
        next = curr -> next;
        free(curr);
        curr = next;
    }
    free(head);
}

void proclist_add(struct proclist* head, pid_t pid, char *cmd) {
    if(head -> length == 0){
        procnode n;
        procnode *node = malloc(sizeof(procnode));
        n.pid = pid;
        n.cmd = malloc(strlen(cmd) + 1);
        memcpy(n.cmd, cmd, strlen(cmd) + 1);
        n.next = NULL;

        *node = n;
        head -> head = node;
        head -> length++;
        return;
    }

    procnode *curr = head -> head;
    procnode *next = curr -> next;
    while(next != NULL){
        curr = next;
        next = curr -> next;
    }
    procnode n;
    procnode *node = malloc(sizeof(procnode));
    n.pid = pid;
    n.cmd = malloc(strlen(cmd) + 1);
    memcpy(n.cmd, cmd, strlen(cmd) + 1);
    n.next = NULL;

    *node = n;
    curr -> next = node;
    head -> length ++;
}

struct procnode* proclist_find(struct proclist* head, pid_t pid) {

    procnode *curr = head -> head;
    while(curr != NULL){
        if(curr -> pid == pid){
            return curr;
        }
        curr = curr -> next;
    }
    return NULL;
}

void proclist_remove(struct proclist *head, pid_t pid) {
    procnode* curr = head -> head;
    procnode* next = curr -> next;
    if(curr -> pid == pid){
        head -> head = next;
        free(curr -> cmd);
        free(curr);
        head -> length --;
    }
    while(next != NULL){
        if(next -> pid == pid){
            curr -> next = next -> next;
            free(next -> cmd);
            free(next);
            head -> length --;
            return;
        }
        curr = next;
        next = curr -> next;
    }

}

void proclist_print(struct proclist *head) {
    printf("Processes currently active ");
    if(head -> length == 0){
        printf("none\n");
    }else{
        printf("\n");
    }
    procnode *curr = head -> head;
    while(curr != NULL){
        printf("%i , %s\n", curr -> pid, curr -> cmd);
        curr = curr -> next;
    }
}

pid_t proclist_get(proclist *head, int index){
    procnode *curr = head -> head;
    for(int i = 0; i < index; i++){
        if(curr -> next == NULL){
            perror("Index out of bounds\n");
        }
        curr = curr -> next;
    }
    return curr -> pid;
}
