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
#include <sstream>
#include <arpa/inet.h>
#include <iostream>
#include <signal.h>

// static void usage()
// {
//    perror("Usage: server [-l listener-port]");
//    perror("[-l listener-port] (default = 0) Port number to which the server must listen");
//    exit(1);
// }

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
 * @brief Sums input from stream and writes sum and length back to socket
 * 
 * @param argc 
 * @param argv 
 */
main(int argc, char* argv[]){
    int sock;
    socklen_t length;
    struct sockaddr_in server;

    int port;

    int opt;
    while ((opt = getopt(argc, argv, "l:")) != -1)
    {
      switch(opt)
      {
        case 'l': // Port to listen
        {
            port = atoi(optarg);
            break;
        }
        default:
            port = 0;
      }
    }

    int msgsock;
    char buf[2048];
    int rval;

    signal(SIGPIPE, SIG_IGN);

    /* Create Socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0){
        perror("opening stream socket");
        exit(2);
    }

    /* Name socket using wildcards */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port); // htons(-l serverport)
    // server.sin_port = htons(9965);
    if (bind(sock, (sockaddr*)&server, sizeof(server))){
        perror("binding stream socket");
        exit(2);
    }

    /* Find out assigned port number and print it out */
    length = sizeof(server);
    if (getsockname(sock, (sockaddr*)&server, &length)) {
        perror("getting sock name");
        exit(2);
    }
    std::cout << "Socket has port #" << ntohs(server.sin_port) << std::endl;

    /* Start accepting connections */
    listen(sock, 5);
    do {
        struct sockaddr_in from;        // Used to display the address of connection peer
        socklen_t from_len = sizeof(from);
        msgsock = accept(sock, (struct sockaddr*)&from, &from_len);
        // msgsock = accept(sock, 0, 0);

        if (msgsock == -1)
            perror("accept");
        else {
            inet_ntop(from.sin_family, &from.sin_addr, buf, sizeof(buf));
            std::cout << "Accepted connection from ’" << buf << "’, port " << ntohs(from.sin_port) << std::endl;

            uint16_t sum = 0;   // Sum to write to client
            uint32_t by_len = 0;// Len to write to client

            do {
                if ((rval = read(msgsock, buf, sizeof(buf)/*-1*/)) < 0)
                    perror("reading stream message");
                if (rval == 0)
                    std::cout << "Ending connection\n" << std::endl;
                else{
                    // buf[rval] = 0;
                    // printf("-->%s\n", buf);  
                    // memset(buf, 0, sizeof(buf));
                    // buf[rval] = '\0';
                    // std::cout << "---- read " << rval << "bytes --->>>" << buf << "<<<" << std::endl;  
                
                    // Add the input as bytes
                    for (int i = 0; i < rval; i++){
                        sum += (uint8_t) buf[i];
                    }
                    by_len += rval;
                }
            } while (rval != 0);

            std::ostringstream os;
            os << "Sum: " << sum << " Len: " << by_len;
            std::string str = os.str();
            const char *ch = str.c_str();

            safe_write(msgsock, ch, strlen(ch));

    // //  one of these is likely to receive a SIGPIPE and terminate server
    //         rval = write(msgsock, "thank you!", 10);
    //         std::cout << "---- write " << rval << " bytes" << std::endl;
    //         rval = write(msgsock, "very much", 9);
    //         std::cout << "---- write " << rval << " bytes" << std::endl;
            close(msgsock);
        }
    } while (true);
}