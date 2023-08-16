// vim:ts=2:et
//===========================================================================//
//                               "TCP_Acceptor.h":                           //
//===========================================================================//
#pragma once
#include <utxx/enum.hpp>

// Fwd declaration (to reduce compiule time):
namespace spdlog
{
  class logger;
}

namespace Net
{
  //=========================================================================//
  // "TCP_Acceptor" Class:                                                   //
  //=========================================================================//
  class TCP_Acceptor
  {
  public:
    //-----------------------------------------------------------------------//
    // Concurrency Models:                                                   //
    //-----------------------------------------------------------------------//
    UTXX_ENUM
    (
      EConcurModel, int,
      Sequential,              // No concurrency
      Fork,                    // Create a new process for each TCP client
      Thread,                  // Spawn  a new thread  for each TCP client
      ThreadPool               // Re-use existing threads
    )

  private:
    //-----------------------------------------------------------------------//
    // Data Flds:                                                            //
    //-----------------------------------------------------------------------//
    int             const m_acceptorSD; // Acceptor Socket Descriprtor
    EConcurModel    const m_cmodel;
    spdlog::logger* const m_logger;     // PTR NOT OWNED!
    static bool           s_exitRun;    // NB: "static", i.e. class-wide!

  public:
    //-----------------------------------------------------------------------//
    // Ctors, Dtor:                                                          //
    //-----------------------------------------------------------------------//
    // Default Ctor is deleted: Does not make sense:
    TCP_Acceptor() = delete;

    // Copy Ctor is deleted as well:
    TCP_Acceptor(TCP_Acceptor const&) = delete;

    // Non-Default Ctor:
    // Port and IPAddr are given explicitly:
    TCP_Acceptor
    (
      int             a_port,
      spdlog::logger* a_logger,
      char const*     a_acceptor_ip = nullptr,
      EConcurModel    a_cmodel      = EConcurModel::Sequential
    );

    // Non-Default Ctor:
    // Port and IPAddr (optional) are given via command-line args:
    // -p Port [-i IPAddr] [-c Sequential|Fork|Thread|ThreadPool]
    // XXX: NOT THREAD SAFE!
    TCP_Acceptor(int argc, char* argv[], spdlog::logger* a_logger);

    // Dtor:
    ~TCP_Acceptor();

    //-----------------------------------------------------------------------//
    // "Run":                                                                //
    //-----------------------------------------------------------------------//
    // Runs a conceptually-infinite acceptor loop:
    // a_proto :: void(int)
    //
    template<typename ProtoDialogue>
    void Run(ProtoDialogue& a_proto);

  private:
    //-----------------------------------------------------------------------//
    // Signal Handler:                                                       //
    //-----------------------------------------------------------------------//
    static void SigHandler(int a_signum);
  };
}
// End namespace Net
