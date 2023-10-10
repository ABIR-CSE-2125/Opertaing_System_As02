#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

// Define the message structure
struct msg_buffer {
    long msg_type;
    char msg_text[100];
};

int main() {
    // Create a unique key for the message queue
    key_t key = ftok("/tmp", 'M');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Create a message queue with read and write permissions
    int msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    pid_t child_pid;

    // Fork a child process
    child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        // Child process
        struct msg_buffer message;

        // Child receives a message from the parent
        msgrcv(msgid, &message, sizeof(message), 1, 0); // Message type 1
        printf("Child received a message from the parent: %s\n", message.msg_text);

        // Child sends a response to the parent
        strcpy(message.msg_text, "Hi Mom, I love you too!");
        message.msg_type = 2; // Message type 2 for responses
        msgsnd(msgid, &message, sizeof(message), 0);

        exit(0);
    } else {
        // Parent process
        struct msg_buffer message;

        // Parent sends a message to the child
        strcpy(message.msg_text, "Hi, sweetheart!");
        message.msg_type = 1; // Message type 1
        msgsnd(msgid, &message, sizeof(message), 0);

        // Parent waits for a response from the child
        wait(NULL);
        msgrcv(msgid, &message, sizeof(message), 2, 0); // Message type 2 for responses
        printf("Parent received a message from the child: %s\n", message.msg_text);

        // Remove the message queue
        msgctl(msgid, IPC_RMID, NULL);
    }

    return 0;
}
