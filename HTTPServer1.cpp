// vim:ts=2:et
//===========================================================================//
//                               "HTTPServer1.cpp":                          //
//                               Simple HTTP Server                          //
//===========================================================================//
#include "TCP_Acceptor.hpp"
#include "HTTPProtoDialogue.hpp"
#include "Echo.hpp"
#include "Logger.hpp"
#include <stdexcept>

// For logging:
#define  SPDLOG_USE_STD_FORMAT
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>

//===========================================================================//
// "main":                                                                   //
//===========================================================================//
int main(int argc, char* argv[])
{
  try
  {
    //-----------------------------------------------------------------------//
    // Create the Logger:                                                    //
    //-----------------------------------------------------------------------//
    std::shared_ptr<spdlog::logger> loggerShP = Net::MkLogger(argc, argv);
    spdlog::logger*                 logger    = loggerShP.get();
    assert(logger != nullptr);

    //-----------------------------------------------------------------------//
    // Instantiate the TCP Acceptor:                                         //
    //-----------------------------------------------------------------------//
    Net::TCP_Acceptor acc(argc, argv, logger);

    // Instantiate the HTTPProtoDialohue with UserAction = Echo:
    Net::Echo echo;
    Net::HTTPProtoDialogue<Net::Echo> http(echo, logger);

    //-----------------------------------------------------------------------//
    // Run it!                                                               //
    //-----------------------------------------------------------------------//
    acc.Run(http);
    return 0;
  }
  catch (std::exception const& exn)
  {
    std::cerr << "EXCEPTION: " << exn.what() << std::endl;
    return 1;
  }
  catch (...)
  {
    std::cerr << "UNKNOWN EXCEPTION" << std::endl;
    return 1;
  }
  // We will never get here:
  __builtin_unreachable();
}
