// vim:ts=2:et
//===========================================================================//
//                                "ProcessHTTPReqs.c":                       //
//        Processing HTTP Requests in an Established Client Connection       //
//===========================================================================//
#include "ProcessHTTPReqs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

//===========================================================================//
// "ProcessHTTPReq":                                                         //
//===========================================================================//
int ProcessHTTPReqs(int a_sd)
{
  assert(a_sd >= 0);
  char reqBuff [1024];
  char sendBuff[65536];

  // For safety: 0-out both buffers:
  memset(reqBuff,  '\0', sizeof(reqBuff));
  memset(sendBuff, '\0', sizeof(sendBuff));

  // Receive multiple requests from the client:
  while (1)
  {
    int rc = recv(a_sd, reqBuff, sizeof(reqBuff)-1, 0); // Space for '\0'
    if (rc < 0)
    {
      if (errno == EINTR)
        continue;
      // Any other error: exit:
      fprintf(stderr, "WARNING: SD=%d, recv failed: %s, errno=%d\n",
              a_sd, strerror(errno), errno);
      close(a_sd);
      return rc;
    }
    else
    if (rc == 0)
    {
      fprintf(stderr, "INFO: SD=%d: Client disconnected\n", a_sd);
      close(a_sd);
      return rc;
    }
    // 0-terminate the bytes received:
    reqBuff[rc] = '\0';

    // Disconnect this client at the end of servicing this req, UNLESS
    // explicitly asked to keep the connection alive:
    int keepAlive  = 0;

    // If we got here: Received (rc) bytes, rc > 0:
    // XXX: We assume that we have received the whole req (1st line + headers)
    // at once, otherwise parsing would become REALLY complex:
    // Full req must have "\r\n\r\n" at the end:
    //
    if (rc < 4 || strcmp(reqBuff + (rc - 4), "\r\n\r\n") != 0)
    {
      fprintf(stderr, "INFO: SD=%d, Incomplete Req, disconnecting", a_sd);
      close(a_sd);
      return rc;
    }
    // OK, got a possibly complete req:
    // Parse the 1st line. The method must be GET, others not supported:
    if (strncmp(reqBuff, "GET ", 4) != 0)
    {
      fprintf(stderr,  "INFO: SD=%d: Unsupported Method: %s\n", a_sd, reqBuff);
      // Send the 501 error to the client:
      strcpy(sendBuff, "HTTP/1.1 501 Unsupported request\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // 0-terminate the 1st line:
    char*  lineEnd =  strstr(reqBuff, "\r\n");
    assert(lineEnd != NULL);
    *lineEnd = '\0';

    // Find Path: It must begin with a '/', with ' ' afterwards:
    // Start with (reqBuff+4), ie skip "GET " which we checked is there:
    char* path    = strchr(reqBuff + 4,   '/');
    char* pathEnd = (path == NULL) ? NULL : strchr(path, ' ');

    // 0-terminate path:
    if (pathEnd == NULL)
    {
      fprintf(stderr,  "INFO: SD=%d: Missing Path: %s\n", a_sd, reqBuff);
      // Send the 501 error to the client:
      strcpy(sendBuff, "HTTP/1.1 501 Missing Path\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // OK, got a valid and framed path:
    assert(path != NULL);
    *pathEnd = '\0';

    // Check the HTTP version (beyond pathEnd):
    char const* httpVer =  strstr (pathEnd + 1, "HTTP/");
    if (httpVer == NULL || strncmp(httpVer + 5, "1.1", 3) != 0)
    {
      fprintf(stderr,  "INFO: SD=%d: Invalid HTTPVer: %s\n", a_sd, reqBuff);
      // Send the 501 error to the client:
      strcpy(sendBuff,
        "HTTP/1.1 501 Unsupported/Invalid HTTP Version\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // Parse the Headers. We are only interested in the "Connection: " header:
    char const* nextLine = httpVer + 10;  // Was: "HTTP/1.1\r\n"
    char const* connHdr  = strstr(nextLine, "Connection: ");

    if (connHdr != NULL)
    {
      char const* connHdrVal = connHdr + 12;
      // Skip any further spaces:
      for (; *connHdrVal == ' '; ++connHdrVal) ;

      if (strncasecmp(connHdrVal, "Keep-Alive", 11) == 0)
        keepAlive = 1;
      else
      if (strncasecmp(connHdrVal, "Close", 5) != 0)
        connHdr = NULL;  // INVALID!
    }
    if (connHdr == NULL)
    {
      fprintf(stderr,
        "INFO: SD=%d: Missing/Invalid Connecton: Header\n", a_sd);

      // Send the 501 error to the client:
      strcpy(sendBuff,
        "HTTP/1.1 501 Missing/Invalid Connection Header\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // Got Path and KeepAlive params!
    // FIXME: Security considerations are very weak here!
    assert(*path == '/');
    // Prepend path with '.' to make it relative to the current working
    // directory of the server: XXX: Bad style, but wotks in this case:
    --path;
    *path = '.';

    // Open the file specified by path:
    int fd = open(path, O_RDONLY);

    struct stat statBuff;
    rc   = fstat(fd, &statBuff);
    // We can only service regular files:
    int isRegFile = S_ISREG(statBuff.st_mode);

    if (fd < 0 || rc < 0 || !isRegFile)
    {
      fprintf(stderr,  "INFO: Missing/Unaccessible file: %s\n", path);
      // Send a 401 error to the client:
      strcpy(sendBuff, "HTTP/1.1 401 Missing File\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // Get the file size:
    size_t fileSize = statBuff.st_size;

    // Response to the client:
    sprintf(sendBuff,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: %ld\r\n"
      "Connection: %s\r\n\r\n",
      fileSize,
      keepAlive ? "Keep-Alive" : "Close");

    // Read the file in chunks and send it to the client:
    while (1)
    {
      int chunkSize = read(fd,  sendBuff, sizeof(sendBuff));
      rc = send(a_sd, sendBuff, chunkSize, 0);

      // rc <  0: network error;
      // rc == 0: client SWAMPED by our data;
      // XXX:  in both cases, disconnect:
      if (rc <= 0)
      {
        fprintf(stderr, "ERROR: SD=%d: send returned %d: %s, errno=%d\n",
                a_sd, rc, strerror(errno), errno);
        close(fd);
        close(a_sd);
        return rc;
      }
      // Exit normally if got to the end of file (or file reading error):
      if (chunkSize < sizeof(sendBuff))
      {
        close(fd);
        break;
      }
    }
    // Done with this Req:
  NextReq:
    if (!keepAlive)
    {
      close(a_sd);
      fprintf(stderr, "INFO: SD=%d closed: Keep-Alive=0\n", a_sd);
      return 0;
    }
  }
  // This point is unreachable!
  __builtin_unreachable();
}
