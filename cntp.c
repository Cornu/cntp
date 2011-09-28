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

int main(int argc, char *argv[], char **env)
{
    char *port = "123";
    char *server = "pool.ntp.org";
    int sock;
    struct addrinfo *addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    char buf[48];

    if (argc == 2) {
        server = argv[1];
        //printf("Usage :\n\t%s <server> [port]\n", argv[0]);
    }
    if (argc > 2) {
        port = argv[2];
    }
    
    if (getaddrinfo(server, port, NULL, &addr) != 0) {
        printf("Unknown Host !\n");
        exit(-1);
    }
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1) {
        printf("Error creating Socket !\n");
        exit(-1);
    }
    // Send
    memset(&buf, 0, 48);
    buf[0] = 0xE3;
    if (sendto(sock, buf, 48, 0, addr->ai_addr, addrlen) == -1) {
        printf("Error sending\n");
        exit(-1);
    }
    // Recieve
    if (recvfrom(sock, buf, 48, 0, addr->ai_addr, &addrlen) == -1) {
        printf("Error recieving\n");
        exit(-1);
    }

    freeaddrinfo(addr);

    uint32_t *recv_time;
    time_t t;
    struct tm *now;
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
