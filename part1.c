#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 256

// Structure for each process node
typedef struct Process {
    char parent[MAX_NAME_LENGTH];
    char name[MAX_NAME_LENGTH];
    int priority;
    int memory;
    struct Process *left;
    struct Process *right;
} Process;

// Function to create a new process node
Process* createProcess(char parent[], char name[], int priority, int memory) {
    Process* newNode = (Process*)malloc(sizeof(Process));
    if (newNode == NULL) {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->parent, parent);
    strcpy(newNode->name, name);
    newNode->priority = priority;
    newNode->memory = memory;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

// Function to insert a process into the process tree
Process* insertProcess(Process* root, Process* newNode) {
    if (root == NULL)
        return newNode;

    // Traverse left if name is smaller
    if (strcmp(newNode->parent, root->name) < 0)
        root->left = insertProcess(root->left, newNode);
    // Traverse right if name is greater
    else if (strcmp(newNode->parent, root->name) > 0)
        root->right = insertProcess(root->right, newNode);

    return root;
}

// Function to display the process tree recursively
void displayProcessTree(Process* root) {
    if (root != NULL) {
        printf("Parent: %s, Name: %s, Priority: %d, Memory: %d MB\n", root->parent, root->name, root->priority, root->memory);
        displayProcessTree(root->left);
        displayProcessTree(root->right);
    }
}

int main() {
    FILE *fp;
    char parent[MAX_NAME_LENGTH], name[MAX_NAME_LENGTH];
    int priority, memory;
    Process* root = NULL;

    // Open the file for reading
    fp = fopen("processes_tree.txt", "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read from the file and build the process tree
    while (fscanf(fp, "%[^,],%[^,],%d,%d\n", parent, name, &priority, &memory) == 4) {
        Process* newNode = createProcess(parent, name, priority, memory);
        root = insertProcess(root, newNode);
    }

    // Close the file
    fclose(fp);

    // Display the contents of the process tree
    printf("Process Tree:\n");
    displayProcessTree(root);

    return 0;
}
