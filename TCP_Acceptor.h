// vim:ts=2:et
//===========================================================================//
//                               "TCP_Acceptor.h":                           //
//===========================================================================//
#pragma once

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
    enum class EConcurModel: int
    {
      Sequential = 0,          // No concurrency
      Fork       = 1,          // Create a new process for each TCP client
      Thread     = 2,          // Spawn  a new thread  for each TCP client
      ThreadPool = 3           // Re-use existing threads
    };

  private:
    //-----------------------------------------------------------------------//
    // Data Flds:                                                            //
    //-----------------------------------------------------------------------//
    int          m_acceptorSD; // Acceptor Socket Descriprtor
    EConcurModel m_cmodel;

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
      int          a_port,
      char const*  a_acceptor_ip = nullptr,
      EConcurModel a_cmodel      = EConcurModel::Sequential
    );

    // Non-Default Ctor:
    // Port and IPAddr (optional) are given via command-line args:
    // -p Port [-i IPAddr] [-c Sequential|Fork|Thread|ThreadPool]
    // XXX: NOT THREAD SAFE!
    TCP_Acceptor(int argc, char* argv[]);

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
  };
}
// End namespace Net
