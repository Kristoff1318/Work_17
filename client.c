#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

void check_error(int errcheck) {
    if (errcheck == -1) {
        printf("Error #%d: %s", errcheck, strerror(errcheck));
    }
}

void handshake() {
    int errcheck;

    //Private FIFO Name
    char buffer[256];
    sprintf(buffer, "%d", getpid());

    //Client sends private FIFO name
    printf("[1] Sending private FIFO name...\n");
    int fd = open("WKP", O_WRONLY);
    check_error(fd);
    errcheck = write(fd, buffer, sizeof(buffer));
    check_error(errcheck);

    //Client receives server's message, unlinks private FIFO
    mkfifo(buffer, 0644);
    int ncon = open(buffer, O_RDONLY);
    char response[256];
    errcheck = read(ncon, response, sizeof(response));
    check_error(errcheck);
    unlink(buffer);
    printf("[3] Acknowledgement from server received: \"%s\"\n", response);

    //Client sends response to server
    char *confirm = "The client sends its acknowledgements!";
    errcheck = write(fd, confirm, 4*strlen(confirm));
    check_error(errcheck);

    close(fd);
    close(ncon);
}

static void sighandler(int signo) {
    if (signo == SIGINT) {
        exit(0);
    }
}

int main() {
    handshake();
    char line[256];
    int output;
    signal(SIGINT, sighandler);

    int pcon_1 = open("send_to_processor", O_WRONLY);
    int pcon_2 = open("receive_from_processor", O_RDONLY);

    while(1) {
        printf("Count caps: ");
        fgets(line, 256, stdin);
        write(pcon_1, line, 4*strlen(line));
        read(pcon_2, &output, 256);
        printf("%d\n", output);
    }
    return 0;
}
