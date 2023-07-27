// vim:ts=2:et
//===========================================================================//
//                            "OptionPricer.cpp":                            //
//                   Command-Line Option Price Calculator                    //
//===========================================================================//
#include "BSM.h"
#include <iostream>
#include <cstdlib>
#include <getopt.h>

using namespace std;

int main(int argc, char* argv[])
{
  // We must have argc==8: path to this executable + 7 BSM params:
  /* 
  if (argc != 8)
  {
    cerr << "REQUIRED PARAMS: K T r D sigma t St" << endl;
    return 1;     // Return an error code by convention
  }
  */

  const char * const help_str="[-K Strike Px]"
                              "[-T Time to expiration]"
                              "[-r Free interest rate]"
                              "[-D Dividend rate]"
                              "[-s sigma]"
                              "[-t current time]"
                              "[-S St, Current underlying Px]";
  float K     = 0;
  float T     = 0;
  float r     = 0;
  float D     = 0;
  float sigma = 0;
  float t     = 0;
  float St    = 0;

  int opt = 0; // For control getopt returned value
  while ((opt = getopt(argc, argv,":K:T:r:D:s:t:S:"))!=-1){
      switch(opt){
        case 'K':
	 K = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Read strike Px: '%f'\n", K);
         break;
        case 'T':
	 T = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Read expiration time: '%f'\n", T);
	 break;
        case 'r':
	 r = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Read free interest rate: '%f'\n", r);
	 break;
        case 'D':
	 D = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Read divident rate: '%f'\n", D);
	 break;
        case 's':
	 sigma = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Read divident rate: '%f'\n", sigma);
	 break;
        case 't':
	 t = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Current time: %f\n", t);
	 break;
        case 'S':
	 St = (atof(optarg)>0) ? atof(optarg):0;
	 printf("Current Px underlying: %f\n", St);
	 break;
	case ':':
	 fprintf(stderr,"Missing argument value. Option [%c] need a value\n", optopt);
	 exit(EXIT_FAILURE);
	case '?':
	 printf("Unknown argument [%c]\n", optopt);
	 printf("Usage: %s %s\n", argv[0], help_str);
	 exit(EXIT_FAILURE);
	default:
	 printf("Parsing arguments...something went wrong...'%s'\n", optarg);
	 printf("Usage: %s %s\n", argv[0], help_str);
	 exit(EXIT_FAILURE);
      }
  }

  double  callPx = BSM::CallPx(K, T, r, D, sigma, t, St);
  cout << callPx << endl;
  return 0;
}
