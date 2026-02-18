#include "include/rocket.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

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
      /*minAe*/ 0.00001f,
      /*minAt*/ 0.0002f,
      /*maxAe*/ 0.0005f,
      /*maxAt*/ 0.0006f);

  rocket.setBoosterFuel(/*T0*/ 3200.0, /*M*/ 22.0);

  rocket.setBoosterOutputs(0.f, 0.f, 0.f);

  rocket.addComponent({/*m*/ 100.f, /*r*/ {20.f, 70.f}, /*I_local*/ 0.f});
  rocket.addComponent({/*m*/ 20.f, /*r*/ {20.f, -20.f}, /*I_local*/ 0.f});
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

    window.clear();
    window.draw(rocket);

    hudText.setString(rocket.getStatus());
    window.draw(hudText);

    window.display();
  }

  return 0;
}