#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
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

    printf("Waiting for client connection...\n");

    //Server knows it can receive, unlinks WKP
    mkfifo("WKP", 0644);
    int fd = open("WKP", O_RDONLY);
    char pcon_name[256];
    errcheck = read(fd, pcon_name, 256);
    check_error(errcheck);
    unlink("WKP");
    printf("[2] Client message received. Private FIFO name: %s\n", pcon_name);

    //Server connects to client FIFO, sends acknowledgement
    int pcon = open(pcon_name, O_WRONLY);
    char msg[] = "The server sends its acknowledgements!";
    errcheck = write(pcon, msg, sizeof(msg));
    check_error(errcheck);

    //Server receives client's message: handshake complete
    char complete_handshake[256];
    errcheck = read(fd, complete_handshake, sizeof(complete_handshake));
    check_error(errcheck);
    close(fd);
    printf("[4] Acknowledgement from client received: \"%s\"\n", complete_handshake);
}

int count_caps(char * line) {
    char *place = line;
    int c = 0;

    while(*place != '\0') {
        if (*place >= 'A' && *place <= 'Z') c++;
        place++;
    }
    return c;
}

static void sighandler(int signo) {
    if (signo == SIGPIPE) {
        printf("Lost old client. New handshake ready!\n");
        unlink("WKP");
        handshake();
    }
    if (signo == SIGINT) {
        unlink("send_to_processor");
        unlink("receive_from_processor");
        unlink("WKP");
        exit(0);
    }
}

int main() {
    char line[256];
    int output;
    mkfifo("send_to_processor", 0644);
    mkfifo("receive_from_processor", 0644);
    signal(SIGINT, sighandler);
    signal(SIGPIPE, sighandler);
    handshake();

    int pcon_1 = open("send_to_processor", O_RDONLY);
    int pcon_2 = open("receive_from_processor", O_WRONLY);

    while(1) {
        read(pcon_1, line, 256);
        printf("Data received: %s\n", line);
        output = count_caps(line);
        write(pcon_2, &output, sizeof(output));
    }
    return 0;
}
