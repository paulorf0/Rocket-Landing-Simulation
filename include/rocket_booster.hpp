#pragma once

#include "FuelProperties.hpp"
#include "constants.hpp"
#include "numeric_solver.hpp"

#include <cmath>

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
  float delay = 0.2;

  // Defined By Player.
  float output;
  float gamma;

  float minAe, minAt;
  float maxAe, maxAt;

  float target_Ae, target_At;
  float curr_Ae;
  float curr_At;
  float prev_Ae;

  // Defined By Equations
  float Mach;
  float Pe;       // Exit Pressure
  float effecVel; // Effective Velocity
  float Vexit;    // Exit Velocity
  float Te;       // nozzle static temperature

  struct Newton_Raphson solver;
  struct FuelProperties fuelProperties;

  void initBooster() {
    fuelProperties.calculateR();

    target_At = curr_At;
    target_Ae = curr_Ae;
  }

  float getForce() {
    updateVariables();

    return output * effecVel + (Pe - AIR_PRESSURE) * curr_Ae;
  }

  void update(float dt) {
    updateThroatArea(dt);
    updateNozzleArea(dt);
  }

  void updateThroatArea(float dt) {
    if (std::abs(curr_At - target_At) < 0.0001f) {
      curr_At = target_At;
      return;
    }

    curr_At += (target_At - curr_At) * delay * dt;
  }

  void updateNozzleArea(float dt) {
    if (std::abs(curr_Ae - target_Ae) < 0.0001f) {
      curr_Ae = target_Ae;
      return;
    }

    curr_Ae += (target_At - curr_Ae) * delay * dt;
  }

  void controlOutput(float dOut) { output += dOut; }

  void controlNozzleArea(float dA) {
    target_Ae += dA;

    if (target_Ae < minAe)
      target_Ae = minAe;
    if (target_Ae > maxAe)
      target_Ae = maxAe;
  }

  void controlThroatArea(float dA) {
    target_At += dA;

    if (target_At < minAt)
      target_At = minAt;
    if (target_At > maxAt)
      target_At = maxAt;
  }

  void updateVariables() {
    calculateMach();
    calculateExitPressure();
    calculateTe();
    calculateVexit();
    calculateEffecVel();
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
      return (t2 * std::pow(M, expoent + 2.f) + std::pow(M, expoent) -
              std::pow(epsilon, expoent) / t1);
    };

    const auto df = [&](double M) -> double {
      return t2 * (expoent + 1) * std::pow(M, expoent + 1.f) +
             expoent * std::pow(M, expoent - 1);
    };

    solver.setPrecision(0.001);
    solver.setFunc(f, df);

    const double x0 = 0.;
    Mach = solver.solve(x0);
  }

  void calculateExitPressure() {
    // TODO: How to optimize it.
    //  if (curr_Ae == prev_Ae)
    //    return;

    const auto t1 = (gamma - 1) / 2.;
    const auto expoent = -gamma / (gamma - 1);

    // Approximate pc = p0 (atmosphere)

    Pe = AIR_PRESSURE * std::pow(1. + t1 * Mach * Mach, expoent);
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

  void calculateEffecVel() {
    const double diff = Pe - AIR_PRESSURE;
    effecVel = Vexit + diff * curr_Ae / output;
  }
};