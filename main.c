
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

// Function to set resource limits (CPU time and data segment memory)
void set_limits(int cpu_time_limit, int memory_limit) {
    struct rlimit limit;

    // Set CPU time limit (in seconds)
    limit.rlim_cur = cpu_time_limit;
    limit.rlim_max = cpu_time_limit;
    if (setrlimit(RLIMIT_CPU, &limit) == -1) {
        perror("Error setting CPU limit");
        exit(EXIT_FAILURE);
    }

    // Set data segment memory limit (in bytes)
    limit.rlim_cur = memory_limit;
    limit.rlim_max = memory_limit;
    if (setrlimit(RLIMIT_DATA, &limit) == -1) {
        perror("Error setting data segment memory limit");
        exit(EXIT_FAILURE);
    }

    // Optionally, set the stack size limit (in bytes)
    limit.rlim_cur = 8 * 1024 * 1024;  // 8 MB
    limit.rlim_max = 8 * 1024 * 1024;
    if (setrlimit(RLIMIT_STACK, &limit) == -1) {
        perror("Error setting stack size limit");
        exit(EXIT_FAILURE);
    }
}

// Function to simulate a VM by forking a process and setting resource limits
void run_vm(const char *program, char *const args[], int cpu_time_limit, int memory_limit) {
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process (VM)
        printf("VM started (PID: %d)\n", getpid());

        // Set CPU and memory limits for this "VM"
        set_limits(cpu_time_limit, memory_limit);

        // Execute the program (this simulates running a task in the VM)
        execvp(program, args);

        // If execvp fails
        perror("Error executing program in VM");
        exit(EXIT_FAILURE);
    } else {
        // Parent process (host) waits for the child (VM) to finish
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("VM (PID: %d) exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("VM (PID: %d) was killed by signal %d\n", pid, WTERMSIG(status));
        }
    }
}

int main() {
    // Example 1: Run a program with CPU and memory limits
    char *args1[] = {"/usr/bin/bc", "-q", NULL}; // Running 'bc' calculator in VM
    run_vm("/usr/bin/bc", args1, 2, 100 * 1024 * 1024); // Limit to 2 seconds CPU time, 100 MB data segment memory

    // Example 2: Run another program
    char *args2[] = {"/usr/bin/yes", NULL}; // Running 'yes' command in VM (this runs indefinitely)
    run_vm("/usr/bin/yes", args2, 1, 50 * 1024 * 1024); // Limit to 1 second CPU time, 50 MB data segment memory

    return 0;
}
