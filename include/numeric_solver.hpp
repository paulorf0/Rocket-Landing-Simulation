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
    double h = func(x) / derivFunc(x);
    while (abs(h) >= EPSILON) {
      h = func(x) / derivFunc(x);
      x = x - h;
    }

    return x;
  }
};