// vim:ts=2:et
//===========================================================================//
//                              "TCP_Acceptor.cpp":                          //
//                         Common Setup for TCP Servers                      //
//===========================================================================//
#include "TCP_Acceptor.h"
#include <cstring>          // For C-style low-level 0-terminated strings
#include <cstdlib>
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
#include <cassert>
#include <stdexcept>
#include <string>           // For std::string
#include <csignal>
#include <sys/wait.h>
#include <utxx/error.hpp>   // Using utxx for more convenient exceptions
#include <utxx/compiler_hints.hpp>

namespace Net
{
  //=========================================================================//
  // "TCP_Acceptor" Non-Default Ctor:                                        //
  //=========================================================================//
  TCP_Acceptor::TCP_Acceptor
  (
    int             a_port,
    spdlog::logger* a_logger,
    char const*     a_acceptor_ip,
    EConcurModel    a_cmodel
  )
  : m_acceptorSD(-1),            // Initially invalid
    m_cmodel    (a_cmodel),
    m_logger    (a_logger)
  {
    assert(m_logger != nullptr);

    if (UNLIKELY(a_port < 0 || a_port > 65535))
      throw utxx::badarg_error
            ("ERROR: TCP_Acceptor::Ctor: Invalid Port={}", a_port);

    // Create the acceptor socket (NOT for data interchange!)
    int sd = socket(AF_INET, SOCK_STREAM, 0);  // Default protocol: TCP
    if (UNLIKELY(sd < 0))
      // NB: Variable-arg ctor -- using a variadic template:
      throw utxx::runtime_error
            ("ERROR: Cannot create acceptor socket: errno=", errno, ": ",
             strerror(errno));

    // Bind the socket to the given IP Address and Port:
    sockaddr_in sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr =
      (a_acceptor_ip == nullptr || *a_acceptor_ip == '\0')
      ? INADDR_ANY
      : inet_addr(a_acceptor_ip);
    sa.sin_port        = htons(uint16_t(a_port));

    int rc = bind(sd, (sockaddr const*)(&sa), sizeof(sa));
    if (UNLIKELY(rc < 0))
    {
      int e =    errno;
      (void) close(sd);
      throw utxx::runtime_error
            ("ERROR: Cannot bind SD=",  sd, " to IP=", inet_ntoa(sa.sin_addr),
             ", Port=", a_port, ", errno=", e, ": ",   strerror(e));
    }
    // Create listen queue for 1024 clients:
    (void) listen(sd, 1024);

    // Setup successful: Memoise the socket created:
    const_cast<int&>(m_acceptorSD) = sd;

    // Set up the signal handler:
    // SIGINT, SIGQUIT, SIGTERM -- will terminate the acceptor loop gracefully;
    // SIGCHILD                 -- will "wait" for the child terminated to avoid
    //                             a zombie
    // FIXME: Better use "sigaction":
    if (UNLIKELY
       (signal(SIGINT,  SigHandler) == SIG_ERR ||
        signal(SIGQUIT, SigHandler) == SIG_ERR ||
        signal(SIGTERM, SigHandler) == SIG_ERR ||
        signal(SIGCHLD, SigHandler) == SIG_ERR))
    {
      (void) close(m_acceptorSD);
      throw utxx::runtime_error("ERROR: Cannot setup signal handlers");
    }
  }

  //=========================================================================//
  // "TCP_Acceptor": Non-Default Ctor using "argc", "argv":                  //
  //=========================================================================//
  TCP_Acceptor::TCP_Acceptor(int argc, char* argv[], spdlog::logger* a_logger)
  : m_acceptorSD(-1),
    m_cmodel    (EConcurModel::Sequential),
    m_logger    (a_logger)
  {
    assert(m_logger != nullptr);

    // Parse the command-line params. XXX: Using "getopt" is NOT thread-safe.
    // Reset "optind" to 1 so the "getopt" loop may be invoked multiple times
    // from different places in the program:
    optind = 1;
    opterr = 0;  // To avoid error msgs on unrecognised options
    char const*  acceptorIP = nullptr;
    int          port       = -1;
    EConcurModel cmodel     = EConcurModel::Sequential;

    while (true)
    {
      int opt = getopt(argc, argv, "c:i:p:");
      if (UNLIKELY(opt < 0))
        break;

      switch (opt)
      {
      case 'c':
        // The actual arg is in "optarg":
        cmodel     = EConcurModel::from_string(optarg);
        break;

      case 'i':
        acceptorIP = optarg;
        break;

      case 'p':
        port       = atoi(optarg);     // XXX: Better use "strtol"
        break;

      default:
        // Ignore unrecognised args -- they may be used by other components:
        ;
      }
    }
    if (UNLIKELY(port < 0))
      throw utxx::badarg_error
            ("TCP_Acceptor::Ctor: Missing Local Port.\nUSAGE:\n"
             "\t[-i LOCAL_IP_AADR]\n"
             "\t -p LOCAL_PORT\n"
             "\t[-c {Sequential|Fork|Thread|ThreadPool}]");

    // Invoke the actual Ctor: XXX: This is a DIRTY HACK, using "placement new".
    // Otherwise, delegated ctor invocation is only possible inside the initial-
    // isation list, not in the Ctor body:
    //
    new (this) TCP_Acceptor(port, a_logger, acceptorIP, cmodel);
  }

  //=========================================================================//
  // Dtor:                                                                   //
  //=========================================================================//
  TCP_Acceptor::~TCP_Acceptor()
  {
    (void) close(m_acceptorSD);
    // DO NOT de-allocate the logger -- NOT OWNED!
  }

  //=========================================================================//
  // "SigHandler":                                                           //
  //=========================================================================//
  bool TCP_Acceptor::s_exitRun = false; // Statically iinitialise to false

  void TCP_Acceptor::SigHandler(int a_signum)
  {
    switch (a_signum)
    {
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
      s_exitRun = true;
      break;

    case SIGCHLD:
      // TODO: Check and log the child's termination status:
      (void) waitpid(-1, nullptr, 0);
      break;

    default: ;
    }
  }
}
