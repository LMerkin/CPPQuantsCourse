// vim:ts=2:et
//===========================================================================//
//                                 "BSM.cpp":                                //
//       Black-Scholes-Merton Option Pricing Formulas: Implementation        //
//===========================================================================//
#include "BSM.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace BSM
{
  //-------------------------------------------------------------------------//
  // "CallPx":                                                               //
  //-------------------------------------------------------------------------//
  double CallPx
  (
    // Option Spec:
    double a_K,     // Option Strike
    double a_T,     // Opton Expiration Time, as Year Fraction, e.g. 2023.xxx
    // Market Data:
    double a_r,     // Risk-Free Interest Rate (for the Numeraire Ccy)
    double a_D,     // Dividend Rate (Risk-Free Ineterst Rate for Foreign Ccy)
    double a_sigma, // Implied Volatility
    // "Quick" variables:
    double a_t,     // Pricing Time (as Year Fraction)
    double a_St     // Underlying Px at Time "a_t"
  )
  {
    // Time to expiration:
    double tau = a_T - a_t;

    if (tau <  0.0)
      throw std::invalid_argument("Negative Time to Expiration");

    if (tau == 0.0)
      // At expiration time, return the PayOff:
      return std::max(a_St - a_K, 0.0);

    if (a_K <= 0.0 || a_St <= 0.0 || a_sigma <= 0.0)
      throw std::invalid_argument
            ("Non-Positive Strike / UnderlyingPx / Vol");

    // Generic Case:
    double x    = log (a_St / a_K);
    double s    = a_sigma * sqrt(tau);
    double d1   = (x + (a_r - a_D + 0.5 * a_sigma * a_sigma) * tau) / s;
    double d2   = d1 - s;
    double phi1 = Phi(d1);
    double phi2 = Phi(d2);

    double px   = a_St * exp(-a_D * tau) * phi1 -
                  a_K  * exp(-a_r * tau) * phi2;
    // Use assert to enforce (in the debug model only) logically-invariant
    // conditions:
    assert(px > 0.0);
    return px;
  }
  //-------------------------------------------------------------------------//
  // "PutPx ":                                                               //
  //-------------------------------------------------------------------------//
    double PutPx
  (
    double a_K,     // Option Strike
    double a_T,     // Option Expiration Time, as Year Fraction, e.g. 2023.xxx
    double a_r,     // Risk-Free Interest Rate (for the Numeraire Ccy)
    double a_D,     // Dividend Rate (Risk-Free Interest Rate for Foreign Ccy)
    double a_sigma, // Implied Volatility
    double a_t,     // Pricing Time (as Year Fraction)
    double a_St     // Underlying Px at Time "a_t"
  )
  {
    double tau = a_T - a_t;

    if (tau <  0.0)
      throw std::invalid_argument("Negative Time to Expiration");

    if (tau == 0.0)
      return std::max(a_K - a_St, 0.0);

    if (a_K <= 0.0 || a_St <= 0.0 || a_sigma <= 0.0)
      throw std::invalid_argument("Non-Positive Strike / UnderlyingPx / Vol");

    double x    = log (a_St / a_K);
    double s    = a_sigma * sqrt(tau);
    double d1   = (x + (a_r - a_D + 0.5 * a_sigma * a_sigma) * tau) / s;
    double d2   = d1 - s;
    double phi1 = Phi(-d1);
    double phi2 = Phi(-d2);

    double px   = a_K * exp(-a_r * tau) * phi2 -
                  a_St * exp(-a_D * tau) * phi1;

    assert(px > 0.0);
    return px;
  }
  //-------------------------------------------------------------------------//
  // "BinaryCallPx ":                                                        //
  //-------------------------------------------------------------------------//
    double BinaryCallPx
  (
    double a_K,     // Option Strike
    double a_T,     // Option Expiration Time, as Year Fraction, e.g. 2023.xxx
    double a_r,     // Risk-Free Interest Rate (for the Numeraire Ccy)
    double a_D,     // Dividend Rate (Risk-Free Interest Rate for Foreign Ccy)
    double a_sigma, // Implied Volatility
    double a_t,     // Pricing Time (as Year Fraction)
    double a_St     // Underlying Px at Time "a_t"
  )
  {
    double tau = a_T - a_t;

    if (tau <  0.0)
      throw std::invalid_argument("Negative Time to Expiration");

    if (a_K <= 0.0 || a_St <= 0.0 || a_sigma <= 0.0)
      throw std::invalid_argument("Non-Positive Strike / UnderlyingPx / Vol");

    double x    = log (a_St / a_K);
    double s    = a_sigma * sqrt(tau);
    double d2   = (x + (a_r - a_D - 0.5 * a_sigma * a_sigma) * tau) / s;
    double phi2 = Phi(d2);

    double px   = exp(-a_r * tau) * phi2;

    assert(px >= 0.0);
    return px;
  }
  //-------------------------------------------------------------------------//
  // "BinaryPutPx ":                                                         //
  //-------------------------------------------------------------------------//
  double BinaryPutPx
  (
    double a_K,     // Option Strike
    double a_T,     // Option Expiration Time, as Year Fraction, e.g. 2023.xxx
    double a_r,     // Risk-Free Interest Rate (for the Numeraire Ccy)
    double a_D,     // Dividend Rate (Risk-Free Interest Rate for Foreign Ccy)
    double a_sigma, // Implied Volatility
    double a_t,     // Pricing Time (as Year Fraction)
    double a_St     // Underlying Px at Time "a_t"
  )
  {
    double tau = a_T - a_t;

    if (tau <  0.0)
      throw std::invalid_argument("Negative Time to Expiration");

    if (a_K <= 0.0 || a_St <= 0.0 || a_sigma <= 0.0)
      throw std::invalid_argument("Non-Positive Strike / UnderlyingPx / Vol");

    double x    = log (a_St / a_K);
    double s    = a_sigma * sqrt(tau);
    double d2   = (x + (a_r - a_D - 0.5 * a_sigma * a_sigma) * tau) / s;
    double phi2 = Phi(-d2);

    double px   = exp(-a_r * tau) * phi2;

    assert(px >= 0.0);
    return px;
  }


}
