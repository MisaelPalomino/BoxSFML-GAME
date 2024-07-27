#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <memory>
#include <thread>
#include <future>
#include "map.h"
#include "player.h"
#include "stats.h"
#include "enemy.h"

enum GameState { MENU, PLAYING, PAUSED, GAME_OVER };

int main() {

    sf::RenderWindow window(sf::VideoMode(800, 600), "Roguelike");
    GameState state = MENU;

    // Menú
    sf::Font font;
    if (!font.loadFromFile("resources/Fonts/PixelifySans-VariableFont_wght.ttf")) {
        return -1;
    }

    // Texto del menú
    sf::Text title("Roguelike Game", font, 50);
    title.setPosition(200, 150);

    sf::Text playOption("Press Enter to Play", font, 30);
    playOption.setPosition(250, 300);

    sf::Text pauseOption("Press P to Pause", font, 30);
    pauseOption.setPosition(250, 350);

    sf::Text gameOverText("Game Over! Press R to Restart", font, 30);
    gameOverText.setPosition(150, 300);

    // Inicialización del juego
    const unsigned int width = 20;
    const unsigned int height = 50;

    std::vector<int> level = generateRandomMap(width, height);

    TileMap map;
    if (!map.load("resources/Tiles/grass.png", sf::Vector2u(16, 16), level.data(), width, height)) {
        return -1;
    }

    std::vector<std::unique_ptr<Enemy>> enemies;
    enemies.push_back(FactoryEnemy::createEnemy("orco", "resources/Enemies/orc.png", sf::Vector2f(0, 0)));
    enemies.push_back(FactoryEnemy::createEnemy("slime", "resources/Enemies/SlimeMove.png", sf::Vector2f(0, 0)));
    enemies.push_back(FactoryEnemy::createEnemy("slime", "resources/Enemies/slimemen.png", sf::Vector2f(100, 100), true));

    Player player("resources/Characters/idle.png", sf::Vector2u(80, 80), 4, level.data(), width, height);
    Player player2("resources/Characters/idle.png", sf::Vector2u(80, 80), 4, level.data(), width, height);

    player.setHandleInput(sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D);
    player2.setHandleInput(sf::Keyboard::Up, sf::Keyboard::Down, sf::Keyboard::Left, sf::Keyboard::Right);

    sf::View view(sf::FloatRect(0, 0, 400, 300));
    view.setCenter(player.getPosition());

    sf::View defaultView = window.getDefaultView();
    auto stats1 = std::make_shared<StatsDisplay>(sf::Vector2f(10, 10));
    auto stats2 = std::make_shared<StatsDisplay>(sf::Vector2f(10, 10), true);
    player.addObserver(stats1);
    player2.addObserver(stats2);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (state == MENU) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                    state = PLAYING;
                }
            } else if (state == PLAYING) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                    state = PAUSED;
                }
            } else if (state == PAUSED) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                    state = PLAYING;
                }
            } else if (state == GAME_OVER) {
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                    player.reset();
                    player2.reset();
                    state = MENU;
                }
            }
        }

        window.clear();

        if (state == MENU) {
            window.setView(defaultView);
            window.draw(title);
            window.draw(playOption);
        } else if (state == PLAYING) {
            view.setCenter(player.getPosition());
            window.setView(view);
            player.handleInput();
            player2.handleInput();
            player.update();
            player2.update();
            stats1->updatePosition(player.getPosition(), view);
            stats2->updatePosition(player.getPosition(), view);
            window.clear();
            window.draw(map);
            player.draw(window);
            player2.draw(window);

            // Cálculo de direcciones concurrentemente
            std::vector<std::future<sf::Vector2f>> futures;
            for (auto& enemy : enemies) {
                futures.push_back(std::async(std::launch::async, [&player, &player2, &enemy]() {
                    sf::Vector2f direction = player.getPosition() - enemy->getPosition();
                    sf::Vector2f direction2 = player2.getPosition() - enemy->getPosition();

                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    float length2 = std::sqrt(direction2.x * direction2.x + direction2.y * direction2.y);

                    sf::Vector2f chosenDirection = (length2 >= length) ? direction : direction2;
                    float chosenLength = (length2 >= length) ? length : length2;

                    if (chosenLength != 0) {
                        chosenDirection /= chosenLength;
                    }
                    return chosenDirection;
                }));
            }

            for (size_t i = 0; i < enemies.size(); ++i) {
                sf::Vector2f chosenDirection = futures[i].get();
                enemies[i]->templateMethod(player.getPosition(), player2.getPosition(), player, player2, window, chosenDirection);
            }

            for (auto& enemy : enemies) {
                if (player._isAttacking() && player.checkCollision(player.getPosition(), enemy->getPosition(), player.getSize(), enemy->getSize())) {
                    enemy->receiveDamage(player.getDamage());
                    break;
                }
                if (player2._isAttacking() && player2.checkCollision(player2.getPosition(), enemy->getPosition(), player2.getSize(), enemy->getSize())) {
                    enemy->receiveDamage(player2.getDamage());
                    break;
                }
            }

            stats1->draw(window);
            stats2->draw(window);
            if (!player.isAlive() && !player2.isAlive()) {
                state = GAME_OVER;
            }
            for (auto it = enemies.begin(); it != enemies.end();) {
                if ((*it)->getVIDAWAA() <= 0) {
                    it = enemies.erase(it);
                } else {
                    ++it;
                }
            }
        } else if (state == PAUSED) {
            window.setView(defaultView);
            window.draw(pauseOption);
        } else if (state == GAME_OVER) {
            window.setView(defaultView);
            window.draw(gameOverText);
        }

        window.display();
    }
    return 0;
}
