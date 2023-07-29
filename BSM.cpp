#include "BSM.h"
#include <algorithm>
#include <stdexcept>
#include <cassert>

namespace BSM
{
  double PriceOption
  (
    OptionType optionType,
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
    double d1, d2, phi1, phi2, px;

    switch(optionType)
    {
      case OptionType::Call:
        if (tau == 0.0)
          return std::max(a_St - a_K, 0.0);
        d1   = (x + (a_r - a_D + 0.5 * a_sigma * a_sigma) * tau) / s;
        d2   = d1 - s;
        phi1 = Phi(d1);
        phi2 = Phi(d2);
        px   = a_St * exp(-a_D * tau) * phi1 - a_K  * exp(-a_r * tau) * phi2;
        assert(px > 0.0);
        return px;

      case OptionType::Put:
        if (tau == 0.0)
          return std::max(a_K - a_St, 0.0);
        d1   = (x + (a_r - a_D + 0.5 * a_sigma * a_sigma) * tau) / s;
        d2   = d1 - s;
        phi1 = Phi(-d1);
        phi2 = Phi(-d2);
        px   = a_K * exp(-a_r * tau) * phi2 - a_St * exp(-a_D * tau) * phi1;
        assert(px > 0.0);
        return px;

      case OptionType::BinaryCall:
        d2   = (x + (a_r - a_D - 0.5 * a_sigma * a_sigma) * tau) / s;
        phi2 = Phi(d2);
        px   = exp(-a_r * tau) * phi2;
        assert(px >= 0.0);
        return px;

      case OptionType::BinaryPut:
        d2   = (x + (a_r - a_D - 0.5 * a_sigma * a_sigma) * tau) / s;
        phi2 = Phi(-d2);
        px   = exp(-a_r * tau) * phi2;
        assert(px >= 0.0);
        return px;

      default:
        throw std::invalid_argument("Invalid Option Type");
    }
  }
}
