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
    // TODO: call waitpid with WNOHANG; return true if the child is done
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
    // TODO
}

/*
 * check_background - return true if cmd ends with '&', and strip the '&'
 * (replace it with a space) so the rest of the parsing sees a clean string.
 */
bool check_background(char *cmd) {
    // TODO: scan from the end of cmd looking for '&'; if found, replace
    // it with ' ' and return true; otherwise return false
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
    // TODO: fork(); in child call execvp(cmd[0], cmd) with error handling;
    // in parent optionally waitpid; return child pid
    return -1;
}

/*
 * process_command_line - parse and execute one line of shell input.
 *
 * Returns true to keep the shell running, false to exit.
 */
bool process_command_line(char *buffer, struct proclist *plist) {
    // TODO:
    //   1. call handle_comment(buffer)
    //   2. split on ";" to get individual commands
    //   3. for each command:
    //      a. check_background to detect (and strip) trailing '&'
    //      b. split on " \t\n" to get argv-style tokens
    //      c. skip if first token is NULL (empty command)
    //      d. handle built-ins: exit, status, kill <pid>
    //      e. otherwise call spawn_process (foreground or background)
    //   4. free all allocated memory
    //   5. return true (or false if "exit" was requested with no running jobs)
    return true;
}

int main(int argc, char **argv) {
    struct proclist *plist = proclist_new();
    char buffer[1024];
    printf("prompt> ");
    fflush(NULL);
    while (true) {
        if (fgets(buffer, 1024, stdin) == NULL) {
            // TODO: on EOF, if background jobs are still running, print a
            // warning once and wait (block) for all of them to finish
            break;
        } else {
            // TODO: call process_command_line; if it returns false, break
        }
        printf("prompt> ");
        fflush(NULL);
        // TODO: call sweep_children to reap any finished background processes
    }

    proclist_free(plist);
    return EXIT_SUCCESS;
}
