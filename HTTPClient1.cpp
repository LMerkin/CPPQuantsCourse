// vim:ts=2:et
// Language-Standard Headers:
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <iostream>

// Socket API Headers (mostly UNIX-Specific):
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// UNIX-Specific Headers:
#include <errno.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char* argv[])
{
  using namespace std;

  if (argc != 2)
  {
    fputs("ARGUMENT: URL\n", stderr);
    return 1;
  }
  //-------------------------------------------------------------------------//
  // Parse the URL:                                                          //
  //-------------------------------------------------------------------------//
  // For example:
  // http://hostname[:port][/path]
  //
  char* url = argv[1];

  // Find the "://" separator:
  char* protoEnd = strstr(url, "://");
  if (protoEnd == nullptr)
  {
    cerr << "ERROR: Invalid URL: " << url << endl;
    return 1;
  }
  // Dirty trick: DESTRUCTIVE PARSING:
  // Currently, protoEnd points to ':' followed by "//..."
  // Overwrite by '\0' to terminate the proto:
  assert(*protoEnd == ':');
  *protoEnd         = '\0';
  char const* proto = url;
  url = protoEnd + 3;  // Now points to the hostname

  // XXX: Currently, only "http" proto is supported:
  if (strcmp(proto, "http") != 0)
  {
    cerr << "ERROR: Unsupported Protocol: " << proto << endl;
    return 1;
  }
  char const* path  = "";               // Empty yet; leading '/' assumed
  char* hostPortEnd = strchr(url, '/');
  if (hostPortEnd != nullptr)
  {
    *hostPortEnd    = '\0';             // Over-writing '/'
    path            = hostPortEnd + 1;  // W/o the leading '/'
  }

  // Have we got the port?
  char const*   hostName = url;
  unsigned long port     = 80;          // Default for HTTP
  char* colon = strchr(url, ':');
  if (colon != nullptr)
  {
    // Port specified explicitly:
    // Replace ':' with '\0', thus terminating the hostName:
    *colon    = '\0';
    // colon+1 points to port digits:
    char* end = nullptr;

    port   = strtoul(colon + 1, &end, 10);
    if (port == 0 || port > 65535 || end == nullptr || *end != '\0')
    {
      cerr << "ERROR: Invalid Port: " << (colon + 1) << endl;
      return 1;
    }
  }
  // Parsing Done!
  // We got: hostName, port, path (w/o the leading '/')

  //-------------------------------------------------------------------------//
  // Get the hostIP: Make DNS enquiery (XXX: BLOCKING, NON-THREAD-SAFE!):    //
  //-------------------------------------------------------------------------//
  // FIXME: Replace "gethostbyname" with "getaddrinfo":
  hostent* he = gethostbyname(hostName);

  // Count the IP addresses we have received:
  int nAddrs = 0;
  for (; he != nullptr && he->h_addr_list[nAddrs] != nullptr; ++nAddrs) ;

  // NB: Check that we got anything at all, the address(es) are of 4-byte
  // format (IPv4) and of the Internet type:
  assert(nAddrs == 0 || he != nullptr);
  if (nAddrs == 0 || he->h_length != 4 || he->h_addrtype != AF_INET)
  {
    cerr << "ERROR: Cannot resolve HostName: " << hostName << endl;
    return 1;
  }

  // Get the 0th result from the list:
  assert(nAddrs > 0);
  // XXX: UNSAFE, YOU MUST KNOW EXACTLY WHAT YOU ARE DOING!
  in_addr const* ia = reinterpret_cast<in_addr const*>(he->h_addr_list[0]);

  // Make the server address:
  sockaddr_in sa;
  sa.sin_family = AF_INET;
  sa.sin_addr   = *ia;
  sa.sin_port   = htons(uint16_t(port));

  //-------------------------------------------------------------------------//
  // Create or client-side socket:                                           //
  //-------------------------------------------------------------------------//
  int sd = socket(AF_INET, SOCK_STREAM, 0);  // HTTP is over IP + TCP
  if (sd < 0)
  {
    cerr << "ERROR: Cannot create socket: " << strerror(errno) << endl;
    return 1;
  }
  // Can also bind this socket to a LocalIP+LocalPort, but this is not strictly
  // necessary on the Client Side...

  //-------------------------------------------------------------------------//
  // Connect to HostIP+Port:                                                 //
  //-------------------------------------------------------------------------//
  int rc = 0;
  while (true)
  {
    // This is blocking call:
    rc = connect(sd, reinterpret_cast<sockaddr const*>(&sa), sizeof(sa));
    if (rc < 0)
    {
      if (errno == EINTR)
        continue;  // Innocent error

      // Any other error: exit:
      // NB: "inet_ntoa" is not theread-safe!
      cerr << "ERROR: Cannot connect to server: SD=" << sd   << ", IP="
           << inet_ntoa(sa.sin_addr) << ", Port="    << port << ": "
           << strerror(errno)        << endl;
      close(sd);       // Not really necessary...
      return 1;
    }
    assert(rc >= 0);
    break;             // Successfully connected
  }
  // If we got here, TCP connect was successful!

  //-------------------------------------------------------------------------//
  // Initialise the HTTP session:                                            //
  //-------------------------------------------------------------------------//
  // Send the HTTP/1.1 Request:
  // GET Path HTTP/1.1
  // Headers...
  char sendBuff[1024];

  int actLen =
    snprintf(sendBuff, sizeof(sendBuff),
          "GET /%s HTTP/1.1\r\n"
          "Host: %s\r\n"          // HTTP/1.1 requires the HostName once again
          "Connection: Close\r\n"
          "\r\n",
          path, hostName);
  if (actLen >= int(sizeof(sendBuff)))
  {
    cerr << "ERROR: Request too long: " << sendBuff << endl;
    return 1;
  }
 
  // OK, ready to send it out:
  cerr << "INFO: Sending the HTTP Request:\n" << sendBuff << endl;
  rc = send(sd, sendBuff, actLen, 0);

  // We expect all data in "sendBuff" to be sent at once. In general, this may
  // not be the case:
  if (rc != actLen)
  {
    cerr << "ERROR: Cannot send HTTP Req: Only " << rc << " bytes sent: "
         << strerror(errno) << ", errno=" << errno     << endl;
    close(sd);      // Not necessary
    return 1;
  }
  //-------------------------------------------------------------------------//
  // Get the response:                                                       //
  //-------------------------------------------------------------------------//
  char recvBuff[65536];     // 64k
  while (true)
  {
    rc = recv(sd, recvBuff, sizeof(recvBuff)-1, 0);  // 1 byte for 0-terminator
    // rc >  0: #bytes received
    // rc <  0: error
    // rc == 0: the server has closed connection
    if (rc < 0)
    {
      if (errno == EINTR)
        continue;  // Innocent, just try again

      // Any other error: exit:
      cerr  << "ERROR: recv failed: " << strerror(errno) << endl;
      close(sd);
      return 1;
    }
    else
    if (rc == 0)
      break;      // All Done!

    // Recv successful! 0-terminate the string and print what we got:
    assert(rc > 0);
    recvBuff[rc] = '\0';
    cout << recvBuff;
  }
  //-------------------------------------------------------------------------//
  // Clean-up:                                                               //
  //-------------------------------------------------------------------------//
  cout << endl;
  close(sd);
  return 0;
}
