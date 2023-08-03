//===========================================================================//
// vim:ts=2:et
//                            "OptionPricer.cpp":                            //
//                   Command-Line Option Price Calculator                    //
//===========================================================================//
#include "BSM.h"
#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
  try
  {
    const char *const help_str = "[-p PayoffType]"
                                 "[-K Strike Px]"
                                 "[-T Time to expiration]"
                                 "[-r Free interest rate]"
                                 "[-D Dividend rate]"
                                 "[-s sigma]"
                                 "[-t current time]"
                                 "[-S St, Current underlying Px]"
                                 "[-P Pr, Fixed premium (for binay option only)]";

    int poType = 0;
    double K = NAN;
    double T = NAN;
    double r = NAN;
    double D = NAN;
    double sigma = NAN;
    double t = NAN;
    double St = NAN;

    // For binary option only
    double Pr = NAN;

    int opt = 0; // For control getopt returned value
    while ((opt = getopt(argc, argv, ":p:K:T:r:D:s:t:S:P:")) != -1)
    {
      switch (opt)
      {
      case 'p':
        poType = (atoi(optarg) > 0) ? atoi(optarg) : 0;
        // cout << "Read PayoffType: " << poType << endl;
        break;
      case 'K':
        K = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read strike Px: " << K << endl;
        break;
      case 'T':
        T = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read expiration time: " << T << endl;
        break;
      case 'r':
        r = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read free interest rate: " << r << endl;
        break;
      case 'D':
        D = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read divident rate: " << D << endl;
        break;
      case 's':
        sigma = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read sigma: " << sigma << endl;
        break;
      case 't':
        t = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Current time: " << t << endl;
        break;
      case 'S':
        St = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Current Px underlying in time t: " << St << endl;
        break;
      case 'P':
        Pr = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read fixed premium for digital option only: " << Pr << endl;
        break;
      case ':':
        cerr << "Missing argument value. Option -" << static_cast<char>(optopt) << " need a value" << endl;
        exit(EXIT_FAILURE);
      case '?':
        cerr << "Unknown argument " << static_cast<char>(optopt) << endl;
        cerr << "Usage: " << argv[0] << " " << help_str << endl;
        exit(EXIT_FAILURE);
      default:
        cerr << "Parsing arguments...something went wrong..." << optarg << endl;
        cerr << "Usage: " << argv[0] << " " << help_str << endl;
        exit(EXIT_FAILURE);
      }
    }
    if (optind < argc)
    {
      cout << "non-option ARGV-elements: " << endl;
      while (optind < argc)
        cout << argv[optind++] << " " << endl;
      cout << endl;
      return 5;
    }

    double callPx = BSM::Px(BSM::PayoffType(poType), K, T, r, D, sigma, t, St, Pr);
    cout << callPx << endl;
    return 0;
  }
  catch (std::exception const &exp)
  {
    cerr << "EXCEPTION: " << exp.what() << endl;
    return 1;
  }
  catch (...)
  {
    cerr << "UNKNOWN EXCEPTION" << endl;
    return 2;
  }
}