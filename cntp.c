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

#define NTP_OFFSET 2208988800UL		// Offset 1900-1970
typedef struct {
		char dummy[40];
		unsigned long rx_timestamp;
} ntp_struct;

static uint8_t NTP[]= {
    0xd9,0x00,0x0a,0xfa,0x00,0x00,0x00,0x00,
    0x00,0x01,0x04,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0xc7,0xd6,0xac,0x72,0x08,0x00,0x00,0x00
};

int main(int argc, char *argv[])
{
    char *port = "123";
    char *server = "pool.ntp.org";
    int timeout = 5;
    char *timestring = "%a %Y-%m-%d %H:%M:%S %Z";
    int set = 0;
    int sock;
    struct addrinfo *addr;
    socklen_t addrlen = sizeof(struct sockaddr_storage);
    fd_set fdset;
    ntp_struct ntp;

    time_t t;
    struct tm *now;

    //Parse Arguments
    int n;
    for(n=1; n<argc; n++) {
        if (argv[n][0] == '-') {
            switch(argv[n][1])
            {
                case 'h':
                    server = argv[n+1];
                    break;
                case 'p':
                    port = argv[n+1];
                    break;
                case 't':
                    timeout = atoi(argv[n+1]);
                    break;
                case 'f':
                    timestring = argv[n+1];
                    break;
                case 's':
                    set = 1;
                    break;
                case '-':
                    printf("Usage: %s [Options]\n"
                           "\t-h\thost (default: %s)\n"
                           "\t-p\tport (default: %s)\n"
                           "\t-t\ttimeout (default: %is)\n"
                           "\t-f\tformat (default: %s)\n"
                           "\t-s\tset time with 'date'\n"
                           "\t--help\tthis help\n", argv[0], server, port, timeout, timestring);
                    exit(0);
                    break;
                default:
                    fprintf(stderr, "Unknown Parameter\n");
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
    if (sendto(sock, NTP, sizeof(NTP), 0, addr->ai_addr, addrlen) == -1) {
    	struct sockaddr_in *in;
    	in = (struct sockaddr_in*)addr->ai_addr;
        fprintf(stderr, "Network unreachable (%s)\n", inet_ntoa(in->sin_addr));
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
    if (recvfrom(sock, &ntp, sizeof(ntp), 0, addr->ai_addr, &addrlen) == -1) {
        fprintf(stderr, "Error recieving\n");
        exit(-1);
    }
    // Close
    freeaddrinfo(addr);
    close(sock);

	if (htonl(42) == 42) {
		// Big endian
		t = ntp.rx_timestamp - NTP_OFFSET;
	} else {
		// Little endian
		t = htonl(ntp.rx_timestamp) - NTP_OFFSET;
	} 
    now = localtime(&t);

    char str[80];
        
    strftime(str, sizeof(str), "%m%d%H%M%Y.%S", now);
    if (set) {
		char cmd[20];
		sprintf(cmd, "date %s", str);
    	system(cmd);
    } else {
    	strftime(str, sizeof(str), timestring, now);
    	puts(str);
    }

    return 0;
}

