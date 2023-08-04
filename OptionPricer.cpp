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
    const char *const help_str = "[-p PayoffType:{1|2|3|4|180}]\n"
                                 "[-K Strike Px]\n"
                                 "[-T Time to expiration]\n"
                                 "[-r Free interest rate]\n"
                                 "[-D Dividend rate]\n"
                                 "[-s sigma]\n"
                                 "[-t current time]\n"
                                 "[-S St, Current underlying Px]\n"
                                 "[-P Pr, Fixed premium (for binay option only)]\n";

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
    while ((opt = getopt(argc, argv, ":p:K:T:r:D:s:t:S:P:h")) != -1)
    {
      switch (opt)
      {
      case 'p':
        poType = (atoi(optarg) > 0) ? atoi(optarg) : 0;
        // cout << "Read PayoffType: " << poType << endl;
        if (poType == 0)
        {
          cerr << "Parameter -p: {1|2|3|4}" << endl;
          return -1;
        }
        break;
      case 'K':
        K = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read strike Px: " << K << endl;
        if (K == 0)
        {
          cerr << "Parameter -K must be defined and greater 0" << endl;
          return -2;
        }
        break;
      case 'T':
        T = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read expiration time: " << T << endl;
        if (T == 0)
        {
          cerr << "Parameter -T must be defined and greater 0" << endl;
          return -3;
        }
        break;
      case 'r':
        r = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read free interest rate: " << r << endl;
        if (r == 0)
        {
          cerr << "INFO: Free interest rate set to 0" << endl;
        }
        break;
      case 'D':
        D = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read divident rate: " << D << endl;
        if (D == 0)
        {
          cerr << "INFO: Dividend rate set to 0" << endl;
        }
        break;
      case 's':
        sigma = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read sigma: " << sigma << endl;
        if (sigma == 0)
        {
          cerr << "Parameter -s must be defined and greater 0" << endl;
          return -3;
        }
        break;
      case 't':
        t = (atof(optarg) > 0) ? atof(optarg) : 0;
        if (t == 0)
        {
          cerr << "Parameter -t must be defined and greater 0" << endl;
          return -4;
        }
        // cout << "Current time: " << t << endl;
        break;
      case 'S':
        St = (atof(optarg) > 0) ? atof(optarg) : 0;
        if (St == 0)
        {
          cerr << "Parameter -S must be defined and greater 0" << endl;
          return -5;
        }
        // cout << "Current Px underlying in time t: " << St << endl;
        break;
      case 'P':
        Pr = (atof(optarg) > 0) ? atof(optarg) : 0;
        // cout << "Read fixed premium for digital option only: " << Pr << endl;
        break;
      case 'h':
        cerr << "Usage: " << argv[0] << " " << help_str << endl;
        return -6;
        break;
      case ':':
        if (static_cast<char>(optopt) == 'P')
        {
          Pr = 0.0;
          break;
        }
        else
        {
          cerr << "Missing argument value. Option -" << static_cast<char>(optopt) << " need a value" << endl;
          return -7;
        }
      case '?':
        cerr << "Unknown argument " << static_cast<char>(optopt) << endl;
        cerr << "Usage: " << argv[0] << " " << help_str << endl;
        return -8;
      default:
        cerr << "Parsing arguments...something went wrong..." << optarg << endl;
        cerr << "Usage: " << argv[0] << " " << help_str << endl;
        return -9;
      }
    }

    if (optind == 1)
    {
      cerr << "Usage: " << argv[0] << endl
           << help_str << endl;
      return -10;
    }

    if (optind < argc)

    {
      cerr << "Expected argument after option" << endl;
      cerr << "Usage: " << argv[0] << " " << help_str << endl;
      return -11;
    }

    double Px = BSM::Px(BSM::PayoffType(poType), K, T, r, D, sigma, t, St, Pr);
    cout << Px << endl;
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