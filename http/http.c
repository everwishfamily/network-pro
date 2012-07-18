/**
 * @file http.c
 * @brief 
 * @author dwwang
 * @version 1.0.0
 * @date 2012-07-17
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "log.h"

#define HTTP_REQUEST_URL_FORMAT                  "http://"
#define HTTP_REQUEST_DEFAULT_FILE                "index.html"

/**
 * @brief parse request url, get domain, 
 * port and will save file name
 *
 * @param url http request url
 * @param domain 
 * @param file
 * @param port
 *
 * @return if return 0, it success; otherwise, it failed
 */
int parse_http_url(
		char *url, 
		char *domain, 
		char *file, 
		int *port)
{
	if (!url || !domain || !file || !port)
	{
		test_log(MSG_ERROR, 0, stderr, 
				"%s param is NULL!\n", __func__);
		return -1;
	}

	memset(domain, 0, sizeof(domain));
	memset(file, 0, sizeof(file));
	*port = 0;

	// check request url head
	size_t size = strlen(HTTP_REQUEST_URL_FORMAT);
	if (strlen(url) <= size || 
			strncmp(url, HTTP_REQUEST_URL_FORMAT, size) != 0)
	{
		test_log(MSG_ERROR, 0, stderr, "request url not fit http format!\n");
		return -1;
	}

	// fix error input url format
	size_t l = strlen(url);
	if (url[l-1] == '/') url[(l--)-1] = '\0';

	// p point next of "http://"
	char *p = url + size;

	// get port, set it 80 if no specify port 
	char *p2 = NULL;
	if ((p2 = (strchr(p, ':')))) *port = atoi(p2 + 1);
	if (*port == 0) *port = 80;
	test_log(MSG_INFO, 0, stdout, "port: %d\n", *port);

	char *p1 = NULL;
	p1 = strchr(p, '/');

	// calc domain size and copy it
	size_t ds = p2 ? p2 - p : (p1 ? p1 : (url+l)) - p;
	strncpy(domain, p, ds);
	test_log(MSG_INFO, 0, stdout, "domain: %s\n", domain);

	// get saved file name
	strcpy(file, p1 ? p1 + 1 : HTTP_REQUEST_DEFAULT_FILE);
	test_log(MSG_INFO, 0, stdout, "file: %s\n", file);

	return 0;
}

/**
 * @brief http main
 *
 * @param argc
 * @param argv
 *
 * @return 
 */
int main(int argc, char **argv)
{
	// check argc
	if (argc < 2)
	{
		test_log(MSG_ERROR, 0, stderr, "please input request url!\n");
		return -1;
	}

	// parse domain, port and will save file name
	char domain[256];
	char file[256];
	int port;

	int ret = parse_http_url(argv[1], domain, file, &port);
	if (ret != 0) 
		return -1;
	
	int sockfd;
	struct sockaddr_in server_addr;
	struct hostent *host;

	// get host struct
	if (!(host = gethostbyname(domain)))
	{
		test_log(MSG_ERROR, errno, stderr, "get host name failed!\n");
		return -1;
	}

	// get socket fd
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		test_log(MSG_ERROR, 0, stderr, "socket failed!\n");
		return -1;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);

	if(connect(sockfd, (struct sockaddr *)(&server_addr), 
				sizeof(struct sockaddr)) == -1)
	{
		test_log(MSG_ERROR, 0, stderr, "connect failed!\n");
		return -1;
	}

	// fill request url
	char request[1024];
	memset(request, 0, sizeof(request));
	sprintf(request, 
			"GET /%s HTTP/1.1\r\n"
			"Accept: */*\r\n"
			"Accept-Language: zh-cn\r\n"
			"User-Agent: Mozilla/4.0 (compatible;MSIE 5.01;Windows NT 5.0)\r\n"
			"Host: %s:%d\r\n"
			"Connection: Close\r\n\r\n", 
			file, domain, port);
	test_log(MSG_INFO, 0, stdout, "request: %s\n", request);

	// send request
	int send = 0;
	size_t totalsend = 0;
	size_t nbytes = strlen(request);
	while(totalsend < nbytes) 
	{
		send = write(sockfd, request + totalsend, nbytes - totalsend);
		if(send == -1) 
		{
			test_log(MSG_ERROR, errno, stderr, "write socket failed!\n");
			return -1;
		}
		totalsend += send;
	}

	// open local file
	char *pl = NULL;
	pl = strrchr(file, '/');
	pl = pl ? pl + 1 : file;
	test_log(MSG_INFO, 0, stdout, "local file: %s\n", pl);

	int fp = open(pl, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fp == -1)
	{
		test_log(MSG_ERROR, errno, stderr, "open %s failed!\n", file);
		return -1;
	}

	// read response buf
	int flag = 0;
	char *pb = NULL;
	char *p = NULL;
	ssize_t rb = 0;
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	while ((rb = (read(sockfd, buf, sizeof(buf)))) > 0)	
	{
		// skip response header
		p = buf;
		pb = NULL;
		if (!flag && (p = strstr(p, "\r\n\r\n")))
		{
			test_log(MSG_INFO, 0, stdout, "response head: %s\n", buf);
			pb = p;	
			flag = 1;
		}

		// get real data pointer and data size
		pb = pb ? pb + 4 : buf;
		rb -= pb - buf;

		// write into file
		if (rb && write(fp, pb, rb) != rb)
		{
			test_log(MSG_ERROR, errno, stderr, "fwrite %s failed!\n", file);
			break;
		}
		memset(buf, 0, sizeof(buf));
	}

	// close all fd
	close(fp);
	close(sockfd);

	return 0;
}
