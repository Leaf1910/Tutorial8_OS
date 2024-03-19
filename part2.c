#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_NAME_LENGTH 256
#define MEMORY 1024

typedef struct Process {
    char name[MAX_NAME_LENGTH];
    int priority;
    int pid;
    int address;
    int memory;
    int runtime;
    int suspended;
    struct Process* next; // Added next pointer
} proc;

// Function to execute a process
void execute_process(proc *p, int avail_mem[]) {
    printf("Executing Process: %s, Priority: %d, Memory: %d MB, Runtime: %d seconds\n",
           p->name, p->priority, p->memory, p->runtime);
    
    // Fork and execute the process binary
    p->pid = fork();
    if (p->pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (p->pid == 0) { // Child process
        // Execute the process binary
        execl(p->name, p->name, NULL);
        // If execl fails, print error message and exit
        perror("execl");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Mark the memory needed as used
        for (int i = p->address; i < p->address + p->memory; i++) {
            avail_mem[i] = 1;
        }
    }
}

int main() {
    FILE *fp;
    char name[MAX_NAME_LENGTH];
    int priority, memory, runtime;
    int avail_mem[MEMORY] = {0};
    
    // Create FIFO queues
    proc *priority_queue = NULL;
    proc *secondary_queue = NULL;

    // Open the file for reading
    fp = fopen("processes_q2.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read from the file and populate queues
    while (fscanf(fp, "%[^,],%d,%d,%d\n", name, &priority, &memory, &runtime) == 4) {
        proc *new_process = (proc *)malloc(sizeof(proc));
        if (new_process == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        strcpy(new_process->name, name);
        new_process->priority = priority;
        new_process->pid = 0;
        new_process->address = 0;
        new_process->memory = memory;
        new_process->runtime = runtime;
        new_process->suspended = 0;
        new_process->next = NULL; // Initialize next pointer
        
        if (priority == 0)
            execute_process(new_process, avail_mem);
        else {
            // Add to the secondary queue
            new_process->next = secondary_queue;
            secondary_queue = new_process;
        }
    }

    // Close the file
    fclose(fp);

    // Wait for processes in the priority queue
    for (proc *p = priority_queue; p != NULL; p = p->next) {
        waitpid(p->pid, NULL, 0);
        // Free the memory used by the process
        for (int i = p->address; i < p->address + p->memory; i++) {
            avail_mem[i] = 0;
        }
        free(p);
    }

    // Iterate through all processes in the secondary queue
    while (secondary_queue != NULL) {
        proc *current = secondary_queue;
        secondary_queue = secondary_queue->next;

        if (current->runtime <= 1) {
            execute_process(current, avail_mem);
            sleep(current->runtime);
            kill(current->pid, SIGINT);
            waitpid(current->pid, NULL, 0);
            // Free the memory used by the process
            for (int i = current->address; i < current->address + current->memory; i++) {
                avail_mem[i] = 0;
            }
            free(current);
        } else {
            if (current->suspended && current->pid != 0) {
                kill(current->pid, SIGCONT);
            } else {
                execute_process(current, avail_mem);
                current->pid = fork();
                if (current->pid == 0) {
                    sleep(1);
                    kill(getppid(), SIGTSTP);
                    exit(0);
                } else {
                    current->suspended = 1;
                }
            }
            sleep(1);
            kill(current->pid, SIGTSTP);
            current->runtime--;
            // Add the process back to the secondary queue
            current->next = secondary_queue;
            secondary_queue = current;
        }
    }

    return 0;
}
