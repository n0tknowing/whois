/*
 * whois - simple whois command
 *
 * whois has been placed in the public domain.
 * for details, see https://creativecommons.org/publicdomain/zero/1.0
 */

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
	fprintf(stderr, "  -4          Use IPv4 (default)\n");
	fprintf(stderr, "  -6          Use IPv6\n");
	fprintf(stderr, "  -h          Show help message\n");
	fprintf(stderr, "  -p PORT     Use port PORT instead of 43\n");
	fprintf(stderr, "  -s SERV     Use whois server SERV instead of whois.iana.org\n");
	fprintf(stderr, "  -v          Verbose, show description each step\n");

	exit(exit_code);
}

int main(int argc, char **argv)
{
	const char *prog = "whois";

	int af = AF_INET;
	char recv_buf[25], ip[INET6_ADDRSTRLEN];
	int r = 0, fd = -1;
	int opt, verbose = 0;
	void *addr;
	const char *port = "43";
	ssize_t recv_r, recv_tot = 0;
	const char *tld = NULL, *tld_tmp;
	const char *server = "whois.iana.org";
	struct addrinfo hint = {0}, *res = NULL;

	while ((opt = getopt(argc, argv, "46hp:s:v")) != EOF) {
		switch (opt) {
		case '4':
			break;
		case '6':
			af = AF_INET6;
			break;
		case 'h':
			usage(0);
			break;
		case 'p':
			port = optarg;
			if (!check_port(port)) {
				fprintf(stderr, "%s: invalid port %s\n", prog, port);
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

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		fprintf(stderr, "%s: TLD not specified\n", prog);
		usage(1);
	}

	tld = argv[0];

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

	hint.ai_family = af;
	hint.ai_socktype = SOCK_STREAM;

	if (verbose)
		printf("=== Calling getaddrinfo\n");

	if ((r = getaddrinfo(server, port, &hint, &res)) < 0) {
		if (r == EAI_SYSTEM)
			err(1, "%s", server);
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

	if (af == AF_INET)
		addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
	else
		addr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;

	if (!inet_ntop(af, addr, ip, sizeof(ip)))
		err(1, "inet_ntop");

	if (verbose) {
		printf("=== Successfuly created socket descriptor (fd=%d)\n", fd);
		printf("=== Start connecting to %s (%s), port %s\n",
				server, ip, port);
	}

	if ((r = connect(fd, res->ai_addr, res->ai_addrlen)) < 0) {
		fprintf(stderr, "Failed to connecting to %s (%s), port %s: ",
				server, ip, port);
		perror(NULL);
		return 1;
	}

	freeaddrinfo(res);

	if (verbose) {
		printf("=== Successfuly connected to %s (%s), port %s\n",
				server, ip, port);
		printf("=== Start sending command \"%s\\r\\n\"\n", tld);
	}

	if ((r = dprintf(fd, "%s\r\n", tld)) < 0) {
		fprintf(stderr, "Failed to sending command \"%s\\r\\n\"", tld);
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

	close(fd);
	return 0;
}
