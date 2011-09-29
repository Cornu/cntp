/* 
 * Simple NTP Client
 * Copyright (c) 2011, Hans-Harro Horn <h.h.horn@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char *port = "123";
    char *server = "pool.ntp.org";
    int timeout = 5;
    int sock;
    struct addrinfo *addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char buf[48];
    fd_set fdset;

    uint32_t *recv_time;
    time_t t;
    struct tm *now;

    //Parse Arguments
    int n;
    for(n=1; n<argc; n++) {
        if (argv[n][0] == '-') {
            switch(argv[n][1])
            {
                case 's':
                    server = argv[n+1];
                    break;
                case 'p':
                    port = argv[n+1];
                    break;
                case 't':
                    timeout = atoi(argv[n+1]);
                    break;
                case 'h':
                    printf("Usage: %s [Options]\n \
                            -h\thost default: %s)\n \
                            -p\tport (default: %s)\n \
                            -t\ttimeout (default: %is)\n \
                            -h\tthis help\n", argv[0], server, port, timeout);
                    exit(0);
                    break;
                default:
                    fprintf(stderr, "Unknown Parameter");
            }
        }
    }

    // Connect
    if (getaddrinfo(server, port, NULL, &addr) != 0) {
        fprintf(stderr, "Unknown Host !\n");
        exit(-1);
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        fprintf(stderr, "Error creating Socket !\n");
        exit(-1);
    }
    // Send
    memset(&buf, 0, 48);
    buf[0] = 0xE3;
    if (sendto(sock, buf, 48, 0, addr->ai_addr, addrlen) == -1) {
        fprintf(stderr, "Error sending\n");
        exit(-1);
    }
    // Timeout
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    struct timeval tout = {timeout, 0};
    if (select(sock+1, &fdset, NULL, NULL, &tout) == -1) {
        fprintf(stderr, "Timeout Error \n");
        exit(-1);
    }
    if (!FD_ISSET(sock, &fdset)) {
        fprintf(stderr, "Timeout ... %i s\n", timeout);
        exit(-1);
    }
    // Recieve
    if (recvfrom(sock, buf, 48, 0, addr->ai_addr, &addrlen) == -1) {
        fprintf(stderr, "Error recieving\n");
        exit(-1);
    }
    // Close
    freeaddrinfo(addr);
    close(sock);

    recv_time = (uint32_t*)&buf[32];
    t = (((*recv_time & 0xff000000) >> 24)|
         ((*recv_time & 0x00ff0000) >> 8)|
         ((*recv_time & 0x0000ff00) << 8)|
         ((*recv_time & 0x000000ff) << 24));
    t -= 2208988800U;
    now = localtime(&t);

    char str[80];
    strftime(str, sizeof(str), "%a %Y-%m-%d %H:%M:%S %Z", now);
    puts(str);

    return 0;
}
