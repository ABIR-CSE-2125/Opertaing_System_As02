#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024  // Size of the shared memory segment

// Structure to hold the note
struct Note {
    char message[SHM_SIZE];
    int read_by_child;
};

int main() {
    key_t key = ftok("shared_memory_key", 65); // Generate a unique key

    // Create or access the shared memory segment
    int shmid = shmget(key, sizeof(struct Note), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment to the process
    struct Note *shared_note = (struct Note *)shmat(shmid, NULL, 0);
    if (shared_note == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize the shared note
    memset(shared_note->message, 0, SHM_SIZE);
    shared_note->read_by_child = 0;

    // Parent (Mother) writes a note
    printf("Mother: Enter a note for your child: ");
    fgets(shared_note->message, SHM_SIZE, stdin);

    // Mark the note as unread by the child
    shared_note->read_by_child = 0;

    // Detach the shared memory segment from the parent process
    shmdt(shared_note);

    // Fork a child process
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(1);
    } else if (child_pid == 0) {
        // Child process (Child)
        sleep(2); // Wait for a moment to ensure that the parent writes the note first

        // Attach the shared memory segment to the child process
        struct Note *child_shared_note = (struct Note *)shmat(shmid, NULL, 0);
        if (child_shared_note == (void *)-1) {
            perror("shmat");
            exit(1);
        }

        // Display the note from the mother
        printf("Child: Message from Mother: %s", child_shared_note->message);

        // Mark the note as read by the child
        child_shared_note->read_by_child = 1;

        // Detach the shared memory segment from the child process
        shmdt(child_shared_note);
    } else {
        // Parent process (Mother)
        wait(NULL); // Wait for the child to finish

        // Attach the shared memory segment to the parent process
        struct Note *parent_shared_note = (struct Note *)shmat(shmid, NULL, 0);
        if (parent_shared_note == (void *)-1) {
            perror("shmat");
            exit(1);
        }

        // Check if the child has read the note
        if (parent_shared_note->read_by_child) {
            printf("Mother: The child has read the note.\n");
        } else {
            printf("Mother: The child has not read the note yet.\n");
        }

        // Detach and remove the shared memory segment
        shmdt(parent_shared_note);
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
