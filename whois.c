/*
 * whois - simple whois command
 *
 * whois has been placed in the public domain.
 * for details, see https://creativecommons.org/publicdomain/zero/1.0
 */

#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

static int check_port(const char *port)
{
	const char *p = port;

	while (isdigit(*p))
		p++;

	return *p == '\0';
}

static void usage(int exit_code)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  whois [-hv] [-p PORT] [-s SERV] TLD\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -h          Show help message\n");
	fprintf(stderr, "  -p PORT     Use port PORT instead of 43\n");
	fprintf(stderr, "  -s SERV     Use whois server SERV instead of whois.iana.org\n");
	fprintf(stderr, "  -v          Verbose, show description each step\n");

	exit(exit_code);
}

int main(int argc, char **argv)
{
	char recv_buf[25];
	int r = 0, fd = -1;
	int opt, verbose = 0;
	const char *port = "43";
	ssize_t recv_r, recv_tot = 0;
	const char *tld = NULL, *tld_tmp;
	const char *server = "whois.iana.org";
	struct addrinfo hint = {0}, *res = NULL;

	while ((opt = getopt(argc, argv, "hp:s:v")) != EOF) {
		switch (opt) {
		case 'h':
			usage(0);
			break;
		case 'p':
			port = optarg;
			if (!check_port(port)) {
				fprintf(stderr, "%s: invalid port %s\n", argv[0], port);
				usage(1);
			}
			break;
		case 's':
			server = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage(1);
			break;
		}
	}

	if (optind < argc)
		tld = argv[optind];

	if (!tld) {
		fprintf(stderr, "%s: TLD not specified\n", argv[0]);
		usage(1);
	}

	if ((tld_tmp = strstr(tld, "//"))) {
		if (verbose)
			printf("=== Ignoring URI scheme on TLD\n");
		tld_tmp += 2;
		while (*tld_tmp == '/')
			tld_tmp++;
		tld = tld_tmp;
	}

	if (verbose) {
		printf("=== WHOIS server %s, port %s\n", server, port);
		printf("=== Querying %s\n", tld);
	}

	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;

	if (verbose)
		printf("=== Calling getaddrinfo\n");

	if ((r = getaddrinfo(server, port, &hint, &res)) < 0) {
		fprintf(stderr, "%s: %s\n", server, gai_strerror(r));
		return 1;
	}

	if (verbose) {
		printf("=== getaddrinfo OK (%p)\n", (void *)res);
		printf("=== Creating socket descriptor\n");
	}

	if ((fd = socket(res->ai_family, res->ai_socktype, 0)) < 0) {
		perror("Failed to created socket descriptor");
		return 1;
	}

	if (verbose) {
		printf("=== Successfuly created socket descriptor (%d)\n", fd);
		printf("=== Start connecting to %s, port %s\n", server, port);
	}

	if ((r = connect(fd, res->ai_addr, res->ai_addrlen)) < 0) {
		fprintf(stderr, "Failed to connecting to %s, port %s", server, port);
		perror(NULL);
		return 1;
	}

	freeaddrinfo(res);

	if (verbose) {
		printf("=== Successfuly connected to %s, port %s\n", server, port);
		printf("=== Start sending command \"%s\\r\\n\"\n", tld);
	}

	if ((r = dprintf(fd, "%s\r\n", tld)) < 0) {
		fprintf(stderr, "Failed to sending \"%s\\r\\n\"", tld);
		if (errno)
			perror(NULL);
		else
			fprintf(stderr, "\n");
		return 1;
	}

	if (verbose)
		printf("=== %s response:\n\n", server);

	while ((recv_r = recv(fd, recv_buf, 24, 0)) > 0) {
		recv_buf[recv_r] = '\0';
		printf("%s", recv_buf);
		recv_tot += recv_r;
	}

	printf("\n");

	if (verbose)
		printf("=== Received %lu bytes\n", recv_tot);

	return 0;
}
