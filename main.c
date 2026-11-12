#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "list.h"


/*
 * split - tokenize a string on any character in sep.
 *
 * Returns a NULL-terminated heap-allocated array of heap-allocated strings.
 * Free it with cleanup_split() when done.
 *
 * Note: this is the same pattern as the split() function from Project 2.
 * The difference here is that sep is a *string* of separator characters
 * (passed directly to strtok), so we can split on " \t\n" or on ";"
 * as needed for shell parsing.
 */
char** split(const char *s, const char *sep) {
    char *word = NULL;

    // make a copy of the input string
    char *s1 = strdup(s);

    // count how many tokens there are
    int words = 0;
    for (word = strtok(s1, sep);
         word;
         word = strtok(NULL, sep)) words++;
    free(s1);

    s1 = strdup(s);

    // allocate array of char *'s (one extra slot for the NULL terminator)
    char **array = (char **)malloc(sizeof(char *) * (words + 1));
    int i = 0;
    for (word = strtok(s1, sep);
         word;
         word = strtok(NULL, sep)) {
        array[i] = strdup(word);
        i++;
    }
    array[i] = NULL;
    free(s1);
    return array;
}

/*
 * cleanup_split - free the array returned by split().
 */
void cleanup_split(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

/*
 * handle_comment - strip everything from the first '#' to end of string.
 */
void handle_comment(char *buffer) {
    for (int i = 0; i < (int)strlen(buffer); i++) {
        if (buffer[i] == '#') {
            buffer[i] = '\0';
            break;
        }
    }
}

/*
 * child_done - return true if the child process with the given pid has
 * already finished (uses waitpid with WNOHANG so it never blocks).
 */

bool child_done(pid_t pid) {
    pid_t result = waitpid(pid, NULL, WNOHANG);
    if (result == pid) {
        return true;
    }

    return false;
}

/*
 * sweep_children - remove any finished background children from plist.
 *
 * Walk plist; for each node whose child is done, collect its pid.
 * After the walk, remove those pids from plist (do not remove while
 * iterating — build a separate kill list first).
 */
void sweep_children(struct proclist *plist) {
    proclist *kill = proclist_new();
    procnode *curr = plist -> head;
    while(curr != NULL){
        pid_t pid = curr -> pid;
        if(child_done(pid)){
            proclist_add(kill, curr -> pid, curr -> cmd);
        }
        curr = curr -> next;
    }
    curr = kill -> head;
    while(curr != NULL){
        proclist_remove(plist, curr -> pid);
        curr = curr -> next;
    }
    proclist_free(kill);
}

/*
 * check_background - return true if cmd ends with '&', and strip the '&'
 * (replace it with a space) so the rest of the parsing sees a clean string.
 */
bool check_background(char *cmd) {
    int len = strlen(cmd);
    for(int i = len - 1; i >= 0; i--){
        if(cmd[i] == '&'){
            cmd[i] = ' ';
            return true;
        }
    }
    return false;
}

/*
 * spawn_process - fork a child and exec cmd[0] with arguments cmd.
 *
 * If wait_for_child is true, wait for the child to finish before returning.
 * Returns the child's pid.
 *
 * Use execvp (not execv) so the PATH is searched automatically — the
 * user can type "echo" instead of "/bin/echo".
 */
pid_t spawn_process(char **cmd, bool wait_for_child) {

    pid_t pid = fork();
    if(pid < 0){
        perror("Failed to create process\n");
        exit(-1);
    }else if(pid == 0){
        execvp(cmd[0], cmd);
        perror("execvp");
        _exit(127);
    }
    // in parent optionally waitpid; return child pid
    if(wait_for_child){
        waitpid(pid, NULL, 0);
    }
    return pid;
}

// Execute exit command
bool spawn_exit(bool background_processes){
    if(background_processes){
        printf("Cannot exit with child processes still running\n");
        return 1;
    }
    return 0;
}

// Execute kill command
void spawn_kill(pid_t pid, proclist *plist){
    if(proclist_find(plist, pid) != NULL){
        kill(pid, SIGKILL);
        printf("Killed process %d\n", pid);
    }else{
        printf("kill: process %d not found\n", pid);
    }
}

/*
 * process_command_line - parse and execute one line of shell input.
 *
 * Returns true to keep the shell running, false to exit.
 */
bool process_command_line(char *buffer, struct proclist *plist) {
    //   1. call handle_comment(buffer)
    bool toreturn = 1;
    handle_comment(buffer);
    //   2. split on ";" to get individual commands
    char **commands = split(buffer, ";");
    int i = 0;
    //   3. for each command:
    while(commands[i] != NULL){
        char *com = commands[i];
        bool background = check_background(com);
    //      a. check_background to detect (and strip) trailing '&'
    //      b. split on " \t\n" to get argv-style tokens
        char **tokens = split(com, " \t\n");
    //      c. skip if first token is NULL (empty command)
        if(tokens == NULL || *tokens == NULL){
            free(tokens);
            i++;
            continue;
        }
        if(!strcmp(*tokens, "exit")){
            sweep_children(plist);
            if(!spawn_exit(plist -> length)){
                toreturn = 0;
            }
        }else if(!strcmp(*tokens, "status")){
            proclist_print(plist);
        }else if (!strcmp(*tokens, "kill")){
            spawn_kill(atoi(tokens[1]), plist);
        }else{
            if(background){
                proclist_add(plist, spawn_process(tokens, 0), com);
            }else{
                spawn_process(tokens, 1);
            }
        }
        
        cleanup_split(tokens);
        i ++;
    }
    cleanup_split(commands);
    return toreturn;
}

int main(int argc, char **argv) {
    struct proclist *plist = proclist_new();
    char buffer[1024];
    printf("prompt> ");
    fflush(stdout);
    while (true) {
        if (fgets(buffer, 1024, stdin) == NULL) {
            sweep_children(plist);
            if(plist -> length > 0){
                printf("Cannot exit with child processes still running\n");
            }
            procnode *curr = plist -> head;
            while(curr != NULL){
                waitpid(curr -> pid, NULL, 0);
                curr = curr -> next;
            }
            break;
        } else {
            if(!process_command_line(buffer, plist)){
                break;
            }
        }
        printf("prompt> ");
        fflush(stdout);
        sweep_children(plist);
    }

    proclist_free(plist);
    return EXIT_SUCCESS;
}
