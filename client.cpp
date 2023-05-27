#define _POSIX_C_SOURCE 200809L

//***************************************************************************
//
//  Garrett Jennings
//  z1927185
//  CSCI463 -- 1
//
//  I certify that this is my own work and where appropriate an extension 
//  of the starter code provided for the assignment.
//
//***************************************************************************


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <assert.h>
#include <iostream>
#include <fstream>

// #define DATA "Half a league, half a league . . ."

// constexpr const char data[] = "Lorem ipsum dolor sit amet,\n"
//     "consectetur adipiscing elit. Sed nunc elit, porttitor eget quam sit amet, laoreet venenatis sapien.\n"
//     "Maecenas malesuada feugiat dapibus. Nunc at aliquam risus, vel tincidunt nisi. Aenean eros lacus, semper sit amet euismod non, dignissim id augue.\n"
//     "Phasellus ac porta elit. Morbi lacus diam, imperdiet auctor ligula quis, bibendum iaculis risus. Sed nec neque eget ipsum dictum pharetra sed id libero. Suspendisse sed enim sagittis, posuere tellus a, lacinia urna.\n"
//     "Nulla sagittis et massa eget interdum. Aenean molestie accumsan consectetur. Ut eu nunc pellentesque, semper ante vel, efficitur metus. Proin a fringilla neque.";

/*
 * This program creates a socket and initiates a connection with the socket
 * given in the command line. One message is sent over the connection and
 * then the socket is closed, ending the connection. The form of the command
 * line is streamwrite hostname portnumber
 */

static void usage()
{
   perror("Usage: client [-s server-ip] server-port");
   perror(" [-s server-ip] Specify server IPv4 number in dotted-quad format");
   perror(" server-port Server Port number to which client must connect");
   exit(1);
}

/**
 * @brief Same as write(), but includes a loop to complete any partials
 * 
 */
static ssize_t safe_write(int fd, const char *buf, size_t len){
    size_t real_len = 0;
    while(len>0){
        ssize_t wlen = write(fd, buf, len);
        if (wlen == -1)
            return -1;
        
        // std::cout << "sent " << wlen << " bytes" << std::endl;
        len -= wlen;
        buf += wlen;
        real_len += wlen;
    }
    return real_len;
}
/**
 * @brief Prints response from the server
 * 
 * @param fd file descriptor to write to (socket fd)
 * @return int returns 0 on success, -1 on failure
 */
static int print_response(int fd){
    
    char buf[1024];

    int rval = 1;
    while(rval > 0){
        if ((rval = read(fd, buf, sizeof(buf)-1)) == -1){
            perror("reading stream message");
            return -1;
        }
        else if (rval > 0){
            /*ssize_t len = */
            write(fileno(stdout), buf, rval);
        }
    }

    write(fileno(stdout), "\0", 1);
    return 0;
}

/**
 * @brief Takes an IPv4 num and a port and streams stdin to a server
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[]){

    char *ipnum = (char *)"127.0.0.1";;

    int opt;
    while ((opt = getopt(argc, argv, "s:")) != -1)
    {
      switch(opt)
      {
        case 's': // Server IP
        {
            ipnum = optarg;
            break;
        }
      }
    }

    // std::cout << "ipnum:  " << ipnum << std::endl;

    if (optind >= argc)
		usage();	// missing port

    int sock;                           // FD for socket
    struct sockaddr_in server;          // Socket addr for server connection
    // struct hostent *hp;                 // for converting IP num into sockaddr_in
    
    /* Create Socket */
    sock = socket(AF_INET, SOCK_STREAM, 0); // IPv4, TCP, 
    if (sock < 0){
        perror("opening stream socket");
        exit(2);
    }

    /* Connect socket using name specified by cl */
    server.sin_family = AF_INET;
    // hp = gethostbyname(argv[1]); // IP_num
    if (inet_pton(AF_INET, ipnum, &server.sin_addr) <= 0){
        perror("invalid address/format");
        exit(3);
    }
    // memcpy(hp->h_addr, &server.sin_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[optind]));

    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("connecting stream socket");
        exit(2);
    }

    char buf[2048];
    ssize_t len;

    do {
        if ( (len = read(fileno(stdin), buf, sizeof(buf/*-1*/))) < 0)
            perror("reading stdin");

        if (safe_write(sock, buf, len) < 0)
            perror("writing on stream socket");
    } while (len != 0);

    // half close socket(read server perspective, write client perspective)
    shutdown(sock, SHUT_WR);

    print_response(sock);
    close(sock);

    return 0;
}