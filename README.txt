whois - simple whois command

  Usage:
    whois [-h] [-p PORT] [-s SERV] TLD

  Options:
    -4          Use IPv4
    -6          Use IPv6
    -h          Show help message
    -p PORT     Use port PORT instead of 43
    -s SERV     Use whois server SERV instead of whois.iana.org


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


references
----------
- https://en.wikipedia.org/wiki/WHOIS
- https://datatracker.ietf.org/doc/html/rfc3912

this software, whois, has been placed in the public domain.
