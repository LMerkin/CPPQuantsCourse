// vim:ts=2:et
//===========================================================================//
//                               "HTTPServer1.cpp":                          //
//                         Simple Sequential HTTP Server                     //
//===========================================================================//
#include "ServerSetup.h"
#include "ProcessHTTPReqs.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>

//===========================================================================//
// "main":                                                                   //
//===========================================================================//
int main(int argc, char* argv[])
{
  // Create and bind the Acceptor Socket:
  int sd  = ServerSetup(argc, argv);
  if (sd < 0)
    return 1;

  // Acceptor Loop:
  while (1)
  {
    // Accept a connection, create a data exchange socket:
    int sd1 = accept(sd, NULL, NULL);
    if (sd1 < 0)
    {
      // Some error in "accept", but may be not really serious:
      if (errno == EINTR)
        // "accept" was interrupted by a signal, this is OK, just continue:
        continue;

      // Any other error:
      std::cerr << "ERROR: accept failed: " << strerror(errno) << std::endl;
      return 1;
    }
    // XXX: Clients are services SEQUNTIALLY. If the currently-connected client
    // sends multiple reqs, all other clients will be locked out until this one
    // disconnects:
    (void) ProcessHTTPReqs(sd1);
  }
  return 0;
}
