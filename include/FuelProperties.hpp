#pragma once

struct FuelProperties {
  const double Ru = 8314.; // J / (kmol * K)

  double T0 = 0; // Fuel temperature (Like 300K - kelvin).
  double M = 0;  // kg/kmol
  double R = 0;

  void calculateR() { R = Ru / M; }
};