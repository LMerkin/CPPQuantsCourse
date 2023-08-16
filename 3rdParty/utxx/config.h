// Level1 cacheline size
#define UTXX_CL_SIZE 64


// Define to 1 if you have `z' library (-lz)
#define UTXX_HAVE_LIBZ

// Define to 1 if Thrift library is available
/* #undef UTXX_HAVE_THRIFT_H */

// This is needed for Thrift:
#ifndef HAVE_INTTYPES_H
#define HAVE_INTTYPES_H
#endif
#ifndef HAVE_NETINET_IN_H
#define HAVE_NETINET_IN_H
#endif

// Define to 1 if enum.h and enumx.h should be friends of boost::serialization::access
/* #undef UTXX_ENUM_SUPPORT_SERIALIZATION */

#define UTXX_HAVE_BOOST_TIMER_TIMER_HPP

// Define to 1 if struct tcphdr has th_flags
/* #undef UTXX_HAVE_TCPHDR_TH_FLAGS_H */

