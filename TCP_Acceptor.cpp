// vim:ts=2:et
//===========================================================================//
//                              "TCP_Acceptor.cpp":                          //
//                         Common Setup for TCP Servers                      //
//===========================================================================//
#include "TCP_Acceptor.h"
#include <cstring>      // For C-style low-level 0-terminated strings
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
#include <string>       // For std::string

namespace Net
{
  //=========================================================================//
  // "TCP_Acceptor" Non-Default Ctor:                                        //
  //=========================================================================//
  TCP_Acceptor::TCP_Acceptor
  (
    int          a_port,
    char const*  a_acceptor_ip,
    EConcurModel a_cmodel
  )
  : m_acceptorSD(-1),            // Initially invalid
    m_cmodel    (a_cmodel)
  {
    // Create the acceptor socket (NOT for data interchange!)
    int sd = socket(AF_INET, SOCK_STREAM, 0);  // Default protocol: TCP
    if (sd < 0)
      throw std::runtime_error
            ("ERROR: Cannot create acceptor socket: " +
             std::string(strerror(errno)));

    // Bind the socket to the given IP Address and Port:
    sockaddr_in sa;
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr =
      (a_acceptor_ip == nullptr || *a_acceptor_ip == '\0')
      ? INADDR_ANY
      : inet_addr(a_acceptor_ip);
    sa.sin_port        = htons(uint16_t(a_port));

    int rc = bind(sd, (sockaddr const*)(&sa), sizeof(sa));
    if (rc < 0)
    {
      int e =    errno;
      (void) close(sd);
      throw std::runtime_error
            ("ERROR: Cannot bind SD=" + std::to_string(sd) + " to IP=" +
             std::string(inet_ntoa(sa.sin_addr))           + ", Port=" +
             std::to_string(a_port)   + ": " + std::string(strerror(e)));
    }
    // Create listen queue for 1024 clients:
    (void) listen(sd, 1024);

    // Setup successful: Memoise the socket created:
    m_acceptorSD = sd;
  }

  //=========================================================================//
  // "TCP_Acceptor": Non-Default Ctor using "argc", "argv":                  //
  //=========================================================================//
  // XXX: NOT THREAD-SAFE: Using "getopt":
  //
  TCP_Acceptor::TCP_Acceptor(int argc, char* argv[])
  {
    char const*  acceptorIP = nullptr;
    int          port       = -1;
    EConcurModel cmodel     = EConcurModel::Sequential;

    while (true)
    {
      int opt = getopt(argc, argv, "c:i:p:");
      if (opt < 0)
        break;

      switch (opt)
      {
      case 'c':
        // The actual arg is in "optarg":
        cmodel =
          (strcmp(optarg, "Sequential") == 0)
          ? EConcurModel::Sequential
          :
          (strcmp(optarg, "Fork")   == 0)
          ? EConcurModel::Fork
          :
          (strcmp(optarg, "Thread") == 0)
          ? EConcurModel::Thread
          :
          (strcmp(optarg, "ThreadPool") == 0)
          ? EConcurModel::ThreadPool
          :
          throw 
            std::invalid_argument
              ("Invalid EConcurModel: " + std::string(optarg));
        break;

      case 'i':
        acceptorIP = optarg;
        break;

      case 'p':
        port       = atoi(optarg);     // XXX: Better use "strtol"
        break;

      default:
        // OR JUST IGNORE UNUSED ARGS???
        assert(0 <= optind && optind < argc);
        throw
          std::invalid_argument("Invalid option: " + std::string(argv[optind]));
      }
    }
    // Invoke the actual Ctor:
    TCP_Acceptor(port, acceptorIP, cmodel);
  }

  //=========================================================================//
  // Dtor:                                                                   //
  //=========================================================================//
  TCP_Acceptor::~TCP_Acceptor()
  {
    (void) close(m_acceptorSD);
  }
}
