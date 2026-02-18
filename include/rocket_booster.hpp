#pragma once

#include "FuelProperties.hpp"
#include "constants.hpp"
#include "numeric_solver.hpp"

#include <cmath>
#include <iostream>

/*
        This struct control the Rocket Booster.

        Define:
                Rocket Fuel Flow Rate (float output kg / s).
                                Nozzle Area. This is controllable. (float minAe,
  maxAe meter^2) Throat area. Inside the tank. (float minAt, maxAt meter^2)
  specific heat ratio in the tank (float gamma)
                                   // Fuel temperature (Like 300K - kelvin).
  (double fuelProperties.T0 = 0)
  Molar mass of fuel (double fuelProperties.M = 0
  kg/kmol);
*/
struct RocketBooster {
  // TODO: Ae and At can be modified by the player.

  // Constants.
  float delay = 0.6;

  // Defined By Player.
  float target_output;
  float curr_output;
  float gamma;

  float minAe, minAt;
  float maxAe, maxAt;

  float curr_Ae;
  float curr_At;
  float prev_Ae;

  // Defined By Equations
  float Mach;
  float last_know_Mach;
  float Pe;       // Exit Pressure
  float effecVel; // Effective Velocity
  float Vexit;    // Exit Velocity
  float Te;       // nozzle static temperature

  struct Newton_Raphson solver;
  struct FuelProperties fuelProperties;

  void initBooster() {
    fuelProperties.calculateR();
    target_output = curr_output;

    last_know_Mach = 0.;
    last_know_Mach = 0.;
    Mach = 0.;
    Vexit = 0.;
    Pe = AIR_PRESSURE;
  }

  float getForce() {
    updateVariables();

    const auto f = curr_output * Vexit + (Pe - AIR_PRESSURE) * curr_Ae;

    return f * PPM;
  }

  void update(float dt) { updateOutput(dt); }

  void updateOutput(float dt) {
    if (std::abs(curr_output - target_output) < 0.0001f) {
      curr_output = target_output;
      return;
    }

    curr_output += (target_output - curr_output) * delay * dt;
  }

  void controlOutput(float dOut) {
    target_output += dOut;
    if (target_output < 0)
      target_output = 0;
  }

  void updateVariables() {
    calculateMach();
    calculateExitPressure();
    calculateTe();
    calculateVexit();
    // calculateEffecVel();
  }

  void calculateMach() {
    if (curr_Ae == prev_Ae)
      return;
    prev_Ae = curr_Ae;

    const auto epsilon = curr_Ae / curr_At;
    const auto t1 = 2.f / (gamma + 1);
    const auto t2 = (gamma - 1) / 2.f;
    const auto expoent = (gamma + 1) / (2.f * (gamma - 1));

    const auto f = [&](double M) -> double {
      double m_abs = std::abs(M);
      return (t2 * std::pow(m_abs, expoent + 2.f) + std::pow(m_abs, expoent) -
              std::pow(epsilon, expoent) / t1);
    };

    const auto df = [&](double M) -> double {
      double m_abs = std::abs(M);
      return t2 * (expoent + 1) * std::pow(m_abs, expoent + 1.f) +
             expoent * std::pow(m_abs, expoent - 1);
    };

    solver.setPrecision(0.001f);
    solver.setFunc(f, df);

    auto x0 = 2;
    if (last_know_Mach > 0.0001f) {
      x0 = last_know_Mach;
    }

    Mach = solver.solve(x0);
  }

  void calculateExitPressure() {
    // TODO: How to optimize it.
    //  if (curr_Ae == prev_Ae)
    //    return;

    const auto t1 = (gamma - 1) / 2.;
    const auto expoent = -gamma / (gamma - 1);

    // Approximate pc = p0 (atmosphere)

    double P_chamber = AIR_PRESSURE * 20.0;
    Pe = P_chamber * std::pow(1. + t1 * Mach * Mach, expoent);
  }

  void calculateTe() {
    if (fuelProperties.T0 <= 0) {
      throw std::runtime_error("Fuel Temperature lower or equal 0");
    }

    const double t1 = (gamma - 1) / 2.;

    Te = fuelProperties.T0 / (1. + t1 * Mach * Mach);
  }

  void calculateVexit() {
    if (fuelProperties.R <= 0) {
      throw std::runtime_error("FuelProperties.R lower or equal 0");
    }

    Vexit = Mach * std::sqrt(gamma * fuelProperties.R * Te);
  }

  // void calculateEffecVel() {
  //   const double diff = Pe - AIR_PRESSURE;
  //   effecVel = Vexit + diff * curr_Ae / output;
  // }
};