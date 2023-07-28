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
  //-------------------------------------------------------------------------//
  // "CallPx": Calculation of Call Option Px:                                //
  //-------------------------------------------------------------------------//
  double CallPx
  (
    double a_K,     
    double a_T,     
    double a_r,     
    double a_D,     
    double a_sigma, 
    double a_t,     
    double a_St     
  );

  //-------------------------------------------------------------------------//
  // "PutPx": Calculation of Put Option Px:                                  //
  //-------------------------------------------------------------------------//
  double PutPx
  (
    double a_K,     
    double a_T,     
    double a_r,     
    double a_D,     
    double a_sigma, 
    double a_t,     
    double a_St     
  );

  //-------------------------------------------------------------------------//
  // "BinaryCallPx": Calculation of Binary Call Option Px:                   //
  //-------------------------------------------------------------------------//
  double BinaryCallPx
  (
    double a_K,     
    double a_T,     
    double a_r,     
    double a_D,     
    double a_sigma, 
    double a_t,     
    double a_St     
  );

  //-------------------------------------------------------------------------//
  // "BinaryPutPx": Calculation of Binary Put Option Px:                     //
  //-------------------------------------------------------------------------//
  double BinaryPutPx
  (
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
