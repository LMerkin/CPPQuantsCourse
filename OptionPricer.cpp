// vim:ts=2:et
//===========================================================================//
//                            "OptionPricer.cpp":                            //
//                   Command-Line Option Price Calculator                    //
//===========================================================================//
#include "BSM.h"
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char* argv[])
{
  // We must have argc==8: path to this executable + 7 BSM params:
  if (argc != 8)
  {
    cerr << "REQUIRED PARAMS: K T r D sigma t St" << endl;
    return 1;     // Return an error code by convention
  }
  double K     = atof(argv[1]);
  double T     = atof(argv[2]);
  double r     = atof(argv[3]);
  double D     = atof(argv[4]);
  double sigma = atof(argv[5]);
  double t     = atof(argv[6]);
  double St    = atof(argv[7]);

  double  callPx = BSM::CallPx(K, T, r, D, sigma, t, St);
  cout << callPx << endl;
  return 0;
}
