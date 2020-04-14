// [[Rcpp::plugins(cpp11)]]
#ifndef __MACROS_INCLUDED__
#define __MACROS_INCLUDED__

// We swap out some commonly used error and message funtions depending on if this
// code is being compiled with RCPP available or not. When RCPP is being used ot
// compile the code these functions make sure messages are properly passed to R.
// #if NO_RCPP
#ifndef BEGIN_RCPP
#include <iostream>
#define LOGIC_ERROR(msg) throw std::logic_error(msg)
#define RANGE_ERROR(msg) throw std::range_error(msg)
#define WARN_ABOUT(msg) std::cerr << std::string(msg) << std::endl
#define OUT_MSG std::cout
#define ALLOW_USER_BREAKOUT

using InOut_String_Vec = std::vector<std::string>;
using InOut_Int_Vec    = std::vector<int>;
using InOut_Double_Vec = std::vector<double>;

// Just pass a string through directly if in c++ only mode
inline const std::string& to_str(const std::string& str)
{
  return str;
}

inline const std::vector<std::string>& to_str_vec(const InOut_String_Vec& in_vec)
{
  return in_vec;
}

#else
#include <Rcpp.h>
// Eases the process of wrapping functions to get errors forwarded to R
#define LOGIC_ERROR(msg) throw Rcpp::exception(std::string(msg).c_str(), false)
#define RANGE_ERROR(msg) throw Rcpp::exception(std::string(msg).c_str(), false)
#define WARN_ABOUT(msg) Rcpp::warning(std::string(msg).c_str())
#define OUT_MSG Rcpp::Rcout
#define ALLOW_USER_BREAKOUT Rcpp::checkUserInterrupt()

using InOut_String_Vec = Rcpp::CharacterVector;
using InOut_Int_Vec    = Rcpp::IntegerVector;
using InOut_Double_Vec = Rcpp::NumericVector;

// Just pass a string through directly if in c++ only mode
template <typename String_Like>
inline const std::string to_str(const String_Like& str)
{
  return std::string(str);
}

inline const std::vector<std::string> to_str_vec(const InOut_String_Vec& in_vec)
{
  std::vector<std::string> out;
  out.reserve(in_vec.size());
  for (int i = 0; i < in_vec.size(); i++) out.push_back(std::string(in_vec[i]));
  return out;
}

#endif

#endif