// vim:ts=2:et
//===========================================================================//
//                              "Logger.hpp":												         //
//                    Logger Creation and Initialisation                     //
//===========================================================================//
#pragma once

#define  SPDLOG_USE_STD_FORMAT  // XXX: It would not compile without this!..
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <cassert>

namespace Net
{
  //=========================================================================//
  // "MkLogger":                                                             //
  //=========================================================================//
  inline std::shared_ptr<spdlog::logger> MkLogger(int argc, char* argv[])
  {
    assert(argc > 0 && argv != nullptr);

    // Get the  LogFileName from the command line. XXX This is NOT thread-safe!
    // Reset "optind" to 1 so the "getopt" loop may be invoked multiple times
    // from different places in the program:
    optind = 1;
    opterr = 0; // To avoid error msgs on unrecognised options
    char const* logFileName = nullptr;

    while (true)
    {
      int opt = getopt(argc, argv, "l:");
      if (UNLIKELY(opt < 0))
        break;    // No more opts...

      if (opt == 'l')
      {
        logFileName = optarg;
        break;
      }
    }
    if (logFileName == nullptr || *logFileName == '\0')
      throw utxx::badarg_error
            ("ERROR: Missing the LogFileName. Use the param:\n"
             "\t-l LogFileName");

    // Now extract the logger name (for internal use) from argv[0]. It should
    // not be empty, but if it is, it is not an error:
    char const* lastSep    = strrchr(argv[0], '/');
    char const* loggerName = (lastSep == nullptr) ? argv[0] : lastSep + 1;

    // OK:
    // MaxQueue=1024, 1 Thread, Thread-Safe:
    spdlog::init_thread_pool(1024, 1);

    std::shared_ptr<spdlog::logger> loggerShP =
      spdlog::rotating_logger_mt<spdlog::async_factory_nonblock>
      (
        loggerName,
        logFileName,
        1024 * 1024,        // LogFileSize=1M
        100                 // 100 log file rotations
      );
    loggerShP->set_pattern("%+", spdlog::pattern_time_type::utc);
    return loggerShP;
  }
}
// End namespace Net
