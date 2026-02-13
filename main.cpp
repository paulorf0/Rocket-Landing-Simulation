#include "include/rocket.hpp"
#include <SFML/Graphics.hpp>

int main() {

  sf::Font font;
  if (!font.loadFromFile("JetBrainsMonoNL-Regular.ttf")) {
    std::cerr << "Erro ao carregar a fonte" << std::endl;
    return -1;
  }

  const int width = 1000;
  const int height = 1000;

  sf::RenderWindow window(sf::VideoMode({width, height}), "SFML works!");

  Rocket rocket(40.f, 100.f, 1.f, 100.f, 30.f, 100.f, 30.f);

  window.setFramerateLimit(60.f);
  sf::Clock clock;

  rocket.setPosition(width / 2.0f, height / 2.0f);
  rocket.setThrusterSideForce(70.f);
  rocket.setThrusterBottomForce(4000.f);

  sf::Text debugText;
  debugText.setFont(font);
  debugText.setCharacterSize(16);
  debugText.setFillColor(sf::Color::Green);
  debugText.setPosition(10.f, 10.f);

  float debugTimer = 0.f;
  const float debugUpdateInterval = 0.2f;

  while (window.isOpen()) {
    sf::Time elapsed = clock.restart();
    float dt = elapsed.asSeconds();
    debugTimer += dt;
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        rocket.active_thruster_side(true);
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        rocket.active_thruster_side(false);
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
        rocket.active_thruster_bottom();
      }
    }

    rocket.updateRocket();

    if (debugTimer >= debugUpdateInterval) {
      debugText.setString(rocket.getDebugString());
      debugTimer = 0.f;
    }

    window.clear();
    window.draw(rocket);
    window.draw(debugText);
    window.display();
  }
}
