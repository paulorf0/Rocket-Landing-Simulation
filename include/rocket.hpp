#pragma once

#include <SFML/Graphics.hpp>

class Rocket : public sf::Drawable, public sf::Transformable {
public:
  Rocket();

  void consume_fuel(float dt, float throttle);

private:
  void updateCenterMass();

  sf::Vector2f v;
  double theta; // Orientation.

  double componentMass, fuelMassMax, fuelMassCurr; // 0.0 <= fuelMassCurr <= 1.0
  double yComponents, yTank; // yComponents = Location of Components.
  double fuel;
  const double x = 20.f; // center of mass x
  const double delta_consume = 0.02f;

  sf::RectangleShape body;
  sf::ConvexShape nose; // Tringle Shape.

  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
    states.transform *= getTransform();

    target.draw(body, states);
    target.draw(nose, states);
  }
};
