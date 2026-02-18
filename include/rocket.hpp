#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <iostream>
#include <vector>

#include "constants.hpp"
#include "numeric_solver.hpp"
#include "rocket_booster.hpp"

#define PI 3.1415926535
#define DEGREES_TO_RADIANS (PI / 180)
#define RADIANS_TO_DEGREES (180 / PI)

struct MassComponent {
  float m;        // Component Mass
  sf::Vector2f r; // Component Pos
  float I_local;  // Component Inertia
};

struct MassProps {
  float m;           // Total Mass
  sf::Vector2f r_cm; // Center of Mass Position
  float I_cm;        // Total inertia (rocket)
};

class Rocket : public sf::Transformable, public sf::Drawable {

public:
  Rocket(int rocket_width, int body_height, int nose_height);

  std::string getStatus() const;

  void setInitialPosition(float x, float y);

  inline void applyForce(sf::Vector2f force) { this->force += force; }
  inline void resetForce() { force = {0.f, 0.f}; }

  void applyTorque(sf::Vector2f force, sf::Vector2f global_dist);
  inline void resetTorque() { torque = 0.f; }

  void update(float dt);
  void updatePosition(float dt);
  void updateRotation(float dt);

  void applyDragForce();

  void activeLeftBooster();
  void activeRightBooster();
  void activeBottomBooster();

  void addComponent(struct MassComponent comp);
  void updateCmAndInertia();
  void calculateCm();
  void calculateInertia();

  void consumeFuelMass(float dt);

  void configureSideBooster(const float gamma, const float minAe,
                            const float minAt, const float maxAe,
                            const float maxAt);
  void setNose(const sf::Color &color);
  void setBody(const sf::Color &color);
  void setSideThrusters(const int &y, const int &width, const int &height,
                        const sf::Color &color);
  void setBottomThrusters(const int &x, const int &width, const int &height,
                          const sf::Color &color);

  void updateBoosters(float dt);

  void controlLeftOutput(float dOut);
  void controlRightOutput(float dOut);
  void controlBottomOutput(float dOut);

  void controlLeftNozzleArea(float dA);
  void controlRightNozzleArea(float dA);
  void controlBottomNozzleArea(float dA);

  void controlLeftThroatArea(float dA);
  void controlRightThroatArea(float dA);
  void controlBottomThroatArea(float dA);
  void setBoosterFuel(double T0, double M);
  void setBoosterOutputs(float leftOut, float rightOut, float bottomOut);

  const MassProps &massProps() const { return rocket_prop; } // debug

private:
  struct Newton_Raphson solver;
  struct RocketBooster left;
  struct RocketBooster right;
  struct RocketBooster bottom;

  float area; // The bigger area at rocket

  sf::Vector2f vel;
  sf::Vector2f pos;
  sf::Vector2f acc;
  sf::Vector2f pos_prev;
  sf::Vector2f force;
  // TODO: All functions that use the angle need be fixed. Because the angle =
  // 0 make the rocket point to up.
  float angle; // angle = 0 radians (x direction, to right)
  float torque;
  float angVel;

  std::vector<struct MassComponent>
      components; // The tank need be the last component.
  struct MassProps rocket_prop;

  // Rocket Design
  const int rocket_width, body_height, nose_height;
  sf::RectangleShape body;
  sf::ConvexShape nose;
  sf::RectangleShape left_thruster;
  sf::RectangleShape right_thruster;
  sf::RectangleShape bottom_thruster;

  void draw(sf::RenderTarget &target, sf::RenderStates states) const override {
    states.transform *= getTransform();

    target.draw(body, states);
    target.draw(nose, states);
    target.draw(left_thruster, states);
    target.draw(right_thruster, states);
    target.draw(bottom_thruster, states);
  }

  inline float vector_mod(const sf::Vector2f vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
  }

  inline float vector_len_sqr(const sf::Vector2f vec) {
    return vec.x * vec.x + vec.y * vec.y;
  }
};