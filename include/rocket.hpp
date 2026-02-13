#pragma once

#include "constants.h"

#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <string>

class Rocket : public sf::Drawable, public sf::Transformable {
public:
  Rocket(double componentMass, double fuelMassMax, double fuelMassCurr,
         int rocket_height, int rocket_width, double yComponents, double yTank);

  void updateRocket();
  void debug_thruster_bottom();

  void active_thruster_bottom();
  void
  active_thruster_side(bool side); // side = true (left), side = false (left).

  void check_collision(int width, int height);

  void consume_fuel(float throttle);
  float calculateInertia() const;

  void applyForce(const sf::Vector2f &force_) { force += force_; };
  void resetForce() { force = {0, 0}; };

  void applyTorque(const double torque);
  void resetTorque() { torque = 0; }

  float getFuelMass() const { return fuelMassCurr; };
  sf::Vector2f getCenterMass() const { return {x, y}; };
  sf::Vector2f getVelocity() const { return v; };
  std::string getDebugString() const;

  void setThrusterBottomForce(float force) { thruster_bottom_force = force; };
  void setThrusterSideForce(float force) { thruster_side_force = force; };

private:
  void updateCenterMass();
  inline float totalMass() const {
    return fuelMassCurr * fuelMassMax + componentMass;
  };

  float thruster_bottom_force = 0.f, thruster_side_force = 0.f;
  float y_thruster;

  sf::Vector2f v;
  sf::Vector2f force;
  double torque;
  double angularVelocity;
  double theta; // Orientation.

  double componentMass, fuelMassMax, fuelMassCurr; // 0.0 <= fuelMassCurr <= 1.0
  double yComponents, yTank; // yComponents = Location of Components.

  double y;              // center of mass y
  const double x = 20.f; // center of mass x

  const double delta_consume = 0.02f;

  sf::RectangleShape body;
  sf::RectangleShape bottom_thruster;
  sf::RectangleShape left_thruster;
  sf::RectangleShape right_thruster;
  sf::ConvexShape nose; // Tringle Shape.

  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
    states.transform *= getTransform();

    target.draw(body, states);
    target.draw(nose, states);
    target.draw(bottom_thruster, states);
    target.draw(left_thruster, states);
    target.draw(right_thruster, states);
  }
};
