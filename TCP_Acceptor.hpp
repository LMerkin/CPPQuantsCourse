// vim:ts=2:et
//===========================================================================//
//                            "TCP_Acceptor.hpp":                            //
//               Implementation of the Templated "Run" Method                //
//===========================================================================//
#pragma once

#include "TCP_Acceptor.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdexcept>
#include <utxx/error.hpp>
#define  SPDLOG_USE_STD_FORMAT
#include <spdlog/spdlog.h>

namespace Net
{
  //=========================================================================//
  // "TCP_Acceptor::Run":                                                    //
  //=========================================================================//
  template<typename ProtoDialogue>
  void TCP_Acceptor::Run(ProtoDialogue& a_proto)
  {
    while (!s_exitRun)
    {
      //---------------------------------------------------------------------//
      // Accept a connection, create a data exchange socket:                 //
      //---------------------------------------------------------------------//
      int sd1 = accept(m_acceptorSD, nullptr, nullptr);
      if (sd1 < 0)
      {
        // Some error in "accept", but may be not really serious:
        if (errno == EINTR)
          // "accept" was interrupted by a signal, this is OK, just continue:
          continue;

        // Any other error:
        throw utxx::runtime_error
              ("ERROR: TCP_Acceptor::Run: accept failed: errno=", errno, ": ",
               strerror(errno));
      }
      //---------------------------------------------------------------------//
      // Accept Successful:                                                  //
      //---------------------------------------------------------------------//
      switch (m_cmodel)
      {
      //---------------------------------------------------------------------//
      case EConcurModel::Sequential:
      //---------------------------------------------------------------------//
        // XXX: Clients are serviced SEQUNTIALLY. If the currently-connected
        // client sends multiple reqs, all other clients will be locked out
        // until this one disconnects:
        try
        {
          //-----------------------------------------------------------------//
          // Run the ProtoDialogue:                                          //
          //-----------------------------------------------------------------//
          a_proto(sd1);
          (void) close(sd1);
        }
        // Log possible exceptions:
        catch (std::exception const& exn)
        {
          m_logger->error("EXCEPTION in ProtoDialogue: {}", exn.what());
        }
        catch (...)
        {
          m_logger->error("UNKNOWN EXCEPTION in ProtoDialogue");
        }
        break;

      //---------------------------------------------------------------------//
      case EConcurModel::Fork:
      //---------------------------------------------------------------------//
        if (fork() == 0)
        {
          // It is a CHILD process:
          try
          {
            // Good style -- close unneeded file descriptors inherited from the
            // parent:
            (void) close(m_acceptorSD);

            //---------------------------------------------------------------//
            // Run the ProtoDialogue:                                        //
            //---------------------------------------------------------------//
            a_proto(sd1);

            // Clean-up and exit:
            (void) close(sd1);
            exit(0);    // Successful completion of the child
          }
          // Log possible exceptions:
          catch (std::exception const& exn)
          {
            m_logger->error("EXCEPTION in ProtoDialogue(Child): {}",
                            exn.what());
            (void) close(sd1);
            exit(1);       // Child terminated with error
          }
          catch (...)
          {
            m_logger->error("UNKNOWN EXCEPTION in ProtoDialogue(Child)");
            (void) close(sd1);
            exit(1);       // Child terminated with error
          }
        }
        // If we got here, it is the PARENT process. Continue as usual:
        (void) close(sd1);
        break;

      //---------------------------------------------------------------------//
      default:
      //---------------------------------------------------------------------//
        throw utxx::logic_error
              ("ERROR: Unimplemented Concurrency Model: ",
              m_cmodel.to_string());
      }
    }
    // This point is unreachable:
  }
}
// End namespace Net
