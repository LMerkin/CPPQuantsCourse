// vim:ts=2:et
//===========================================================================//
//                                   "BSM.h":																 //
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
  //-------------------------------------------------------------------------//
  // "Option Payoff Types:                                                   //
  //-------------------------------------------------------------------------//
  enum class PayoffType : int
  {
    UNDEFINED = 0,
    Call = 1,
    Put = 2,
    DigitalCall = 3,
    DigitalPut = 4,
    Arbitrary = 180,
  };
  //-------------------------------------------------------------------------//
  // "CallPx": Calculation of Call Option Px:                                //
  //-------------------------------------------------------------------------//
  double CallPx(
      // Option Spec:
      PayoffType a_type, // Type of option
      double a_K,        // Option Strike
      double a_T,        // Opton Expiration Time, as Year Fraction, e.g. 2023.xxx
      // Market Data:
      double a_r,     // Risk-Free Interest Rate (for the Numeraire Ccy)
      double a_D,     // Dividend Rate (Risk-Free Ineterst Rate for Foreign Ccy)
      double a_sigma, // Implied Volatility
      // "Quick" variables:
      double a_t,  // Pricing Time (as Year Fraction)
      double a_St, // Underlying Px at Time "a_t"
      double a_Pr  // Fixed premium for digital options
  );
  //-------------------------------------------------------------------------//
  // "Phi": Standard Normal CDF:                                             //
  //-------------------------------------------------------------------------//
  // NB: If a function is fully-defined (with a body!)   in a header file,  it
  // must be declared "inline" to prevent linking errors from multiply-defined
  // symbols:
  //
  inline double Phi(double a_x)
  {
    return 0.5 * (1.0 + erf(a_x * M_SQRT1_2));
  }
}
// End namespace BSM
