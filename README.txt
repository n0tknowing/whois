whois - simple whois command

  Usage:
    whois [-hv] [-p PORT] [-s SERV] TLD

  Options:
    -4          Use IPv4 (default)
    -6          Use IPv6
    -h          Show help message
    -p PORT     Use port PORT instead of 43
    -s SERV     Use whois server SERV instead of whois.iana.org
    -v          Verbose, show description each step

installation and example
------------------------
$ make
$ ./whois example.com
% IANA WHOIS server
% for more information on IANA, visit http://www.iana.org
% This query returned 1 object

domain:       EXAMPLE.COM

organisation: Internet Assigned Numbers Authority

created:      1992-01-01
source:       IANA


$ ./whois -v example.com
=== WHOIS server whois.iana.org, port 43
=== Querying example.com
=== Calling getaddrinfo
=== getaddrinfo OK (0x56051a650870)
=== Creating socket descriptor
=== Successfuly created socket descriptor (3)
=== Start connecting to whois.iana.org, port 43
=== Successfuly connected to whois.iana.org, port 43
=== Start sending command "example.com\r\n"
=== whois.iana.org response:

% IANA WHOIS server
% for more information on IANA, visit http://www.iana.org
% This query returned 1 object

domain:       EXAMPLE.COM

organisation: Internet Assigned Numbers Authority

created:      1992-01-01
source:       IANA


=== Received 233 bytes

references
----------
- https://en.wikipedia.org/wiki/WHOIS
- https://datatracker.ietf.org/doc/html/rfc3912

this software, whois, has been placed in the public domain.
