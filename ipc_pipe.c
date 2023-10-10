#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    int pipe_fd[2]; // File descriptors for the pipe
    pid_t child_pid;

    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork a child process
    child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        // Child process
        close(pipe_fd[0]); // Close the unused read end of the pipe

        char message[] = "Hi Mom, I love you!";
        write(pipe_fd[1], message, sizeof(message));

        // Child also writes a response to the parent
        char response[] = "Thanks, sweetie!";
        write(pipe_fd[1], response, sizeof(response));

        close(pipe_fd[1]); // Close the write end of the pipe in the child

        exit(0);
    } else {
        // Parent process
        close(pipe_fd[1]); // Close the unused write end of the pipe

        char received_message[100];
        read(pipe_fd[0], received_message, sizeof(received_message));
        printf("Mother received a message from the child: %s\n", received_message);

        // Parent sends a response to the child
        char message[] = "You're welcome!";
        write(pipe_fd[0], message, sizeof(message));

        close(pipe_fd[0]); // Close the read end of the pipe in the parent

        wait(NULL); // Wait for the child to complete

        printf("Mother sent a message to the child.\n");
    }

    return 0;
}
