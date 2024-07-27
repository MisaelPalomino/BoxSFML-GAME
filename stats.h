#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>

class Observer {
public:
    virtual ~Observer() {}
    virtual void update(float health, float shield, float speed, float damage) = 0;
};

class StatsDisplay : public Observer {
public:
    StatsDisplay(sf::Vector2f position, bool topRight = false);
    void update(float health, float shield, float speed, float damage) override;
    void draw(sf::RenderWindow& window);
    void updatePosition(const sf::Vector2f& playerPosition, const sf::View& view);

private:
    sf::Font font;
    sf::Text text;
    sf::Vector2f position;
    float health;
    float shield;
    float speed;
    float damage;
    bool topRight;  //variable para determinar la esquina
};

StatsDisplay::StatsDisplay(sf::Vector2f position, bool topRight) : position(position), health(100), shield(5), speed(2.0f), damage(10), topRight(topRight) {
    if (!font.loadFromFile("resources/fonts/arial.ttf")) {
    }
    text.setFont(font);
    text.setCharacterSize(10); 
    text.setFillColor(sf::Color::White);
    text.setPosition(position);
}

void StatsDisplay::update(float health, float shield, float speed, float damage) {
    this->health = health;
    this->shield = shield;
    this->speed = speed;
    this->damage = damage;
    text.setString("HP: " + std::to_string(health) + "\nShield: " + std::to_string(shield) + "\nSpeed: " + std::to_string(speed) + "\nDamage: " + std::to_string(damage));
}

void StatsDisplay::draw(sf::RenderWindow& window) {
    window.draw(text);
}

void StatsDisplay::updatePosition(const sf::Vector2f& playerPosition, const sf::View& view) {
    sf::Vector2f offset;
    if (topRight) {
        offset = sf::Vector2f(view.getSize().x - 110.0f, 10.0f);  // Esquina superior derecha
    } else {
        offset = sf::Vector2f(10.0f, 10.0f);  // Esquina superior izquierda
    }
    sf::Vector2f statsPosition = playerPosition - view.getSize() / 2.0f + offset;
    text.setPosition(statsPosition);
}
