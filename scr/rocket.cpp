#include "../include/rocket.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>

// output_* = kg / s
Rocket::Rocket(int rocket_width, int body_height, int nose_height)
    : rocket_width(rocket_width), body_height(body_height),
      nose_height(nose_height) {
  nose.setPointCount(3);
  body.setOrigin(0, 0);
  nose.setOrigin(0, 0);

  resetForce();

  ;

  area = M_PI * rocket_width * rocket_width / 4.f;

  vel = {0, 0};
  pos = {0, 0};
  acc = {0, 0};
  pos_prev = {0, 0};
  force = {0, 0};
  angle = 0;
  torque = 0;
  angVel = 0;
}

void Rocket::setSideThrusters(const int &y, const int &width, const int &height,
                              const sf::Color &color) {
  left_thruster = sf::RectangleShape(
      {static_cast<float>(width), static_cast<float>(height)});
  right_thruster = sf::RectangleShape(
      {static_cast<float>(width), static_cast<float>(height)});

  left_thruster.setPosition({-width * 1.0f, static_cast<float>(y)});
  right_thruster.setPosition({1.f * rocket_width, static_cast<float>(y)});

  left_thruster.setFillColor(color);
  right_thruster.setFillColor(color);
}

void Rocket::setBottomThrusters(const int &x, const int &width,
                                const int &height, const sf::Color &color) {
  bottom_thruster = sf::RectangleShape(
      {static_cast<float>(width), static_cast<float>(height)});
  bottom_thruster.setPosition({1.f * x, body_height * 1.f});

  bottom_thruster.setFillColor(color);
}

void Rocket::setNose(const sf::Color &color) {
  sf::Vector2f left = {0.0, 0.0};
  sf::Vector2f right = {static_cast<float>(rocket_width), 0.0};
  sf::Vector2f sup = {static_cast<float>(rocket_width / 2.0),
                      static_cast<float>(-nose_height)};
  nose.setPoint(0, left);
  nose.setPoint(1, right);
  nose.setPoint(2, sup);

  nose.setFillColor(color);
}

void Rocket::setBody(const sf::Color &color) {
  body.setSize(
      {static_cast<float>(rocket_width), static_cast<float>(body_height)});
  body.setFillColor(color);
}

void Rocket::applyDragForce() {
  const auto mod_vel = vector_mod(vel);
  const auto mod_D = (1.f / 2.f) * (AIR_DENSITY * mod_vel * mod_vel * area);
  const auto f_angle = angle;

  const auto fx = mod_D * std::cos(f_angle);
  const auto fy = mod_D * std::sin(f_angle);

  applyForce({fx, fy});
}

void Rocket::configureSideBooster(const float gamma, const float minAe,
                                  const float minAt, const float maxAe,
                                  const float maxAt) {
  left.output = 0; // The player can be control it.
  left.gamma = gamma;
  left.minAe = minAe;
  left.maxAe = maxAe;
  left.minAt = minAt;
  left.maxAt = maxAt;
  left.curr_Ae = (minAe + maxAe) / 2.;
  left.curr_At = (minAt + maxAt) / 2.;
  left.prev_Ae = 0.;

  right.output = 0; // The player can be control it.
  right.gamma = gamma;
  right.minAe = minAe;
  right.maxAe = maxAe;
  right.minAt = minAt;
  right.maxAt = maxAt;
  right.curr_Ae = (minAe + maxAe) / 2.;
  right.curr_At = (minAt + maxAt) / 2.;
  right.prev_Ae = 0.;

  bottom.output = 0; // The player can be control it.
  bottom.gamma = gamma;
  bottom.minAe = minAe;
  bottom.maxAe = maxAe;
  bottom.minAt = minAt;
  bottom.maxAt = maxAt;
  bottom.curr_Ae = (minAe + maxAe) / 2.;
  bottom.curr_At = (minAt + maxAt) / 2.;
  bottom.prev_Ae = 0.;

  left.initBooster();
  right.initBooster();
}

void Rocket::calculateInertia() {
  if (components.empty() || rocket_prop.m <= 0.f) {
    rocket_prop.r_cm = {0.f, 0.f};
    return;
  }

  float inertia = 0;
  auto cm = rocket_prop.r_cm;
  for (const auto &[mass, pos, inertia_local] : components) {
    inertia += inertia_local + mass * vector_len_sqr(pos - cm);
  }

  rocket_prop.I_cm = inertia;
}

void Rocket::calculateCm() {
  if (components.empty() || rocket_prop.m <= 0.f) {
    rocket_prop.r_cm = {0.f, 0.f};
    return;
  }

  sf::Vector2f cm = {0.0, 0.0};

  for (const auto &[mass, pos, i_] : components) {
    cm += mass * pos;
  }

  cm /= rocket_prop.m;
  rocket_prop.r_cm = cm;
}

void Rocket::addComponent(struct MassComponent comp) {
  components.push_back(comp);
  rocket_prop.m += comp.m;

  updateCmAndInertia();
}

void Rocket::updateCmAndInertia() {
  calculateCm();
  calculateInertia();
}

void Rocket::consumeFuelMass(float dt) {
  const auto flow_rate_left = left.output;
  const auto flow_rate_right = right.output;
  const auto flow_rate_bottom = bottom.output;

  const auto total = flow_rate_left + flow_rate_right + flow_rate_bottom;

  if (total == 0)
    return;

  const auto total_mass = total * dt;

  components.back().m -= total_mass;
  updateCmAndInertia();
}

void Rocket::activeLeftBooster() {
  const auto f = left.getForce();
  const auto f_angle = angle + PI / 2.;

  const auto fx =
      f * std::cos(f_angle); // The left-hand propeller pushes to the right.
  const auto fy = f * std::sin(f_angle);

  applyForce({static_cast<float>(fx), static_cast<float>(fy)});

  sf::Vector2f thrusterPosGlobal =
      getTransform().transformPoint(left_thruster.getPosition());

  applyTorque({static_cast<float>(fx), static_cast<float>(fy)},
              thrusterPosGlobal);
}

void Rocket::activeRightBooster() {
  const auto f = right.getForce();
  const auto f_angle = angle + PI / 2.;

  const auto fx =
      -f * std::cos(f_angle); // The right-hand propeller pushes to the left.
  const auto fy = f * std::sin(f_angle);

  applyForce({static_cast<float>(fx), static_cast<float>(fy)});

  sf::Vector2f thrusterPosGlobal =
      getTransform().transformPoint(left_thruster.getPosition());

  applyTorque({static_cast<float>(fx), static_cast<float>(fy)},
              thrusterPosGlobal);
}

void Rocket::activeBottomBooster() {
  const auto f = bottom.getForce();
  const auto f_angle = angle;

  const auto fx =
      f * std::cos(f_angle); // The right-hand propeller pushes to the left.
  const auto fy = -f * std::sin(f_angle);

  applyForce({static_cast<float>(fx), static_cast<float>(fy)});

  sf::Vector2f thrusterPosGlobal =
      getTransform().transformPoint(left_thruster.getPosition());

  applyTorque({static_cast<float>(fx), static_cast<float>(fy)},
              thrusterPosGlobal);
}

void Rocket::applyTorque(sf::Vector2f force, sf::Vector2f global_dist) {
  float rx = global_dist.x - rocket_prop.r_cm.x; // r_cm precisa ser Global
  float ry = global_dist.y - rocket_prop.r_cm.y;
  float t = rx * force.y - ry * force.x;

  this->torque += t;
}

void Rocket::updateRotation(float dt) {
  float alpha = (rocket_prop.I_cm > 1e-6f) ? (torque / rocket_prop.I_cm) : 0.f;

  angVel += alpha * dt;
  angle += angVel * dt;

  setRotation(angle * RADIANS_TO_DEGREES);
}

void Rocket::updatePosition(float dt) {
  sf::Vector2f a = force / rocket_prop.m;

  vel += a * dt;
  pos += vel * dt;

  setPosition(pos);
}

void Rocket::update(float dt) {
  const sf::Vector2f gravity = {0., 9.81};
  applyDragForce();
  applyForce(gravity * rocket_prop.m);

  updatePosition(dt);
  updateRotation(dt);

  resetForce();
  resetTorque();
}
