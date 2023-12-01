#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define EXIT_MESSAGE_WITH_HELP "Thank you for using the chat program, goodbye!\nUsage for chat program: chat -p <port> -s <other_ip_address>\n-h to get help\n"
#define PORT_PARAMETER "-p"
#define OTHER_IP_PARAMETER "-s"
#define HELP_PARAMETER "-h"

#define DEFAULT_SERVER_PORT "3360"
#define MAX_LISTEN 5
#define BUFFER_SIZE 256
#define MAX_STRING_LENGTH 140

void packi16(unsigned char *buf, unsigned int i) {
    *buf++ = i>>8; *buf++ = i;
}

int unpacki16(unsigned char *buf) {
    unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
    int i;
    if (i2 <= 0x7fffu) {
        i = i2;
    }
    else {
        i = -1 - (unsigned int)(0xffffu - i2);
    }
    return i;
}

bool checkPort(const char* port) {
    for(int i = 0; i < strlen(port); i++) {
        if(!isdigit(port[i])) {
            printf("You may only use digits 0-9 in the port...\n");
            printf(EXIT_MESSAGE_WITH_HELP);
            return false;
        }
    }
    return true;
}

bool checkIP(const char* ip) {
    for(int i = 0; i < strlen(ip); i++) {
        if(!isdigit(ip[i]) && ip[i] != '.') {
            printf("You may only use digits 0-9 and '.' in the IP...");
            printf(EXIT_MESSAGE_WITH_HELP);
            return false;
        }
    }
    return true;
}

void server() {

    int sock, new_sock;
    struct addrinfo *servinfo, hints, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    bool receive_mode = true; // server is initially receiving.

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rv = getaddrinfo(NULL, DEFAULT_SERVER_PORT, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "server: getaddrinfo error: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sock == -1) {
            perror("server: socket error\n");
            continue;
        }

        if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("server: setsockopt\n");
            exit(1);
        }

        if(bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("server: bind error\n");
            continue;
        }

        break;

    }

    freeaddrinfo(servinfo);

    if(p == NULL) {
        perror("server: failed to bind");
        exit(1);
    }

    if(listen(sock, MAX_LISTEN) == -1) {
        perror("server: listen\n");
        exit(1);
    }

    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if(hostname == -1) {
        perror("server: could not resolve hostname\n");
        exit(1);
    }

    host_entry = gethostbyname(hostbuffer);
    if(host_entry == NULL) {
        perror("server: gethostbyname error\n");
        exit(1);
    }

    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));

    printf("Welcome to chat!\nWaiting for a connection on %s port %s\n", IPbuffer, DEFAULT_SERVER_PORT);

    while(1) {
        sin_size = sizeof their_addr;
        new_sock = accept(sock, (struct sockaddr*)&their_addr, &sin_size);
        if(new_sock == -1){
            perror("server: accept\n");
            continue;
        }

        printf("Found a friend! You receive first.\n");

        while (1) {

            if(receive_mode) {

                char buf[BUFFER_SIZE];
                int numbytes = recv(new_sock, buf, BUFFER_SIZE - 1, 0);

                if (numbytes == -1) {
                    perror("client: recv error");
                    exit(1);
                }

                buf[numbytes] = '\0';

                uint16_t recovered_version = ntohs(unpacki16(buf));
                uint16_t recovered_string_length = ntohs(unpacki16(buf + 2));

                printf("Friend: %s\n", buf + 4);

            } else {

                char packet[145];
                char buffer[BUFFER_SIZE];

                uint16_t packet_version = 457;

                printf("You: ");
                fgets(buffer, BUFFER_SIZE, stdin);
                uint16_t string_length = strlen(buffer) - 1;

                if(string_length > MAX_STRING_LENGTH) {
                    printf("Error: Input too long.\n");
                    continue;
                }

                uint16_t new_packet_version = htons(packet_version);
                uint16_t new_string_length  = htons(string_length);

                packi16(packet, new_packet_version);
                packi16(packet + 2, new_string_length);
                memmove(packet + 4, buffer, string_length);

                int send_status = send(new_sock, packet, string_length + 4, 0);
                if(send_status == -1) {
                    perror("server: send\n");
                }

            }

            receive_mode = !receive_mode;

        }

    }


}

void client(const char* clientIP, const char* clientPort) {

    int sock, numbytes;
    char buf[BUFFER_SIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    bool receive_mode = false; // client is initially broadcasting.

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(clientIP, clientPort, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "gettaddrinfo error: %s\n", gai_strerror(rv));
        exit(1);
    }

    printf("Connecting to server... ");

    for(p = servinfo; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sock == -1) {
            perror("client: socket error\n");
            continue;
        }

        if(connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("client: connect error\n");
            continue;
        }

        break;

    }

    if (p == NULL) {
        perror("client: failed to connect\n");
        exit(1);
    }

    printf("Connected!\n");

    freeaddrinfo(servinfo);

    while(1) {

        if (receive_mode) {

            numbytes = recv(sock, buf, BUFFER_SIZE - 1, 0);

            if (numbytes == -1) {
                perror("client: recv error");
                exit(1);
            }

            buf[numbytes] = '\0';

            uint16_t recovered_version = ntohs(unpacki16(buf));
            uint16_t recovered_string_length = ntohs(unpacki16(buf + 2));

            printf("Friend: %s\n", buf + 4);

        } else {

            char packet[145];
            char buffer[BUFFER_SIZE];

            uint16_t packet_version = 457;

            printf("You: ");
            fgets(buffer, BUFFER_SIZE, stdin);
            uint16_t string_length = strlen(buffer) - 1;
            if(string_length > MAX_STRING_LENGTH) {
                printf("Error: Input too long.\n");
                continue;
            }

            uint16_t new_packet_version = htons(packet_version);
            uint16_t new_string_length  = htons(string_length);

            packi16(packet, new_packet_version);
            packi16(packet + 2, new_string_length);
            memmove(packet + 4, buffer, string_length);

            int send_status = send(sock, packet, string_length + 4, 0);
            if(send_status == -1) {
                perror("server: send\n");
            }

        }

        receive_mode = !receive_mode;

    }

}

int main (int argc, char* argv[]) {

    bool serverMode = false;

    char* clientPort;
    char* clientIP;

    if (argc == 1) {
        serverMode = true;
    }

    if(!serverMode && argc < 4) {
        printf("Invalid amount of client arguments.\n");
        printf(EXIT_MESSAGE_WITH_HELP);
        exit(1);
    }

    if(!serverMode) {

        for(int i = 1; i < argc; i++) {
            if(strcmp(argv[i], PORT_PARAMETER) == 0) {
                clientPort = argv[i + 1];
            }
            if(strcmp(argv[i], OTHER_IP_PARAMETER) == 0) {
                clientIP = argv[i + 1];
            }
            if(strcmp(argv[i], HELP_PARAMETER) == 0) {
                printf(EXIT_MESSAGE_WITH_HELP);
                return 0;
            }
        }

        if(!checkPort(clientPort)) {
            exit(1);
        }
        if(!checkIP(clientIP)) {
            exit(1);
        }

    }

    if(serverMode) {
        server();
    } else {
        client(clientIP, clientPort);
    }


}


