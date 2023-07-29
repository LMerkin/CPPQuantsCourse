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
                                 "[-S St, Current underlying Px]";
    int poType = 0;
    float K = NAN;
    float T = NAN;
    float r = NAN;
    float D = NAN;
    float sigma = NAN;
    float t = NAN;
    float St = NAN;

    int opt = 0; // For control getopt returned value
    while ((opt = getopt(argc, argv, ":p:K:T:r:D:s:t:S:")) != -1)
    {
      std::cout << "In cycle" << std::endl;

      switch (opt)
      {
      case 'p':
        poType = (atoi(optarg) > 0) ? atoi(optarg) : 0;
        printf("Read PayoffType strike Px: '%d'\n", poType);
        break;
      case 'K':
        K = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Read strike Px: '%f'\n", K);
        break;
      case 'T':
        T = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Read expiration time: '%f'\n", T);
        break;
      case 'r':
        r = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Read free interest rate: '%f'\n", r);
        break;
      case 'D':
        D = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Read divident rate: '%f'\n", D);
        break;
      case 's':
        sigma = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Read divident rate: '%f'\n", sigma);
        break;
      case 't':
        t = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Current time: %f\n", t);
        break;
      case 'S':
        St = (atof(optarg) > 0) ? atof(optarg) : 0;
        printf("Current Px underlying: %f\n", St);
        break;
      case ':':
        fprintf(stderr, "Missing argument value. Option [%c] need a value\n", optopt);
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
    if (optind < argc)
    {
      printf("non-option ARGV-elements: ");
      while (optind < argc)
        printf("%s ", argv[optind++]);
      printf("\n");
      return 5;
    }
    std::cout << "Hmmm...." << std::endl;

    double callPx = BSM::CallPx(BSM::PayoffType(poType), K, T, r, D, sigma, t, St);
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
