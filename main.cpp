/*
 * * Socket Client Example, based on this video: https://www.youtube.com/watch?v=bdIiTxtMaKA&list=PL9IEJIKnBJjH_zM5LnovnoaKlXML5qh17
 * * Note: What's the point of using c++, if like 75% of the code ends up being C, i feel kinda scammed :v
 * * But oh well, at least im learning stuff
 * * PD: Install bettercomments for vscode and enable single line parsing, it really helps
 */
#include <sys/socket.h> // ? Basic socket headers
#include <sys/types.h>  // ? In order: Functions to create and manage sockets, network data types, inet protocols definition

#include <netinet/in.h> // ? Needed for sockaddr_in struct, htons()
#include <arpa/inet.h>  // ? Needed fot inet_pton()
#include <unistd.h>     // ? Needed for write()

#include <cerrno>  // ? Needed to access c's errno, error codes from the socket.h functions get stored there
#include <cstdarg> // ? Needed to pass a variable argument list to a function just like in c (don't quote me on that one)
#include <cstdio>  // ? In this case needed to print formated strings like c
#include <cstring>

#include <iostream>

#define PORT 80              // Port that the client sends data to
#define DATABUFFER_SIZE 4096 // Size of the buffer where the recieved data goes

void err_n_die(const char *fmt, ...); /** @param fmt Format for the cstring */

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        err_n_die("usage: %s <server address>", argv[0]);
    }

    int sockfd; /*// ? Will hold the file descriptor for our socket
                  // ? Mandatory reminders: Everything in linux is a damn file, even conections,
                  // ? so sending something to a socket is kinda like writing/reading to a file
                  // * A file descriptor is kinda like an id that points to an open file,
                  // * socket() function returns one of those*/

    // ? sockaddr_in struct defined in "netinet/in.h"
    // * Holds a socket address, wich means IP(or domain equivalent bla bla bla...), port(or dmn equ...) and other stuff like this
    struct sockaddr_in servaddr;

    /** socket() Creates a socket to write and read data to
     * @param domain: Not in a DNS way, domain as in protocol as in for where does the data drive.
     * *    We mean IPv4/6 (AF_INET/6), bluetooth, radio, VSOCK to conect with vmware guests and this kind of stuff
     * @param type: Essentially, TCP or UDP (SOCK_STREAM, SOCK_DGRAM), but also has other options
     * @param protocol: Usually for each type+domain there is only one protocol. 0 is automatic detection.
     * * see /etc/protocols. Example: for the AF_INET with SOCK_STREAM there is only the tcp protocol.
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        err_n_die("Error creating the socket");
    }

    // Cleans memory that holds our epic servaddr struct, because garbage data will mess this up
    bzero(&servaddr, sizeof(sockaddr_in));
    servaddr.sin_family = AF_INET;   // Set this address domain, AF_INET because well, we are tryinging to conect to a AF_INET socket duh
    servaddr.sin_port = htons(PORT); /* Same but with the port, htons(host to network short),
                                        makes sure the socket we'll conecto to and our computer don't mess up because one is big endian and the other not*/

    // Converts an ip in str format into a binary representation of that, "1.2.3.4" => [1,2,3,4].
    // Needs (domain/address format,ip str, where to store it)
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        err_n_die("Error with address %s", argv[1]);
    }

    // Connects to a server, shoves the recieved data into the socket we created before (using the file descriptor, aka sockfd, aka first param)
    // Param 2: Is the address, we have to cast it to a "sockaddr *", our "sockaddr_in" can ve casted with no problems,
    // they are the same kinda, but sockaddr holds less stuff. Both hold the server address
    // Param 3: byte size of param 2

    if (connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        err_n_die("Couldn't connect!");
    }

    //Char array that holds the data we want to send
    char data_to_send[DATABUFFER_SIZE];
    // Prints to a pointer that has a cstr, in this case the text is a typical get request

    sprintf(data_to_send, "GET / HTTP/1.1\r\n\r\n");
    int data_length = strlen(data_to_send); // Store the length of whatever we want to send

    // Write data to socket using c write function. write() writes data to a file descriptor
    if ((write(sockfd, data_to_send, data_length)) < 0)
    {
        err_n_die("could not write to socket");
    }

    // Char array that will hold the server response, remeber to set it to 0
    char recieved_data_buffer[DATABUFFER_SIZE];
    memset(recieved_data_buffer, 0, DATABUFFER_SIZE);
    // Read it
    if (read(sockfd, recieved_data_buffer, DATABUFFER_SIZE) > 0)
    {
        std::cout << recieved_data_buffer;
    }else{
        err_n_die("Error reading buffer");
    }
}

void err_n_die(const char *fmt, ...)
{

    int error_number; // Error number, well set to whatever "errno" is
    error_number = errno;

    va_list arguments_pointer; // Pointer to the arguments array

    // Print Error to standard out
    va_start(arguments_pointer, fmt);         // Needed to make the args array accessible, //**  @params: args pointer, last fixed argument
    vfprintf(stdout, fmt, arguments_pointer); // Pointer to an c output stream (internalli a FILE pointer), format for the string (c again), args pointer
    fprintf(stdout, "\n");
    fflush(stdout);

    if (errno != 0)
    {
        fprintf(stdout, "(errno = %d) : %s\n", error_number, strerror(error_number));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(arguments_pointer);
    exit(1);
}