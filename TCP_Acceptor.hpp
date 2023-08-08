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
#include <stdexcept>

namespace Net
{
	//=========================================================================//
	// "TCP_Acceptor::Run":																										 //
	//=========================================================================//
	template<typename ProtoDialogue>
  void TCP_Acceptor::Run(ProtoDialogue& a_proto)
	{
  	while (true)
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
	      throw std::runtime_error
      			  ("ERROR: TCP_Acceptor::Run: accept failed: " +
							 std::string(strerror(errno)));
			}
			//---------------------------------------------------------------------//
			// Accept Successful:																									 //
			//---------------------------------------------------------------------//
			try
			{
				switch (m_cmodel)
				{
				case EConcurModel::Sequential:
		    	// XXX: Clients are serviced SEQUNTIALLY. If the currently-connected
		    	// client sends multiple reqs, all other clients will be locked out
		    	// until this one disconnects:
    			a_proto(sd1);
					break;

				default:
					throw std::logic_error("ERROR: Unimplemented Concurrency Model");
				}
			}
			catch (std::exception const& exn)
			{
				std::cerr << "EXCEPTION in Acceptor Loop: " << exn.what() << std::endl;
			}
		}
	}
}
// End namespace Net
