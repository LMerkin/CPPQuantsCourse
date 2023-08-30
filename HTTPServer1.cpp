// vim:ts=2:et
//===========================================================================//
//                               "HTTPServer1.cpp":                          //
//                               Simple HTTP Server                          //
//===========================================================================//
#include "TCP_Acceptor.hpp"
#include "HTTPProtoDialogue.hpp"
#include "Echo.hpp"

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
  //-------------------------------------------------------------------------//
  // Create the Logger: MaxQueue=1024, 1 Thread, Thread-Safe:                //
  //-------------------------------------------------------------------------//
  char const* logFileName = "HTTPServer1";

  // TODO: Get non-default "logFileName" from argc/argv using "getopts"

  spdlog::init_thread_pool(1024, 1);
  std::shared_ptr<spdlog::logger> loggerShP =
    spdlog::rotating_logger_mt<spdlog::async_factory_nonblock>
    (
      "HTTPServer1",      // LoggerName
      logFileName,
      1024 * 1024,        // LogFileSize=1M
      100                 // 100 log file rotations
    );
  spdlog::logger* logger = loggerShP.get();

  assert(logger != nullptr);
  logger->set_pattern("%+", spdlog::pattern_time_type::utc);

  //-------------------------------------------------------------------------//
  // Instantiate the TCP Acceptor:                                           //
  //-------------------------------------------------------------------------//
  Net::TCP_Acceptor acc(argc, argv, logger);

  // Instantiate the HTTPProtoDialohue with UserAction = Echo:
  Net::Echo echo;
  Net::HTTPProtoDialogue<Net::Echo> http(echo, logger);

  //-------------------------------------------------------------------------//
  // Run it!                                                                 //
  //-------------------------------------------------------------------------//
  acc.Run(http);
  return 0;
}
