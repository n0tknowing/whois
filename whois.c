/*
 * whois - simple whois command
 *
 * whois has been placed in the public domain.
 * for details, see https://creativecommons.org/publicdomain/zero/1.0
 */

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static void whois_query(const char *server, const char *port, int family,
			const char *tldreq)
{
	char recvbuf[2048];
	char *sendbuf;
	size_t tldlen = strlen(tldreq);
	ssize_t len;
	int sockfd, rc;
	struct addrinfo hint, *res;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = family;
	hint.ai_socktype = SOCK_STREAM;

	rc = getaddrinfo(server, port, &hint, &res);
	if (rc < 0) {
		const char *errmsg;
		errmsg = (rc == EAI_SYSTEM) ? strerror(errno) : gai_strerror(rc);
		fprintf(stderr, "getaddrinfo(): %s\n", errmsg);
		exit(1);
	}

	sockfd = socket(res->ai_family, res->ai_socktype, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket(): %s\n", strerror(errno));
		exit(1);
	}

	rc = connect(sockfd, res->ai_addr, res->ai_addrlen);
	if (rc < 0) {
		fprintf(stderr, "connect(): %s\n", strerror(errno));
		exit(1);
	}

	sendbuf = calloc(tldlen + 3, sizeof(char));
	if (sendbuf == NULL) {
		fprintf(stderr, "malloc(): %s\n", strerror(errno));
		exit(1);
	}

	memcpy(sendbuf, tldreq, tldlen);
	sendbuf[tldlen] = '\r';
	sendbuf[tldlen+1] = '\n';

	len = send(sockfd, sendbuf, strlen(sendbuf), 0);
	if (len < 1) {
		const char *errmsg;
		errmsg = (len < 0) ? strerror(errno) : "Empty";
		fprintf(stderr, "send(): %s\n", errmsg);
		exit(1);
	}

	while (1) {
		len = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
		if (len < 1) break;
		recvbuf[len] = '\0';
		printf("%s", recvbuf);
	}

	freeaddrinfo(res);
	free(sendbuf);
	close(sockfd);
}

static int check_port(const char *port)
{
	const char *p = port;
	int r = 0;

	while (*p >= '0' && *p <= '9') {
		r = r * 10;
		if (r >= 65536)
			break;
		r = r + *p - '0';
		if (r >= 65536)
			break;
		p++;
	}

	return r > 0 && r < 65536;
}

static void usage(int exit_code)
{
	printf("Usage:\n");
	printf("  whois [-h] [-p PORT] [-s SERV] TLD\n");
	printf("\n");
	printf("Options:\n");
	printf("  -4          Use IPv4 (default)\n");
	printf("  -6          Use IPv6\n");
	printf("  -h          Show help message\n");
	printf("  -p PORT     Use port PORT instead of 43\n");
	printf("  -s SERV     Use whois server SERV instead of whois.iana.org\n");
	exit(exit_code);
}

int main(int argc, char **argv)
{
	const char *server = "whois.iana.org";
	const char *port = "43";
	int opt, af = AF_INET;

	while ((opt = getopt(argc, argv, ":46hp:s:")) != EOF) {
		switch (opt) {
		case '4':
			af = AF_INET;
			break;
		case '6':
			af = AF_INET6;
			break;
		case 'h':
			usage(0);
			break;
		case 'p':
			port = optarg;
			break;
		case 's':
			server = optarg;
			break;
		default:
			usage(1);
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		fprintf(stderr, "TLD not specified\n");
		return 1;
	}

	if (!check_port(port)) {
		fprintf(stderr, "Invalid port '%s'\n", port);
		return 1;
	}

	/* TODO: multiple tld requests? */
	whois_query(server, port, af, argv[0]);

	return 0;
}
