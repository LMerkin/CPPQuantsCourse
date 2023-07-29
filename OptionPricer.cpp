// vim:ts=2:et
//===========================================================================//
//                            "OptionPricer.cpp":                            //
//                   Command-Line Option Price Calculator                    //
//===========================================================================//
#include "BSM.h"
#include <iostream>
#include <cstdlib>
#include <stdexcept>

using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    // We must have argc==9: path to this executable + 7 BSM params:
    if (argc != 9)
    {
      cerr << "REQUIRED PARAMS: PayoffType K T r D sigma t St" << endl;
      return 1;     // Return an error code by convention
    }
    int    poType = atoi(argv[1]);  // 1=Call, 2=Put
    double K      = atof(argv[2]);
    double T      = atof(argv[3]);
    double r      = atof(argv[4]);
    double D      = atof(argv[5]);
    double sigma  = atof(argv[6]);
    double t      = atof(argv[7]);
    double St     = atof(argv[8]);

    double  px = BSM::Px(BSM::PayoffType(poType), K, T, r, D, sigma, t, St);
    cout << px << endl;
    return 0;
  }
  catch (std::exception const& exn)
  {
    cerr << "EXCEPTION: " << exn.what() << endl;
    return 1;
  }
  catch (...)
  {
    cerr << "UNKNOWN EXCEPTION" << endl;
    return 2;
  }
}
