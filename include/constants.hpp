#pragma once

#include <SFML/System/Vector2.hpp>

// 60 pixel / 1 meter.
const float PPM = 60.f;

// const auto AIR_DENSITY = 1.f;
const auto AIR_DENSITY = 0.0005f;
// const auto AIR_PRESSURE = 1.f;
const auto AIR_PRESSURE = 101325.f;
// const sf::Vector2f GRAVITY = {0.f, 98 * 7.f};
const sf::Vector2f GRAVITY = {0.f, 9.8f * PPM * 1.f /*9.8f * PPM*/};

const auto DRAG_COEFFICIENT = 1.f;
