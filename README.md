# Project 4: Build a Shell

## Overview
For this project, you'll work with a partner to implement a *command-line interpreter* (shell) — a program that repeatedly reads a line of input, executes the command(s) specified, and waits for them to finish before asking for the next line.

Your shell will be similar in spirit to `bash` (the same shell that starts in the terminal within codespaces), but simplified.  You'll get practice with:

* `fork` — creating child processes
* `execvp` — replacing a child's image with a new program
* `waitpid` — waiting for (or polling) children
* Linked-list management — tracking background jobs

The project is divided into three stages.  **Complete them in order.**

### ❗ Important tips ❗

* **Thoroughly read the project instructions before you write any code.**  Research shows that skimming leads to missed details, which makes it harder to create a correct, well-designed program.
* **Start the project shortly after it is released and work on it over multiple sessions.**  This gives you time to think through problems, ask questions, and produce better outcomes.  The opportunity to revise your project is contingent on your git commit history demonstrating that you started early and worked incrementally.
* **Adhere to the [program design](#program-design) requirements while writing your code, instead of refactoring afterwards.**  For example, it is much easier to add a helper function as you write it than to factor out a monolithic `main` later.
* **Test and debug early and often.**  Run `valgrind` starting with Stage 1 — memory errors are far easier to fix before you have a lot of code.

### Getting started
Accept the Classroom50 invitation on Moodle (click on the link).  Open the repository on Github codespaces.  To submit, commit and push your code to Github.

**Don't forget to commit and push your code and shut down your codespace after each editing session!**

Work in **groups of 2**.

Your repository includes a `Makefile`.  Running `make` produces two binaries: `shell` (the main shell) and `listtest` (a unit tester for your linked list).

---

## Stage 1: Linked List (`list.c`)

Before writing any shell code, implement the six linked-list functions in `list.c`.  Read `list.h` carefully — it defines two structs:

* **`struct proclist`** — the "header" node; holds a pointer to the first list node and the current length.  Your shell will have exactly one `proclist`.
* **`struct procnode`** — a single list node; holds a process id (`pid`), a heap-allocated copy of the command name (`cmd`), and a `next` pointer.

| Function | What it should do |
|---|---|
| `proclist_new` | Allocate and initialize a new `proclist` on the heap; return a pointer to it |
| `proclist_free` | Free every `procnode` (and its `cmd` string) in the list, then free the `proclist` itself |
| `proclist_add` | Allocate a new `procnode`, copy `cmd` to the heap (use `strdup`), and prepend it to the list; increment `length` |
| `proclist_remove` | Find the node with the given `pid`, remove it from the list, free its `cmd`, free the node; decrement `length` |
| `proclist_find` | Search the list for the given `pid`; return a pointer to the matching node, or `NULL` if not found |
| `proclist_print` | Print `Processes currently active: ` followed by a newline and then one `pid cmd` line per process; if the list is empty print `Processes currently active: none` |

**Testing your list:** run `./runtests.sh 1`.  This builds your code, runs `./listtest` (a unit test for your list, which should report `all tests passed`), and runs `valgrind ./listtest` to check for memory errors and leaks.  See [Testing and debugging](#testing-and-debugging) for details.

---

## Stage 2: Basic Shell (`main.c`)

Once your list works, implement the basic shell loop in `main.c`.  When `make` succeeds you will have a `shell` binary.

### Helper functions (already provided)

* **`split(s, sep)`** — tokenizes a string `s` on the separator characters in `sep`; returns a `NULL`-terminated array of `char *`.  This is the same pattern as `split` from Project 2 — here `sep` is a *string* of separator characters (e.g., `" \t\n"` or `";"`) rather than a single `char`. 
* **`cleanup_split(tokens)`** — frees the array returned by `split`.
* **`handle_comment(buffer)`** — strips everything from the first `#` to the end of the string.

### What you need to implement

#### Prompt loop (`main`)

A skeleton is provided.  Fill in the loop body:

* Print `prompt> ` and flush stdout before every read.
* Read a line with `fgets` (buffer size 1024).
* On EOF (`fgets` returns `NULL`): if background jobs are running, print `Cannot exit with child processes still running` once, then wait (blocking `waitpid`) for every background job to finish and remove it from the list.  Then break out and exit.
* Otherwise, call `process_command_line`.  If it returns `false`, break out and exit.
* After each iteration, sweep for completed background children.

When the loop exits, free the proclist.

#### `process_command_line(buffer, plist)` → `bool`

Returns `true` to keep running, `false` to exit.

1. Call `handle_comment` to strip comments.
2. Use `split(buffer, ";")` to get a list of semicolon-separated commands.
3. For each command:
   a. Call `check_background` to test for (and strip) a trailing `&`.
   b. Use `split(cmd, " \t\n")` to tokenize the command into words.
   c. If the first word is `NULL` (empty command), skip it.
   d. Handle built-in commands (see below).
   e. Otherwise, call `spawn_process`.
4. Free all allocated memory and return `true`.

**Built-in commands:**

* **`exit`** — if no background jobs are running, free memory and return `false`.  Otherwise print `Cannot exit with child processes still running`.
* **`status`** — call `proclist_print(plist)`.

#### `spawn_process(cmd, wait_for_child)` → `pid_t`

1. Call `fork()`.  On error, call `perror("fork")` and `exit(-1)`.
2. In the child: call `execvp(cmd[0], cmd)`.  On error, call `perror("execvp")` and `exit(-1)`.
3. In the parent: if `wait_for_child` is `true`, call `waitpid` to wait for the child.
4. Return the child's pid.

Note: `execvp` (not `execv`) searches your `PATH` automatically — `echo`, `ls`, `sleep`, etc. work without full paths.

#### `check_background(cmd)` → `bool`

Scan the command string from the end looking for `&`.  If found, replace it with a space and return `true`; otherwise return `false`.

#### `child_done(pid)` and `sweep_children(plist)`

`child_done` uses `waitpid` with `WNOHANG` to check (without blocking) whether a child has finished.

`sweep_children` iterates the proclist; any child that is done gets removed.  Build a separate list of PIDs to remove *before* modifying `plist` — do not remove nodes while iterating.

**Testing Stage 2:** run `./runtests.sh 2`.

---

## Stage 3: Background Jobs

Background job support requires `process_command_line`, `spawn_process`, and `main` to handle the `&` case plus a new `kill` built-in:

* **Background execution:** if `check_background` returns `true`, call `spawn_process(cmd, false)` (don't wait), then add the child to `plist` with `proclist_add`.
* **Sweeping:** call `sweep_children` each iteration of the main loop so finished background children are removed automatically.
* **`kill <pid>` built-in:** look up the pid in `plist` with `proclist_find`.  If found, call `kill(pid, SIGKILL)` and print `Killed process <pid>`.  If not found, print `kill: process <pid> not found`.  (The killed child is removed from `plist` by the next sweep.)
* **Exit blocking:** if `exit` is typed while jobs are running, print the warning and continue the loop instead of exiting.  If EOF arrives while jobs are running, print the warning once and wait for the jobs to finish before exiting (see Stage 2).

**Testing Stage 3:** run `./runtests.sh 3`, then run all the tests with `./runtests.sh`.

---

## Testing and debugging

The first step in debugging is to **fix any compiler errors or warnings** — they almost always point to a real bug.  The test script counts any compiler warning as a failure.

### Running the provided tests

Run the tests from a terminal in your codespace, in your repository's directory (the one containing `main.c`):

```bash
./runtests.sh       # run all tests
./runtests.sh 1     # Stage 1 tests only (linked list)
./runtests.sh 2     # Stage 2 tests only (basic shell)
./runtests.sh 3     # Stage 3 tests only (background jobs and kill)
./runtests.sh design  # program design checks only (globals and comments)
```

(If you get `Permission denied`, run `bash runtests.sh` instead, or run `chmod +x runtests.sh` once.)

The script runs `make` first; if your code doesn't compile, it shows the errors and stops.  It then prints `PASS` or `FAIL` for each test, and a summary line at the end.  Run the tests for each stage as soon as you finish it, and **run all of them before you submit**: changes for a later stage can break an earlier one.

| Stage | Test | Overview |
|---|---|---|
| 1 | `./listtest` | Unit tests of the six list functions |
| 1 | `valgrind ./listtest` | No memory errors or leaks in your list code |
| 2 | `cmd0` – `cmd2` | Foreground commands, empty commands, comments, invalid commands, `exit` |
| 2 | `cmd3` | Multiple `;`-separated commands on one line, ending at end of input (EOF) |
| 2 | no zombies | A finished foreground child is reaped with `waitpid` |
| 2 | `valgrind ./shell < cmd2.txt` | No memory errors or leaks running foreground commands |
| 3 | `cmd4` | Background jobs and `exit` blocking |
| 3 | `cmd5` | Sweeping a background job killed outside the shell, and `status` |
| 3 | `cmd6` | A finished background job is swept, so a later `exit` succeeds |
| 3 | `kill` built-in | `kill <pid>` kills a background job, which then disappears from `status` |
| 3 | `valgrind ./shell < cmd7.txt` | No memory errors or leaks running a background job |
| design | no global variables | No variables declared outside a function (including `static` ones) in `main.c` or `list.c` |
| design | function comments | Every function except `main` has a comment immediately before it, and no `TODO` comments remain |

### Understanding a failed test

The `cmdN` tests feed the commands in `cmdN.txt` to your shell (as if you typed them) and compare *everything* your shell prints with `cmdN_output.txt`, including every `prompt> `.  When one fails, the differences are shown: lines starting with `<` are your shell's output, and lines starting with `>` are the expected output.  To run one test yourself and see the full output:

```bash
./shell < cmd4.txt
```

Common causes of failures:

* **Output in the wrong order:** you forgot to `fflush(stdout)` after printing the prompt.
* **A test takes 15 seconds and then fails:** your shell got stuck (for example, it never exits, or it waits for a background job).  Each test is stopped after 15 seconds.
* **Missing `Cannot exit ...` messages or an `exit` that is refused:** check `sweep_children` and when you call it.

For a `valgrind` failure, run the same command yourself (e.g., `valgrind --leak-check=full ./shell < cmd2.txt`) to see the full report.

Passing the provided tests does not guarantee that your shell is correct.  You should also try commands of your own, both typed interactively and in input files of your own.

## Program design

Good program design dictates that you:

* **Use multiple functions:** Do not put all of your code in `main`.
* **Avoid duplicated code:** Factor repeated logic into helper functions.
* **Avoid global variables:** Use local variables and parameters instead.  (`runtests.sh` checks this.)
* **Fix all compiler warnings:** Warnings almost always indicate a real bug.
* **Include comments:** Each function (other than `main`) must have a short comment, immediately before it, explaining what it does.  Inline comments should explain non-obvious logic.  Replace the `TODO` comments in the starter code with your own comments.  (`runtests.sh` checks for function comments and leftover `TODO`s, but your instructor will also read your comments.)
* **Meaningful commit messages:** Short, descriptive summaries like `Implement proclist_add and proclist_remove` or `Fix sweep_children to not modify list while iterating`.


---

## Submission instructions
Please edit the section below to complete the genAI transparency statement and other feedback.

You should commit and push your updated files to your git repository. However, as noted above, do not wait until your entire program is working before you commit it to your git repository; you should commit your code each time you write and debug a piece of functionality.

## Instructor transparency

Claude code was used to enhance tests for this project and to identify and fix some inconsistencies in the project description as well as to improve documentation.

## Student documentation and feedback

### Sources / student transparency

Please edit this section to indicate any sources you used for help on this project, including human and non-human (e.g., genAI, stackoverflow) help.  For any genAI help, please include the name of the tool and the initial prompt used.


### Feedback

Please edit this section to provide feedback on this assignment.  What was good?  What could be improved?


## Instructor feedback

I'll complete this section with feedback and grading information from your beta and final submissions.
