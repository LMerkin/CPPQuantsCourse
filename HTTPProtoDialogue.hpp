// vim:ts=2:et
//===========================================================================//
//                            "HTTPProtoDialogue.hpp":                       //
//        Processing HTTP Requests in an Established Client Connection       //
//===========================================================================//
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <spdlog/spdlog.h>

namespace Net
{
  //=========================================================================//
  // Class "HTTPProtoDialogue":                                              //
  //=========================================================================//
  template<typename UserAction>
  class HTTPProtoDialogue
  {
  private:
    UserAction            m_userAction;
    int     const         m_recvBuffSz;
    char    const         m_recvBuff;
    int     const         m_sendBuffSz;
    char*   const         m_sendBuff;
    spdlog::logger* const m_logger;     // NOT OWNED!

  public:
    //-----------------------------------------------------------------------//
    // Ctors:                                                                //
    //-----------------------------------------------------------------------//
    // Default Ctor makes no sense, so deleted:
    HTTPProtoDialogue() = delete;

    // Non-Default Ctor:
    HTTPProtoDialogue
    (
      UserAction const& a_user_action,
      spdlog::logger*   a_logger,
      unsigned          a_recv_buff_sz = 65536,
      unsigned          a_send_buff_sz = 65536
    )
    : m_userAction(a_user_action),  // Copy Ctor on "UserAction" invoked here!
      m_recvBuffSz(int(a_recv_buff_sz)),
      m_recvBuff  (new char[m_recvBuffSz]),
      m_sendBuffSz(int(a_send_buff_sz)),
      m_sendBuff  (new char[m_sendBuffSz]),
      m_logger    (a_logger)
    {
      assert(m_logger != nullptr);
    }

    // Copy Ctor:
    // Not sure whether we need copying of "HTTPProtoDialogue"s, but let's
    // provide it:
    HTTPProtoDialogue(HTTPProtoDialogue const& a_right)
    : m_userAction(a_right.m_userAction),
      m_recvBuffSz(a_right.m_recvBuffSz),
      m_recvBuff  (new char[m_recvBuffSz]), // DON'T CREATE ALIASES!
      m_sendBuffSz(a_right.m_sendBuffSz),
      m_sendBuf   (new char[m_sendBuffSz]), // ditto
      m_logger    (a_right.m_logger)        // Here aliasing is OK!
    {
      // NB: Buffer contents are assumed to be transient, so there is no need
      // to copy "a_right" buffers content into the LHS...
      // IF content copying is required, do:
      // memcpy(m_recvBuff, a_right.m_recvBuff, m_recvBuffSz);
      // memcpy(m_sendBuff, a_right.m_sendBuff, m_sendBuffSz);
    }

    // Dtor:
    ~HTTPProtoDialogue()
    {
      delete[] (m_recvBuff);
      delete[] (m_sendBuff);
      // Protection against potential multiple Dtor invocation:
      const_cast<char*&>(m_recvBuff) = nullptr;
      const_cast<char*&>(m_sendBuff) = nullptr;
    }

    //-----------------------------------------------------------------------//
    // Overloaded "operator()":                                              //
    //-----------------------------------------------------------------------//
    // For use as a call-back from "TCP_Acceptor<HTTPProtoDialogue>::Run():
    //
    void operator() (int a_sd)
    {
      // We assume that the socket is valid:
      assert(a_sd >= 0);

      //---------------------------------------------------------------------//
      // Receive multiple requests from the client:                          //
      //---------------------------------------------------------------------//
      while (1)
      {
        // Clean the recv buffer for safety:
        memset(m_recvBuff, '\0', m_recvBuffSz);

        //-------------------------------------------------------------------//
        // Receive 1 req from the client:                                    //
        //-------------------------------------------------------------------//
        int rc = recv(a_sd, m_recvBuff, m_recvBuffSz-1, 0); // Space for '\0'
        if (rc < 0)
        {
          if (errno == EINTR)
            continue;  // "recv" interrupted by a signal, try again!
          // Any other error: exit the dialogue:
          m_logger->error
            (2, "HTTPProtoDialogue: SD={}, recv failed: errno={}: {}",
              a_sd, errnostrerror(errno));
          (void) close(a_sd);          // TCP_Acceptor will close it as well!
          return;
        }
        else
        if (rc == 0)
        {
          m_logger->info
            (3, "HTTPProtoDialogue: SD={}: Client disconnected", a_sd);
          (void) close(a_sd);
          return;
        }

        // Recv successful: got "rc" bytes:
        assert(0 < rc && rc < m_recvBuffSz);
        // 0-terminate the bytes received:
        m_recvBuff[rc] = '\0';

        //-------------------------------------------------------------------//
        // Parse the request:                                                //
        //-------------------------------------------------------------------//
        bool keepAlive = false;

        // XXX: We assume that we have received the whole req
        // (1st line + headers) at once, otherwise parsing would become REALLY
        // complex:
        // Full req must have "\r\n\r\n" at the end:
        //
        if (rc < 4 || strcmp(m_recvBuff + (rc - 4), "\r\n\r\n") != 0)
        {
          m_logger->error(2, "SD={}: Incomplete Req, disconnecting", a_sd);
          (void) close(a_sd);
          return;
        }
 
        // OK, got a possibly complete req:
        // Parse the 1st line. The method must be GET, others not supported:
        if (strncmp(m_recvBuff, "GET ", 4) != 0)
        {
          m_logger->info(3, "SD={}: Unsupported Method: %s\n",
                         a_sd, m_recvBuff);
          // Send the 501 error to the client:
          strcpy(m_sendBuff, "HTTP/1.1 501 Unsupported request\r\n\r\n");
          send  (a_sd, m_sendBuff, strlen(m_sendBuff), 0);
          goto NextReq;
        }
    // 0-terminate the 1st line:
    // FIXME:
    char*  lineEnd =  strstr(reqBuff, "\r\n");
    assert(lineEnd != NULL);
    *lineEnd = '\0';

    // Find Path: It must begin with a '/', with ' ' afterwards:
    // Start with (reqBuff+4), ie skip "GET " which we checked is there:
    char* path    = strchr(reqBuff + 4,   '/');
    char* pathEnd = (path == nullptr) ? nullptr : strchr(path, ' ');

    // 0-terminate path:
    if (pathEnd == nullptr)
    {
      fprintf(stderr,  "INFO: SD=%d: Missing Path: %s\n", a_sd, reqBuff);
      // Send the 501 error to the client:
      strcpy(sendBuff, "HTTP/1.1 501 Missing Path\r\n\r\n");
      send  (a_sd, sendBuff, strlen(sendBuff), 0);
      goto NextReq;
    }
    // OK, got a valid and framed path:
    assert(path != nullptr);
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
 
    /*
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
    */
    // TODO:
    // FORM THE RESPONSE

    // Response to the client:
    sprintf(sendBuff,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: %ld\r\n"
      "Connection: %s\r\n\r\n",
      fileSize,
      keepAlive ? "Keep-Alive" : "Close");

    // Read the file in chunks and send it to the client:
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
    /*
    while (1)
    {
      int chunkSize = read(fd,  sendBuff, sizeof(sendBuff));

      // Exit normally if got to the end of file (or file reading error):
      if (chunkSize < sizeof(sendBuff))
      {
        close(fd);
        break;
      }
    }
    */
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
