#pragma once

// Source = https://www.geeksforgeeks.org/dsa/program-for-newton-raphson-method/
#include <bits/stdc++.h>
#include <cmath>
#include <functional>

using namespace std;

struct Newton_Raphson {
  double EPSILON;
  function<double(double)> func;
  function<double(double)> derivFunc;

  void setPrecision(double epsilon) { EPSILON = epsilon; }

  void setFunc(function<double(double)> f, function<double(double)> df) {
    func = f;
    derivFunc = df;
  }

  double solve(double x) {
    if (std::abs(x) < 1e-5)
      x = 2.0;

    double h = func(x) / derivFunc(x);

    while (abs(h) >= EPSILON) {
      double df = derivFunc(x);

      if (std::abs(df) < 1e-9) {
        x += 0.1;
        df = derivFunc(x);
      }

      h = func(x) / df;
      x = x - h;
    }

    if (std::isnan(x) || std::isinf(x))
      return 1.0;

    return x;
  }
};