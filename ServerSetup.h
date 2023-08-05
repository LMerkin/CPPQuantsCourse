// vim:ts=2:et
//===========================================================================//
//                                "ServerSetup.h":                           //
//                          Common Setup for TCP Servers                     //
//===========================================================================//
#pragma once

//---------------------------------------------------------------------------//
// "ServerSetup": Returns the Acceptor Socket, or (-1) on error:             //
//---------------------------------------------------------------------------//
#ifdef __cplusplus
extern "C" int ServerSetup(int argc, char* argv[]);
#else
extern     int ServerSetup(int argc, char* argv[]);
#endif
