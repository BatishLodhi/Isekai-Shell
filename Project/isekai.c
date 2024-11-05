#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

// Function declarations
void handle_signal(int signo);
void execute_command(char *input);
void change_directory(char **args);
void list_processes();
void kill_process(char *pid);
void redirect_io(char **args);

void handle_redirection(char **args);
void handle_pipes(char *input);
void execute_normal_command(char **args);

void add_to_history(char *input);
void display_history();
void set_environment_variable(char **args);
void unset_environment_variable(char *var);
void job_control(char **args);
void bg_command(int job_id);
void fg_command(int job_id);
void display_welcome_message();
void display_help();

#define MAX_HISTORY 100
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    char command[256];
    int status; // 0 = stopped, 1 = running
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;
char *history[MAX_HISTORY];
int history_count = 0;

int main() {
    char input[1024];

    // Display welcome page
    display_welcome_message();

    signal(SIGINT, handle_signal);  // Handle Ctrl+C signal

    while (1) {
        printf("isekai> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        // Add command to history
        add_to_history(input);

        // Execute command
        execute_command(input);
    }

    return 0;
}

// Function to handle signals (Ctrl+C)
void handle_signal(int signo) {
    if (signo == SIGINT) {
        printf("\nCaught SIGINT (Ctrl+C). Use 'exit' to quit.\n");
    }
}

// Function to execute a command
void execute_command(char *input) {

    if (strstr(input, "|") != NULL) {
        handle_pipes(input);
        return;
    }
    
    char *args[64];
    char *token = strtok(input, " ");
    int i = 0;
    int background = 0;
    
    while (token != NULL) {
        // Detect if command ends with '&'
        if (strcmp(token, "&") == 0) {
            background = 1;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    for (int j = 0; args[j] != NULL; j++) {
        if (strcmp(args[j], ">") == 0 || strcmp(args[j], "<") == 0) {
            handle_redirection(args);
            return;
        }
    }

    // Handle built-in commands like cd, kill, help, etc.
    if (strcmp(args[0], "cd") == 0) {
        change_directory(args);
        return;
    } else if (strcmp(args[0], "ps") == 0) {
        list_processes();
        return;
    } else if (strcmp(args[0], "kill") == 0) {
        kill_process(args[1]);
        return;
    } else if (strcmp(args[0], "history") == 0) {
        display_history();
        return;
    } else if (strcmp(args[0], "export") == 0) {
        set_environment_variable(args);
        return;
    } else if (strcmp(args[0], "unset") == 0) {
        unset_environment_variable(args[1]);
        return;
    } else if (strcmp(args[0], "help") == 0) {
        display_help();
        return;
    } else if (strcmp(args[0], "fg") == 0 || strcmp(args[0], "bg") == 0) {
        job_control(args);
        return;
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    // Fork and execute external commands
    pid_t pid = fork();
    if (pid == 0) {
        // Redirect IO or handle pipes if necessary
        if (strchr(input, '>') || strchr(input, '<')) {
            redirect_io(args);
        } else if (strchr(input, '|')) {
            handle_pipes(input);
        } else {
            execvp(args[0], args);
        }
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        if (background) {
            // Add background job to the list
            jobs[job_count].pid = pid;
            strcpy(jobs[job_count].command, input);
            jobs[job_count].status = 1; // Running
            job_count++;
            printf("[%d] %d\n", job_count, pid);
        } else {
            waitpid(pid, NULL, 0); // Wait for the foreground process to finish
        }
    } else {
        perror("fork");
    }

}

// Function to execute standard (non-piped, non-redirected) commands
void execute_normal_command(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }
}

// Function to handle I/O redirection
void handle_redirection(char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);  // Redirect standard output to file
                close(fd);
                args[i] = NULL;
            } else if (strcmp(args[i], "<") == 0) {
                int fd = open(args[i + 1], O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);  // Redirect standard input from file
                close(fd);
                args[i] = NULL;
            }
        }
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }
}

// Change directory command
void change_directory(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "isekai: expected argument to 'cd'\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("chdir");
        }
    }
}

// List running processes using the ps command
void list_processes() {
    system("ps");
}

// Kill a process by PID
void kill_process(char *pid) {
    kill(atoi(pid), SIGKILL);
}

// Add command to history
void add_to_history(char *input) {
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(input);
        history_count++;
    }
}

// Display history of commands
void display_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

// Redirect input/output
void redirect_io(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } else if (strcmp(args[i], "<") == 0) {
            int fd = open(args[i + 1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
        i++;
    }
    execvp(args[0], args);
    perror("execvp");
    exit(1);
}

// Function to handle piping
void handle_pipes(char *input) {
    char *cmd1[64];
    char *cmd2[64];
    char *token;
    int i = 0;

    // Split command by "|"
    char *first_cmd = strtok(input, "|");
    char *second_cmd = strtok(NULL, "|");

    // Parse the first command
    token = strtok(first_cmd, " ");
    while (token != NULL) {
        cmd1[i++] = token;
        token = strtok(NULL, " ");
    }
    cmd1[i] = NULL;

    // Parse the second command
    i = 0;
    token = strtok(second_cmd, " ");
    while (token != NULL) {
        cmd2[i++] = token;
        token = strtok(NULL, " ");
    }
    cmd2[i] = NULL;

    // Set up pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork first command
    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp");
        exit(1);
    } else if (pid1 < 0) {
        perror("fork");
        exit(1);
    }

    // Fork second command
    pid_t pid2 = fork();
    if (pid2 == 0) {
        dup2(pipefd[0], STDIN_FILENO);  // Redirect stdin from pipe
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp");
        exit(1);
    } else if (pid2 < 0) {
        perror("fork");
        exit(1);
    }

    // Close pipes in parent process
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for both child processes
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Set environment variable
void set_environment_variable(char **args) {
    if (args[1] != NULL && args[2] != NULL) {
        setenv(args[1], args[2], 1);
    } else {
        fprintf(stderr, "isekai: expected argument for 'export'\n");
    }
}

// Unset environment variable
void unset_environment_variable(char *var) {
    unsetenv(var);
}

// Job control for fg/bg
void job_control(char **args) {
    if (args[1] == NULL) {
        printf("Usage: fg <job_id> or bg <job_id>\n");
        return;
    }

    int job_id = atoi(args[1]);  // Get job ID from command arguments

    if (strcmp(args[0], "fg") == 0) {
        fg_command(job_id);
    } else if (strcmp(args[0], "bg") == 0) {
        bg_command(job_id);
    } else {
        printf("Unknown job control command: %s\n", args[0]);
    }
}

// Resume a stopped job in the background
void bg_command(int job_id) {
    if (job_id > 0 && job_id <= job_count) {
        pid_t pid = jobs[job_id - 1].pid;
        if (jobs[job_id - 1].status == 0) {  // Check if the job is stopped
            kill(pid, SIGCONT);              // Send SIGCONT to resume process
            jobs[job_id - 1].status = 1;     // Mark as running
            printf("Resuming job [%d] %d in background\n", job_id, pid);
        } else {
            printf("Job [%d] is already running.\n", job_id);
        }
    } else {
        printf("Invalid job ID\n");
    }
}

// Bring a background job to the foreground
void fg_command(int job_id) {
    if (job_id > 0 && job_id <= job_count) {
        pid_t pid = jobs[job_id - 1].pid;
        if (jobs[job_id - 1].status == 1) {  
            printf("Bringing job [%d] %d to foreground\n", job_id, pid);
            waitpid(pid, NULL, 0);       
        } else {
            kill(pid, SIGCONT);           
            waitpid(pid, NULL, 0);          
        }
        jobs[job_id - 1].status = 0;
    } else {
        printf("Invalid job ID\n");
    }
}

// Display welcome message
void display_welcome_message() {
    system("clear");
    printf("==============================================\n");
    printf("          Welcome to Isekai Shell          \n");
    printf("==============================================\n");
    printf("      A Lightweight Customized Shell         \n");
    printf("==============================================\n");
    printf("    Made by 22CS019, 22CS041 and 22CS109     \n");
    printf("==============================================\n");
    printf("\n");
}

// Display available commands (help)
void display_help() {
    printf("\nAvailable Commands in Isekai Kernel:\n");
    printf("-----------------------------------\n");
    printf("cd <directory>   : Change directory\n");
    printf("ps               : List processes\n");
    printf("kill <pid>       : Kill a process\n");
    printf("history          : Display command history\n");
    printf("export <var> <val>: Set environment variable\n");
    printf("unset <var>      : Unset environment variable\n");
    printf("exit             : Exit the shell\n");
    printf("fg <job_id>      : Bring a background job to the foreground\n");
    printf("bg <job_id>      : Resume a stopped job in the background\n");
    printf("help             : Display this help menu\n");
    printf("I/O redirection  : Use >, < for redirection\n");
    printf("Piping           : Use | to pipe commands\n");
    printf("-----------------------------------\n\n");
}

