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
  // "Px":                                                                   //
  //-------------------------------------------------------------------------//
  double Px
  (
    // Option Spec:
    PayoffType a_type,
    double     a_K, // Option Strike
    double     a_T, // Opton Expiration Time, as Year Fraction, e.g. 2023.xxx
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

    if (a_K <= 0.0 || a_St <= 0.0 || a_sigma <= 0.0)
      throw std::invalid_argument
            ("Non-Positive Strike / UnderlyingPx / Vol");

    // Generic Case:
    double px = NAN;

    switch (a_type)
    {
      // Call:
      case PayoffType::Call:
      {
        if (tau == 0.0)
          // At expiration time, return the PayOff:
          return std::max(a_St - a_K, 0.0);

        double x    = log (a_St / a_K);
        double s    = a_sigma * sqrt(tau);
        double d1   = (x + (a_r - a_D + 0.5 * a_sigma * a_sigma) * tau) / s;
        double d2   = d1 - s;
        double phi1 = Phi(d1);
        double phi2 = Phi(d2);

        px   = a_St * exp(-a_D * tau) * phi1 -
               a_K  * exp(-a_r * tau) * phi2;
        break;
      }

      case PayoffType::Put:
      {
        if (a_D == 0.0)
          px = Px(PayoffType::Call, a_K, a_T, a_r, a_D, a_sigma, a_t, a_St)
               - a_St + a_K * exp(-a_r * tau);
        else
          throw std::logic_error("Unsupported: Put with Dividends");
        break;
      }

      default:
        throw std::logic_error("Unsupported PayoffType");
    }
    // Use assert to enforce (in the debug model only) logically-invariant
    // conditions:
    assert(px > 0.0);
    return px;
  }

// PUT-CALL PARITY:
// Call: max(S_T - K, 0)
// Put : max(K - S_T, 0)
// Call - Put: S_T < K :  0 - (K - S_T) = S_T - K
//             S_T >= K:  S_T - K - 0   = S_T - K
//
// (1) Portfolio:
// Long  Call(K, T)
// Short Put (K, T)
// At T: S_T - K
// At t: CallPx(K,T;t) - PutPx(K,T;t)
//
// (2) Portfolio:
// Long Underlying, now: S_t,                  at T: S_T
// Money Account,   now: -K * exp(-r * (T-t)), at T: -K
// At T: S_T - K
// At t: S_t - K * exp(-r * (T-t))
//
// ==> CallPx(K,T;t) - PutPx(K,T;t) = S_t - K * exp(-r * (T-t))
}
