// vim:ts=2:et
//===========================================================================//
//                                "ServerSetup.c":                           //
//                          Common Setup for TCP Servers                     //
//===========================================================================//
#include "ServerSetup.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

//===========================================================================//
// "ServerSetup":                                                            //
//===========================================================================//
// Returns the Acceptor Socket, or (-1) on error:
//
int ServerSetup(int argc, char* argv[])
{
  // FIXME!
  if (argc != 2)
  {
    fputs("ARGUMENT: ServerPort\n", stderr);
    return -1;
  }
  int port = atoi(argv[1]);

  // Create the acceptor socket (NOT for data interchange!)
  int sd = socket(AF_INET, SOCK_STREAM, 0);  // Default protocol: TCP
  if (sd < 0)
  {
    fprintf(stderr, "ERROR: Cannot create acceptor socket: %s\n",
            strerror(errno));
    close(sd);
    return -1;
  }

  // Bind the socket to the given port:
  sockaddr_in sa;
  sa.sin_family      = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port        = htons(uint16_t(port));

  int rc = bind(sd, (struct sockaddr const*)(&sa), sizeof(sa));
  if (rc < 0)
  {
    fprintf(stderr, "ERROR: Cannot bind SD=%d to Port=%d: %s, errno=%d\n",
            sd, port, strerror(errno), errno);
    return -1;
  }
  // Create listen queue for 1024 clients:
  (void) listen(sd, 1024);

  // ALso, for safety, chroot to the current dir:
  if (geteuid() == 0)
  {
    rc = chroot(".");
    fprintf(stderr, "INFO: chroot: rc=%d, errno=%d\n", rc, errno);
    if (rc < 0)
      return -1;
  }
  // Setup successful:
  assert(sd >= 0);
  return sd;
}
