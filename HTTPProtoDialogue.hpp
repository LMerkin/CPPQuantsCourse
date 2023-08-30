// vim:ts=2:et
//===========================================================================//
//                            "HTTPProtoDialogue.hpp":                       //
//        Processing HTTP Requests in an Established Client Connection       //
//===========================================================================//
#pragma once

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
#include <utility>
#define  SPDLOG_USE_STD_FORMAT
#include <spdlog/spdlog.h>

namespace Net
{
  //=========================================================================//
  // Class "HTTPProtoDialogue":                                              //
  //=========================================================================//
  template<typename UserAction>
  class HTTPProtoDialogue
  {
  public:
    // For passing (Key, Val) params to UserAction:
    using KV = std::pair<char const*, char const*>;

  private:
    UserAction            m_userAction;
    int     const         m_recvBuffSz;
    char*   const         m_recvBuff;
    int     const         m_maxKVs;
    KV*     const         m_KVs;
    spdlog::logger* const m_logger;         // NOT OWNED!

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
      unsigned          a_max_kvs      = 256,
      unsigned          a_max_out_sz   = 65536
    )
    : m_userAction(a_user_action),  // Copy Ctor on "UserAction" invoked here!
      m_recvBuffSz(int(a_recv_buff_sz)),
      m_recvBuff  (new char[m_recvBuffSz]),
      m_maxKVs    (int(a_max_kvs)),
      m_KVs       (new KV  [m_maxKVs]),
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
      m_maxKVs    (a_right.m_maxKVs),
      m_KVs       (new KV  [m_maxKVs]),
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
      delete[] (m_maxKVs);
      // Protection against potential multiple Dtor invocation:
      const_cast<char*&>(m_recvBuff) = nullptr;
      const_cast<KV*&>  (m_KVs)      = nullptr;
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
      bool   keepAlive  = true;
      size_t lastRecvSz = 
      memset(m_recvBuff, '\0', m_recvBuffSz);    // For safety

      while (true)
      {
        //-------------------------------------------------------------------//
        // Done with the prev Req:                                           //
        //-------------------------------------------------------------------//
        // NB: "keepAlive" is true 
        if (!keepAlive)
        {
          m_logger->info("SD={} closed", a_sd)
          close(a_sd);
          return;
        }
        // Clean the recv buffer for safety: FIXME: Might be too expensive!

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
            ("HTTPProtoDialogue: SD={}, recv failed: errno={}: {}",
             a_sd, errno, strerror(errno));
          (void) close(a_sd);          // TCP_Acceptor will close it as well!
          return;
        }
        else
        if (rc == 0)
        {
          m_logger->info
            ("HTTPProtoDialogue: SD={}: Client disconnected", a_sd);
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
        // XXX: We assume that we have received the whole req
        // (1st line + headers) at once, otherwise parsing would become REALLY
        // complex:
        // Full req must have "\r\n\r\n" at the end:
        //
        if (rc < 4 || strcmp(m_recvBuff + (rc - 4), "\r\n\r\n") != 0)
        {
          m_logger->error("SD={}: Incomplete Req, disconnecting", a_sd);
          (void) close(a_sd);
          return;
        }
 
        // OK, got a possibly complete req:
        // Parse the 1st line. The method must be GET, others not supported:
        if (strncmp(m_recvBuff, "GET ", 4) != 0)
        {
          m_logger->info("SD={}: Unsupported Method: %s\n", a_sd, m_recvBuff);
          // Send the 501 error to the client:
          constexpr char errMsg[] = "HTTP/1.1 501 Unsupported request\r\n\r\n";
          (void) send (a_sd, errMsg, sizeof(errMsg)-1, 0);
          continue;
        }

        // 0-terminate the 1st line:
        char*  lineEnd =  strstr(m_recvBuff, "\r\n");
        assert(lineEnd != nullptr && lineEnd < m_recvBuff + rc);
        *lineEnd = '\0';

        // Find the Path: It must begin with a '/', with ' ' afterwards:
        // Start with (m_recvBuff+4), ie skip "GET " which we checked is there:
        // XXX: Extra spaces between "GET" and Path are not allowed, as well
        //      as extra spaces at the end of the Path:
        char* path    = strchr(m_recvBuff + 4,  '/');
        char* pathEnd = (path == nullptr) ? nullptr : strchr(path, ' ');

        if (pathEnd == nullptr)
        {
          // This is not a correct req -- path not found:
          m_logger->info("SD={}: Missing Path in\n{}\n", a_sd, m_recvBuff);

          // Send the 501 error to the client:
          constexpr char errMsg[] = "HTTP/1.1 501 Missing Path\r\n\r\n";
          (void) send (a_sd, errMsg, sizeof(errMsg)-1, 0);
          continue;
        }
        // NB: Do not 0-terminate the Path yet, it may truncate the log msg
        // below:

        // Check the HTTP version  (end of 1st req line, beyond pathEnd):
        // It needs to be HTTP/1.1 (other versions not supported here):
        char const* HTTP11 = "HTTP/1.1";

        char const* httpVer =  strstr(pathEnd + 1, HTTP11);
        if (httpVer == nullptr)
        {
          m_logger->info("SD={}: Invalid HTTPVer in\n{}\n", a_sd, m_recvBuff);
          // Send the 501 error to the client:
          constexpr char errMsg[] =
            "HTTP/1.1 501 Unsupported/Invalid HTTP Version\r\n\r\n";
          (void) send(a_sd, errMsg, sizeof(errMsg)-1, 0);
          continue;
        }

        // OK, we can now 0-terminate the Path:
        assert(path != nullptr && path < pathEnd && pathEnd < lineEnd);
        *pathEnd = '\0';

        // 1st line of the request done.
        // Now parse the Headers. XXX: We are currently only interested in the
        // "Connection: " header, so search for it directly -- all others are
        // ignored:
        char const* nextLine = httpVer + strlen(HTTP11) + 2;
        char const* ConnKey  = "Connection: ";
        char const* connHdr  = strstr(nextLine, ConnKey);
        // XXX: Or use "strcasestr" for case-insensitive search?

        if (connHdr != nullptr)
        {
          char const* connHdrVal = connHdr + strlen(ConnKey);
          // XXX: Assume there are no extra spaces between the HdrKey and the
          // HdrVal:
          if (strncasecmp(connHdrVal, "Keep-Alive", 11) == 0)
            keepAlive = true;
          else
          if (strncasecmp(connHdrVal, "Close", 5) != 0)
          {
            assert(!keepAlive);
            connHdr = nullptr;  // INVALID; keepAlive remains false
          }
        }
        if (connHdr == nullptr)
        {
          m_logger->info("SD={}: Missing/Invalid \"Connecton: \" Header", a_sd);
          // Send the 501 error to the client:
          constexpr char errMsg[] =
            "HTTP/1.1 501 Missing/Invalid Connection Header\r\n\r\n";
          (void) send  (a_sd, errMsg, sizeof(errMsg)-1, 0);
          continue;
        }
        // So: Got the Path and KeepAlive params!
        //-------------------------------------------------------------------//
        // Process the HTTP GET request given by Path:                       //
        //-------------------------------------------------------------------//
        // Parse the Path which is assume to be:
        // /OpName?Key1=Val1&...&KeyN=ValN
        //
        assert(*path = '/');
        char const* optName = path + 1;

        // Find the end of "optName";
        int N = 0;
        char* optNameEnd = strchr(optName, '?');
        if (optNameEnd != nullptr)
        {
          assert(optNameEnd < pathEnd);
          // 0-terminate the optName:
          *optNameEnd = '\0';

          char const* key     = optNameEnd + 1;
          bool        invalid = false;

          for (; N < m_maxKVs && key < pathEnd; ++N)
          {
            char const* eq = strchar(key, '=');
            if (eq == nullptr || eq == key+1 || eq == pathEnd-1)
            {
              constexpr char errMsg[] =
                "HTTP/1.1 501 Incorrect Key=Val params\r\n\r\n";
              (void) send  (a_sd, errMsg, sizeof(errMsg)-1, 0);

              invalid = true;
              break;
            }
            char const* val = eq + 1;

            // Find the next key and 0-terminate the val. If not found, we are
            // at the end of Path:
            char* amp = strchr(val, '&');
            if (amp != nullptr)
              *amp = '\0';
 
            // Store the (key,val) ptrs:
            m_KVs[N] = std::make_pair(key, val);

            // Continue or done?
            if (amp == nullptr)
              break;
            key = amp + 1;
          }
          if (invalid)
            continue;
        }

        //-------------------------------------------------------------------//
        // Invoke the UserAction:                                            //
        //-------------------------------------------------------------------//
        // (respBody, respLen) = UserAction(char const*, int, KV const*) :
        std::pair<char const*, int> userResp {nullptr, 0};
        try
        {
          userResp = m_userAction(optName, N, m_KVs);
        }
        catch (std::exception const& exn)
        {
          m_logger->error("EXCEPTION in UserAction: {}", exn.what());
        }
        catch (...)
        {
          m_logger->error("UNKNOWN EXCEPTION in UserAction");
        }

        //-------------------------------------------------------------------//
        // RESPONSE to the client:                                           //
        //-------------------------------------------------------------------//
        int respLen = userResp.second;

        // Handle errors first:
        if (respLen <= 0 || userResp.first == nullptr)
        {
          constexpr char errMsg[] = "HTTP/1.1 401 No Result\r\n\r\n";
          (void) send  (a_sd, errMsg, sizeof(errMsg)-1, 0);
          continue;
        }

        // If we got here:
        assert(respLen > 0 && userResp.second != nullptr);

        // Form the 1st resp line and headers:
        // XXX: Formatted output by "sprintf" may be slightly inefficient:
        char hdrsBuff[256];
        rc = snprintf
        (
          hdrsBuff, sizeof(hdrsBuff),
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/plain\r\n"
          "Content-Length: %d\r\n"
          "Connection: %s\r\n\r\n",
          respLen,
          keepAlive ? "Keep-Alive" : "Close"
        );
        assert(rc < sizeof(hdrsBuff));

        // Gather the data to be sent from "hdrsBuff" and "m_userOut":
        iovec  iov[2]
        {
          { .iov_base = hdrsBuff,       .iov_len = rc      },
          { .iov_base = userResp.first, .iov_len = respLen }  // NULL/0 is OK
        };

        msghdr gather
        {
          .msg_name       = nullptr,
          .msg_namelen    = 0,
          .msg_iov        = &iov,
          .msg_iovlen     = 2,
          .msg_control    = nullptr,
          .msg_controllen = 0,
          .msg_flags      = 0
        };

        // Finally, send the response out:
        rc = sendmsg(a_sd, &gather, 0);

        // rc <  0: network error;
        // rc == 0: client SWAMPED by our data;
        // XXX:  in both cases, disconnect; EINTR is extremely unlikely here:
        //
        if (rc <= 0)
        {
          m_logger->error("SD={}: sendmsg failed: errno={}: {}",
                          a_sd, errno, strerror(errno));
          close(a_sd);
          return;
        }
      }
      // End of infinite "while" loop
    }
    // End of "operator()"
  };
}
// End namespace Net
