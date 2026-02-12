#include "../include/rocket.hpp"

void Rocket::updateCenterMass() {
  const auto m_c = fuelMassMax * fuelMassCurr;
  const auto m_total = componentMass + m_c;

  const auto y = (componentMass * yComponents + m_c * yTank) / m_total;

  setOrigin(x, y);
}

void Rocket::consume_fuel(float dt, float throttle) {
  if (fuelMassCurr <= 0)
    return;

  const float consume_rate = 2.0f;
  const auto delta = consume_rate * throttle * dt;
  const auto porcent = delta / fuelMassMax;

  fuelMassCurr -= porcent;

  if (fuelMassCurr < 0)
    fuelMassCurr = 0;
}
