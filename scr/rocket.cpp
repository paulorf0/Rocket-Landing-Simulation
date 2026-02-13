#include "../include/rocket.hpp"
#include <cmath>
#include <sstream>

const float DEG_TO_RAD = PI / 180.f;
const float RAD_TO_DEG = 180.f / PI;

// fuelMassCurr = 1, default.
Rocket::Rocket(double componentMass, double fuelMassMax, double fuelMassCurr,
               int rocket_height, int rocket_width, double yComponents,
               double yTank)
    : componentMass(componentMass), fuelMassMax(fuelMassMax),
      fuelMassCurr(fuelMassCurr), yComponents(yComponents), yTank(yTank) {
  v = {0, 0};
  force = {0, 0};
  torque = 0;
  angularVelocity = 0.f;

  const float triangle_height = rocket_height * 0.3f;
  const float body_height = rocket_height * 0.7f;

  body.setSize({rocket_width / 1.f, body_height});
  body.setFillColor(sf::Color::White);
  body.setPosition(0.f, 0.f);

  nose.setPointCount(3);
  nose.setFillColor(sf::Color::White);

  nose.setPoint(0, {0.f, 0.f});
  nose.setPoint(1, {rocket_width / 2.f, -triangle_height});
  nose.setPoint(2, {rocket_width / 1.f, 0.f});

  const float thruster_width = rocket_width * 0.4f;

  bottom_thruster.setSize({thruster_width, thruster_width * 0.5f});
  bottom_thruster.setPosition(
      {rocket_width / 2.f - thruster_width / 2.f, body_height});
  bottom_thruster.setFillColor(sf::Color::Red);

  y_thruster = 10.f;

  left_thruster.setSize({thruster_width * 0.5f, thruster_width});
  left_thruster.setPosition({0 - thruster_width / 2, y_thruster});
  left_thruster.setFillColor(sf::Color::Red);

  right_thruster.setSize({thruster_width * 0.5f, thruster_width});
  right_thruster.setPosition({rocket_width / 1.f, y_thruster});
  right_thruster.setFillColor(sf::Color::Red);

  updateCenterMass(); // Set origin of rocket.
}

std::string Rocket::getDebugString() const {
  const float currentRotation = getRotation();
  const auto angleRad = currentRotation * PI / 180.f;
  const auto thrustX = std::sin(angleRad) * thruster_bottom_force;
  const auto thrustY = -std::cos(angleRad) * thruster_bottom_force;
  const auto total_mass = fuelMassCurr * fuelMassMax + componentMass;
  const auto FG = gravity * total_mass;
  const auto inertia = calculateInertia();
  const auto v_mod = sqrt(v.x * v.x + v.y * v.y);

  std::stringstream ss;
  ss << "Rotation (deg): " << currentRotation << "\n"
     << "Angle (rad): " << angleRad << "\n"
     << "Thrust X: " << thrustX << " | Thrust Y: " << thrustY << "\n"
     << "Total Mass: " << total_mass << "\n"
     << "Inertia: " << inertia << "\n"
     << "Gravity Force: " << FG << "\n"
     << "Current Velocity: (" << v.x << ", " << v.y << ")\n"
     << "Mod Current Velocity: " << v_mod << "\n"
     << "dt: " << dt;
  return ss.str();
}

void Rocket::consume_fuel(float throttle) {
  if (fuelMassCurr <= 0)
    return;

  const float consume_rate = 30.0f;
  const auto delta = consume_rate * throttle * dt;
  const auto porcent = delta / fuelMassMax;

  fuelMassCurr -= porcent;

  if (fuelMassCurr < 0)
    fuelMassCurr = 0;

  updateCenterMass();
}

float Rocket::calculateInertia() const {
  const auto t1 = std::abs(yComponents - y);
  const auto t2 = std::abs(yTank - y);

  double inertia =
      (componentMass * t1 * t1) + (fuelMassCurr * fuelMassMax * t2 * t2);

  return (inertia < 100.0f) ? 100.0f : (float)inertia * 0.05;
}

void Rocket::applyTorque(const double tau) { torque += tau; }

void Rocket::updateRocket() {
  // Apply Gravity
  //
  const sf::Vector2f gravity_vec = {0.f, gravity};
  applyForce(gravity_vec * totalMass());

  // Update Velocity and angular velocity
  v += dt * (force / totalMass());
  angularVelocity += dt * (torque / calculateInertia());

  move(v * dt);
  rotate(angularVelocity * dt * RAD_TO_DEG);

  angularVelocity *= angularDamping;

  // Reset torque and forces
  resetTorque();
  resetForce();

  check_collision(1000, 1000);
}

void Rocket::active_thruster_bottom() {
  if (fuelMassCurr <= 0)
    return;

  consume_fuel(0.1);

  const auto theta = (90.f - getRotation()) * DEG_TO_RAD;
  const auto Fx = thruster_bottom_force * std::cos(theta);
  const auto Fy = -thruster_bottom_force * std::sin(theta);

  const sf::Vector2f force = {Fx, Fy};
  applyForce(force);
}

void Rocket::active_thruster_side(bool side) // true = left, false = right.
{
  if (fuelMassCurr <= 0)
    return;
  consume_fuel(0.01);

  const auto force = side == true ? thruster_side_force : -thruster_side_force;
  const auto r = y - y_thruster;

  const auto tau = force * r;

  applyTorque(tau);
}

void Rocket::updateCenterMass() {
  const auto t1 = fuelMassCurr * fuelMassMax * yTank;
  const auto t2 = componentMass * yComponents;

  y = (t1 + t2) / totalMass();
}

void Rocket::check_collision(int width, int height) {
  sf::Vector2f bottomPointLocal(body.getSize().x / 2.f, body.getSize().y);

  sf::Vector2f bottomPointGlobal =
      getTransform().transformPoint(bottomPointLocal);

  if (bottomPointGlobal.y >= height) {
    const auto col_vel = sqrt(v.x * v.x + v.y * v.y);

    if (col_vel > 20)
      std::cout << "Explodiu" << std::endl;

    v.x = 0;
    v.y = 0;
    move(0, height - bottomPointGlobal.y);
  }
}
