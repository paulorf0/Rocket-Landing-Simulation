#include "../include/rocket.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <cmath>
#include <sstream>
#include <string>

// output_* = kg / s
Rocket::Rocket(int rocket_width, int body_height, int nose_height)
    : rocket_width(rocket_width), body_height(body_height),
      nose_height(nose_height) {
  nose.setPointCount(3);
  body.setOrigin(0, 0);
  nose.setOrigin(0, 0);

  resetForce();

  ;

  rocket_prop.m = 0;
  rocket_prop.I_cm = 0;
  rocket_prop.r_cm = {0, 0};

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
  float v_mod = vector_mod(vel);
  if (v_mod < 0.001f)
    return;

  const auto mag_drag = 0.5f * AIR_DENSITY * mod_vel * mod_vel * area;

  const sf::Vector2f dragDir = -vel / v_mod;

  applyForce(dragDir * mag_drag);
}

void Rocket::configureSideBooster(const float gamma, const float minAe,
                                  const float minAt, const float maxAe,
                                  const float maxAt) {

  left.curr_output = 0; // The player can be control it.
  left.gamma = gamma;
  left.minAe = minAe;
  left.maxAe = maxAe;
  left.minAt = minAt;
  left.maxAt = maxAt;
  left.curr_Ae = (minAe + maxAe) / 2.;
  left.curr_At = (minAt + maxAt) / 2.;
  left.prev_Ae = 0.;

  right.curr_output = 0; // The player can be control it.
  right.gamma = gamma;
  right.minAe = minAe;
  right.maxAe = maxAe;
  right.minAt = minAt;
  right.maxAt = maxAt;
  right.curr_Ae = (minAe + maxAe) / 2.;
  right.curr_At = (minAt + maxAt) / 2.;
  right.prev_Ae = 0.;

  bottom.curr_output = 0; // The player can be control it.
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
  bottom.initBooster();
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

  setOrigin(cm);
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
  if (components.empty())
    return;

  const auto flow_rate_left = left.curr_output;
  const auto flow_rate_right = right.curr_output;
  const auto flow_rate_bottom = bottom.curr_output;

  const auto total = flow_rate_left + flow_rate_right + flow_rate_bottom;

  if (total == 0)
    return;

  const auto total_mass = total * dt;

  components.back().m -= total_mass;
  rocket_prop.m -= total_mass;

  if (components.back().m < 0.f)
    components.back().m = 0.f;

  updateCmAndInertia();
}

void Rocket::activeLeftBooster() {
  if (components.back().m <= 0)
    return;

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
  if (components.back().m <= 0)
    return;

  const auto f = right.getForce();
  const auto f_angle = angle + PI / 2.;

  const auto fx =
      f * std::cos(-f_angle); // The right-hand propeller pushes to the left.
  const auto fy = f * std::sin(f_angle);

  applyForce({static_cast<float>(fx), static_cast<float>(fy)});

  sf::Vector2f thrusterPosGlobal =
      getTransform().transformPoint(right_thruster.getPosition());

  applyTorque({static_cast<float>(fx), static_cast<float>(fy)},
              thrusterPosGlobal);
}

void Rocket::activeBottomBooster() {
  if (components.back().m <= 0)
    return;

  const auto f = bottom.getForce();

  const auto fx = f * std::sin(angle);
  const auto fy = -f * std::cos(angle);

  applyForce({static_cast<float>(fx), static_cast<float>(fy)});

  sf::Vector2f thrusterPosGlobal = getTransform().transformPoint(
      bottom_thruster.getPosition() + bottom_thruster.getSize() / 2.f);

  applyTorque({static_cast<float>(fx), static_cast<float>(fy)},
              thrusterPosGlobal);
}

void Rocket::applyTorque(sf::Vector2f force, sf::Vector2f global_dist) {
  sf::Vector2f global_cm = getTransform().transformPoint(rocket_prop.r_cm);

  float rx = global_dist.x - global_cm.x;
  float ry = global_dist.y - global_cm.y;

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
  if (!(std::isfinite(rocket_prop.m)) || rocket_prop.m <= 1e-6f)
    return;
  if (!std::isfinite(force.x) || !std::isfinite(force.y))
    return;

  sf::Vector2f a = force / rocket_prop.m;

  vel += a * dt;
  pos += vel * dt;

  if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(vel.x) ||
      !std::isfinite(vel.y)) {
    pos = {0.f, 0.f};
    vel = {0.f, 0.f};
  }

  setPosition(pos);
}

std::string get_string_vec(const sf::Vector2f &vec) {
  ostringstream os;
  os << "(" << vec.x << ", " << vec.y << "\n";
  return os.str();
}

std::string Rocket::getStatus() const {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2); // Fixa em 2 casas decimais

  ss << "===== ROCKET TELEMETRY =====\n";

  // --- Física Básica ---
  ss << "Position: (" << pos.x << ", " << pos.y << ")\n";
  ss << "Velocity: (" << vel.x << ", " << vel.y << ")\n";
  // Converte radianos para graus para facilitar leitura
  ss << "Angle:    " << angle * RADIANS_TO_DEGREES << " deg\n";
  ss << "Net Force:(" << force.x << ", " << force.y << ")\n";

  ss << "\n--- MASS & BALANCE ---\n";
  ss << "Total Mass: " << rocket_prop.m << " kg\n";

  // Assumindo que o tanque é o último componente (padrão do seu código)
  if (!components.empty()) {
    ss << "Fuel Mass:  " << components.back().m << " kg\n";
  } else {
    ss << "Fuel Mass:  N/A\n";
  }

  // Centro de Massa
  ss << "CM Pos:     (" << rocket_prop.r_cm.x << ", " << rocket_prop.r_cm.y
     << ")\n";

  ss << "\n--- ENGINE OUTPUT (kg/s) ---\n";
  // Mostra o valor Atual (Real) e o Alvo (Comando)
  ss << "Main (Bottom): " << bottom.curr_output
     << " (Target: " << bottom.target_output << ")\n";
  ss << "Left RCS:      " << left.curr_output
     << " (Target: " << left.target_output << ")\n";
  ss << "Right RCS:     " << right.curr_output
     << " (Target: " << right.target_output << ")\n";

  return ss.str();
}

void Rocket::update(float dt) {

  if (!std::isfinite(dt) || dt <= 0.f)
    return;
  if (dt > dt)
    dt = dt;

  const auto SUBSTEPS = 1;
  const float h = dt / static_cast<float>(SUBSTEPS);

  for (int i = 0; i < SUBSTEPS; ++i) {
    applyDragForce();
    applyForce(GRAVITY * rocket_prop.m);

    updatePosition(h);
    updateRotation(h);

    resetForce();
    resetTorque();
  }
}

void Rocket::setBoosterFuel(double T0, double M) {
  left.fuelProperties.T0 = T0;
  left.fuelProperties.M = M;
  left.fuelProperties.calculateR();

  right.fuelProperties.T0 = T0;
  right.fuelProperties.M = M;
  right.fuelProperties.calculateR();

  bottom.fuelProperties.T0 = T0;
  bottom.fuelProperties.M = M;
  bottom.fuelProperties.calculateR();
}

void Rocket::setBoosterOutputs(float leftOut, float rightOut, float bottomOut) {
  left.curr_output = leftOut;
  right.curr_output = rightOut;
  bottom.curr_output = bottomOut;
}

void Rocket::updateBoosters(float dt) {
  left.update(dt);
  right.update(dt);
  bottom.update(dt);
}

void Rocket::controlLeftOutput(float dOut) { left.controlOutput(dOut); }
void Rocket::controlRightOutput(float dOut) { right.controlOutput(dOut); }
void Rocket::controlBottomOutput(float dOut) { bottom.controlOutput(dOut); }

void Rocket::setInitialPosition(float x, float y) {
  pos = {x, y};
  setPosition(x, y);
}
