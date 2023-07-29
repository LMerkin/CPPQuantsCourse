// vim:ts=2:et
//===========================================================================//
//                                   "BSM.h":                               //
//                Black-Scholes-Merton Option Pricing Functions              //
//===========================================================================//
#pragma once
#include <cmath>

// Abbreviations:
// Px  -- price (prix)
// Ccy -- currency
// Qty -- quantity

namespace BSM
{
  enum class OptionType {
    Undifined  = 0,
    Call       = 1,
    Put        = 2,
    BinaryCall = 3,
    BinaryPut  = 4,
    Any        = 100
  };

  //-------------------------------------------------------------------------//
  // "PriceOption": Calculation of Option Px based on the option type:       //
  //-------------------------------------------------------------------------//
  double PriceOption
  (
    OptionType optionType,
    double a_K,     
    double a_T,     
    double a_r,     
    double a_D,     
    double a_sigma, 
    double a_t,     
    double a_St     
  );

  //-------------------------------------------------------------------------//
  // "Phi": Standard Normal CDF:                                             //
  //-------------------------------------------------------------------------//
  inline double Phi(double a_x)
    { return 0.5 * (1.0 + erf(a_x * M_SQRT1_2)); }
}
// End namespace BSM
