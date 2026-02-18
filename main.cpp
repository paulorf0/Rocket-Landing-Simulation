#include "include/rocket.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

inline float dot(const sf::Vector2f &a, const sf::Vector2f &b) {

  return a.x * b.x + a.y * b.y;
}
inline float cross(const sf::Vector2f &a, const sf::Vector2f &b) {
  return a.x * b.y - a.y * b.x;
}

inline sf::Vector2f cross(float w, const sf::Vector2f &r) {
  return sf::Vector2f(-w * r.y, w * r.x);
}

void resolveGroundContact(Rocket &rocket, const sf::RectangleShape &platform) {
  if (!rocket.getBounds().intersects(platform.getGlobalBounds()))
    return;

  const float mass = rocket.getMass();
  const float I = rocket.getInertia();
  if (mass <= 1e-6f || I <= 1e-6f)
    return;

  const sf::Vector2f n(0.f, -1.f);
  const sf::Vector2f t(1.f, 0.f);

  const sf::Vector2f v = rocket.getVel();
  float w = rocket.getAngularVel();
  const sf::Vector2f cm = rocket.getCmWorld();
  const sf::FloatRect rb = rocket.getBounds();
  const sf::FloatRect pb = platform.getGlobalBounds();

  const sf::Vector2f contact(rb.left + rb.width * 0.5f, rb.top + rb.height);
  const sf::Vector2f r = contact - cm;
  const sf::Vector2f v_contact = v + cross(w, r);

  const float vn = dot(v_contact, n);
  const float vt = dot(v_contact, t);

  if (vn >= 0.f)
    return;

  float e = 0.15f;
  if (std::abs(vn) < 1.0f)
    e = 0.f;

  const float rCrossN = cross(r, n);
  const float denomN = (1.f / mass) + (rCrossN * rCrossN) / I;
  float jn = -(1.f + e) * vn / denomN;

  const float frictionCoeff = 0.5f;
  const float rCrossT = cross(r, t);
  const float denomT = (1.f / mass) + (rCrossT * rCrossT) / I;

  float jt = -vt / denomT;
  float maxJt = jn * frictionCoeff;
  jt = std::max(-maxJt, std::min(maxJt, jt));

  const sf::Vector2f impulse = (jn * n) + (jt * t);

  rocket.applyVel(impulse / mass);
  rocket.applyAngVel(cross(r, impulse) / I);

  const float angularThreshold = 0.1f;
  if (std::abs(rocket.getAngularVel()) < angularThreshold) {
    rocket.setAngVel(0.f);
  } else {
    rocket.applyAngVel(-rocket.getAngularVel() * 0.1f);
  }

  const float penetration = (rb.top + rb.height) - pb.top;
  if (penetration > 0.f) {
    const float slop = 0.1f;
    const float beta = 0.3f;
    rocket.applyPos(n * (std::max(penetration - slop, 0.f) * beta));
  }
}
auto createRocket(float screenW, float screenH) {

  Rocket rocket(/*rocket_width*/ 40, /*body_height*/ 140, /*nose_height*/ 60);

  rocket.setBody(sf::Color(220, 220, 220));
  rocket.setNose(sf::Color(200, 80, 80));
  rocket.setSideThrusters(/*y*/ 50, /*w*/ 10, /*h*/ 25,
                          sf::Color(240, 200, 60));
  rocket.setBottomThrusters(/*x*/ 15, /*w*/ 10, /*h*/ 25,
                            sf::Color(240, 200, 60));

  rocket.configureSideBooster(
      /*gamma*/ 1.22f,
      /*minSideAe*/ 0.00001f,
      /*minSideAt*/ 0.0002f,
      /*maxSideAe*/ 0.00008f,
      /*maxSideAt*/ 0.0008f,
      /*minBottomAe*/ 0.00001f,
      /*minBottomAt*/ 0.0002f,
      /*maxBottomAe*/ 0.0005f,
      /*maxBottomAt*/ 0.0004f);

  rocket.setBoosterFuel(/*T0*/ 3200.0, /*M*/ 22.0);

  rocket.setBoosterOutputs(0.f, 0.f, 0.f);

  rocket.addComponent({/*m*/ 100.f, /*r*/ {20.f, 70.f}, /*I_local*/ 0.f});
  rocket.addComponent({/*m*/ 100.f, /*r*/ {20.f, -20.f}, /*I_local*/ 0.f});
  rocket.addComponent({/*m*/ 80.f, /*r*/ {20.f, 110.f}, /*I_local*/ 0.f});
  rocket.setInitialPosition(screenW * 0.5f, screenH * 0.6f);
  return rocket;
}

int main() {
  const float width = 1000;
  const float height = 1000;
  sf::RenderWindow window(sf::VideoMode({static_cast<unsigned int>(width),
                                         static_cast<unsigned int>(height)}),
                          "Rocket Simulator");
  window.setFramerateLimit(120);
  auto rocket = createRocket(width, height);

  sf::RectangleShape platform({300.f, 20.f});
  platform.setFillColor(sf::Color(100, 100, 100));
  platform.setPosition({300.f, 900.f});

  sf::Clock clock;

  sf::Font font;

  if (!font.loadFromFile("/usr/share/fonts/truetype/jetbrains-mono-zorin-os/"
                         "JetBrainsMono-Regular.ttf")) {
  }

  sf::Text hudText;
  hudText.setFont(font);
  hudText.setCharacterSize(14);
  hudText.setFillColor(sf::Color::White);
  hudText.setPosition(10, 10);

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    float dt = clock.restart().asSeconds();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
      rocket.activeBottomBooster();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
      rocket.activeLeftBooster();

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
      rocket.activeRightBooster();

    const float dOut = 10.0f * dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
      rocket.controlBottomOutput(+dOut);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
      rocket.controlBottomOutput(-dOut);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))
      rocket.controlLeftOutput(+dOut);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::J))
      rocket.controlLeftOutput(-dOut);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
      rocket.controlRightOutput(+dOut);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
      rocket.controlRightOutput(-dOut);

    rocket.updateBoosters(dt);
    rocket.consumeFuelMass(dt);

    rocket.update(dt);

    if (rocket.getBounds().intersects(platform.getGlobalBounds())) {
      const auto vel = rocket.getLenVel();
      if (vel > 80.)
        std::cout << "Explodiu\n";

      resolveGroundContact(rocket, platform);
    }

    window.clear();
    window.draw(rocket);
    window.draw(platform);

    hudText.setString(rocket.getStatus());
    window.draw(hudText);
    window.display();
  }

  return 0;
}
