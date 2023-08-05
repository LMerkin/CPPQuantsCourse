// vim:ts=2:et
//===========================================================================//
//                                "ProcessHTTPReqs.h":                       //
//        Processing HTTP Requests in an Established Client Connection       //
//===========================================================================//
#pragma once

//---------------------------------------------------------------------------//
// "ProcessHTTPReqs": Dealing with an HTTP Client via a Socket Descr:        //
//---------------------------------------------------------------------------//
#ifdef __cplusplus
extern "C" int ProcessHTTPReqs(int a_sd);
#else
extern     int ProcessHTTPReqs(int a_sd);
#endif
