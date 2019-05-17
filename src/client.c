#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "lib.h"

#define BUFSIZE 4096 // max number of bytes we can get at once

/**
 * Struct to hold all three pieces of a URL
 */
typedef struct urlinfo_t {
  char *hostname;
  char *port;
  char *path;
} urlinfo_t;

/**
 * Tokenize the given URL into hostname, path, and port.
 * Store hostname, path, and port in a urlinfo_t struct and return the struct.
*/
urlinfo_t *parse_url(char *url)
{
	urlinfo_t *urlinfo = malloc(sizeof(urlinfo_t));
	urlinfo->hostname = strdup(url);
	if (strstr(urlinfo->hostname, "https://")){
		urlinfo->hostname = urlinfo->hostname + 8;
	}
	if (strstr(urlinfo->hostname, "http://")){
		urlinfo->hostname = urlinfo->hostname + 7;
	}
	urlinfo->path = strchr(urlinfo->hostname, '/') + 1;
	*(urlinfo->path - 1) = '\0';
	if (!strchr(urlinfo->hostname, ':')) {
		urlinfo->port = "80";
	} else {
		urlinfo->port = strchr(urlinfo->hostname, ':') + 1 ;
		*(urlinfo->port - 1) = '\0';
	}
	return urlinfo;
}

/**
 * Constructs and sends an HTTP request
 * Return the value from the send() function.
*/
int send_request(int fd, char *hostname, char *port, char *path)
{
	const int max_request_size = 16384;
  	char request[max_request_size];
  	int rv;

  	int request_length = sprintf(request, 
		"GET /%s HTTP/1.1\n"
		"Host: %s:%s\n"
		"Connection: close\n\n",
		path, hostname, port
    );

    rv = send(fd, request, request_length, 0);
    if (rv < 0) {perror("send");}
    return rv;
}

int main(int argc, char *argv[])
{  
	int sockfd, numbytes;  
  	char buf[BUFSIZE];
  	int sr_return;

  	if (argc != 2) 
	{
    	fprintf(stderr,"usage: client HOSTNAME:PORT/PATH\n");
    	exit(1);
  	}

	struct urlinfo_t *url_info = parse_url(argv[1]);
	sockfd = get_socket(url_info->hostname, url_info->port);
	sr_return = send_request(sockfd, url_info->hostname, url_info->port, url_info->path);
	
	while ((numbytes = recv(sockfd, buf, BUFSIZE - 1, 0)) > 0) 
	{
		  printf("%s\n\n -- still more junk -- \n\n", buf);
	}

	free(url_info);
	close(sockfd);
  	return 0;
}
