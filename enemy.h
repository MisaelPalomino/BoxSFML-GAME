#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <memory>
class Enemy {
public:
    Enemy(sf::Vector2f initialPosition)
        : position(initialPosition), currentFrame(0), animationInterval(sf::seconds(0.1f)) {
    }
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void update(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2,sf::Vector2f chosenDirection) = 0;
    void templateMethod(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2, sf::RenderWindow& window,sf::Vector2f chosenDirection) {
        update(playerPosition, player2Position, player, player2,chosenDirection);
        draw(window);
    }
    virtual bool checkCollision(const sf::Vector2f& newPosition, const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position) {
        sf::FloatRect enemyBounds(newPosition - boundingBox.getOrigin(), boundingBox.getSize());

        sf::FloatRect playerBounds = getPlayerBoundsAdjusted(playerPosition, newPosition);
        sf::FloatRect player2Bounds = getPlayerBoundsAdjusted(player2Position, newPosition);

        return enemyBounds.intersects(playerBounds) || enemyBounds.intersects(player2Bounds);
    }
    virtual sf::FloatRect getPlayerBoundsAdjusted(const sf::Vector2f& playerPosition, const sf::Vector2f& enemyPosition) const {
        sf::FloatRect adjustedBounds(playerPosition - boundingBox.getOrigin(), boundingBox.getSize());
        return adjustedBounds;
    }

    virtual void attack(Player& player) = 0;

    virtual void receiveDamage(float amount) {
        vida -= amount;
        if (vida < 0) {
            vida = 0;
        }
    };
    virtual bool isDead() const {
        return vida <= 0;
    };

    virtual ~Enemy() = default;

    virtual sf::Vector2f getPosition() const {
        return sf::Vector2f(position.x,position.y);
    };
    sf::Vector2f getSize() const {
        return sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
    }
    float getVIDAWAA(){return vida;}
protected:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::Vector2f position;
    sf::RectangleShape boundingBox;
    unsigned int currentFrame;
    sf::Clock animationClock;
    sf::Time animationInterval;
    float speed;
    float vida=1000;
    float daño;
    sf::Clock moveClock;
    sf::Time moveTimer;
    sf::Time moveInterval = sf::seconds(0.1f);

    virtual void updateAnimation(const sf::Vector2f& direction, bool isMoving) = 0;
};

class Orco : public Enemy {
public:
    Orco(const std::string& filename, sf::Vector2f initialPosition)
        : Enemy(initialPosition) {
        if (!texture.loadFromFile(filename)) {
            throw std::runtime_error("Unable to load texture: " + filename);
        }

        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
        sprite.setScale(0.45f, 0.45f);
        sprite.setOrigin(32.0f, 32.0f);
        boundingBox.setSize(sf::Vector2f(64 * 0.15f, 64 * 0.15f));
        boundingBox.setOrigin(boundingBox.getSize() / 2.0f);
        boundingBox.setPosition(initialPosition);
    }

    void updateAnimation(const sf::Vector2f& direction, bool isMoving) override {
        if (isMoving) {
            if (animationClock.getElapsedTime() > animationInterval) {
                int row = 0;
                if (std::abs(direction.x) > std::abs(direction.y)) {
                    row = (direction.x < 0) ? 3 : 1; // Izquierda o Derecha
                } else {
                    row = (direction.y < 0) ? 2 : 0; // Arriba o Abajo
                }
                currentFrame = (currentFrame + 1) % frameCount;
                sprite.setTextureRect(sf::IntRect(10 + currentFrame * frameWidth, row * frameHeight, frameWidth, frameHeight));
                animationClock.restart();
            }
        } else {
            sf::Time adjustedInterval = animationInterval / 2.0f;
            if (animationClock.getElapsedTime() > adjustedInterval) {
                int row = 0;
                if (std::abs(direction.x) > std::abs(direction.y)) {
                    row = (direction.x < 0) ? 3 : 1; // Izquierda o Derecha
                } else {
                    row = (direction.y < 0) ? 2 : 0; // Arriba o Abajo
                }
                currentFrame = (currentFrame + 1) % 3;
                sprite.setTextureRect(sf::IntRect(458 + currentFrame * frameWidth, row * frameHeight, frameWidth, frameHeight));
                animationClock.restart();
            }
        }
    }

    void update(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2, sf::Vector2f chosenDirection) override {

            float increasedSpeed = 1.0f;
            chosenDirection *= speed * increasedSpeed;

            moveTimer += moveClock.restart();
            if (moveTimer >= moveInterval) {
                sf::Vector2f newPosition = position + chosenDirection;
                if (!checkCollision(newPosition, playerPosition, player2Position)) {
                    position = newPosition;
                    boundingBox.setPosition(position);
                    moveTimer = sf::Time::Zero;
                    updateAnimation(chosenDirection, true);
                } else {
                    updateAnimation(chosenDirection, false);
                    if (checkCollision(newPosition, player2Position, player2Position)) {
                        attack(player2);
                    } else {
                        attack(player);
                    }
                }
            }

            sprite.setPosition(position);
    }

    void attack(Player& player) override {
        if (attackClock.getElapsedTime() >= attackInterval) {
            player.takeDamage(daño);
            attackClock.restart();
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(sprite);
    }

private:
    static const int frameWidth = 64;
    static const int frameHeight = 64;
    static const int frameCount = 8;
    float speed = 1.5f;
    float vida = 100;
    float daño = 10;
    sf::Clock attackClock;
    sf::Time attackInterval = sf::seconds(1.0f);
};

class Slime : public Enemy {
public:
    Slime(const std::string& filename, sf::Vector2f initialPosition)
        : Enemy(initialPosition) {
        if (!texture.loadFromFile(filename)) {
            throw std::runtime_error("Unable to load texture: " + filename);
        }

        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        sprite.setScale(0.25f, 0.25f); 
        sprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);
        boundingBox.setSize(sf::Vector2f(frameWidth * 0.25f, frameHeight * 0.25f));
        boundingBox.setOrigin(boundingBox.getSize() / 2.0f);
        boundingBox.setPosition(initialPosition);
    }

    void updateAnimation(const sf::Vector2f& direction, bool isMoving) override {
        if (isMoving || attackClock.getElapsedTime() < attackInterval) {
            if (animationClock.getElapsedTime() > animationInterval) {
                currentFrame = (currentFrame + 1) % frameCount;
                sprite.setTextureRect(sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight));
                animationClock.restart();
            }
        } else {
            
            sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        }
    }

    void update(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2,sf::Vector2f chosenDirection) override {
            float increasedSpeed = 1.0f;
            chosenDirection *= speed * increasedSpeed;

            moveTimer += moveClock.restart();
            if (moveTimer >= moveInterval) {
                sf::Vector2f newPosition = position + chosenDirection;
                if (!checkCollision(newPosition, playerPosition, player2Position)) {
                    position = newPosition;
                    boundingBox.setPosition(position);
                    moveTimer = sf::Time::Zero;
                    updateAnimation(chosenDirection, true);
                } else {
                    updateAnimation(chosenDirection, false);
                    if (checkCollision(newPosition, player2Position, player2Position)) {
                        attack(player2);
                    } else {
                        attack(player);
                    }
                }
            }

        sprite.setPosition(position);
    }

    void attack(Player& player) override {
        if (attackClock.getElapsedTime() >= attackInterval) {
            player.takeDamage(daño);
            attackClock.restart();
        }
        
        updateAnimation(sprite.getPosition(), true);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(sprite);
    }

protected:
    static const int frameWidth = 64;
    static const int frameHeight = 64;
    static const int frameCount = 6; 
    float speed = 0.8f; 
    float vida = 30; 
    float daño = 6; 
    sf::Clock attackClock;
    sf::Time attackInterval = sf::seconds(2.0f); 
};

class SlimeDecorator : public Enemy {
public:
    SlimeDecorator(std::unique_ptr<Enemy> slime)
        : Enemy(sf::Vector2f(0, 0)), decoratedSlime(std::move(slime)) {
    }

    void draw(sf::RenderWindow& window) override {
        decoratedSlime->draw(window);
    }

    void update(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2,sf::Vector2f chosenDirection) override {
        decoratedSlime->update(playerPosition, player2Position, player, player2,chosenDirection);
    }

    void attack(Player& player) override {
        decoratedSlime->attack(player);
    }

protected:
    std::unique_ptr<Enemy> decoratedSlime;
};

class FireSlimeDecorator : public SlimeDecorator {
public:
    FireSlimeDecorator(std::unique_ptr<Enemy> slime)
        : SlimeDecorator(std::move(slime)) {}

    void attack(Player& player) override {
        SlimeDecorator::attack(player);
        player.takeDamage(fireDamage);
        std::cout << "Fire damage applied!" << std::endl;
    }

    void update(const sf::Vector2f& playerPosition, const sf::Vector2f& player2Position, Player& player, Player& player2, sf::Vector2f chosenDirection) override {
        SlimeDecorator::update(playerPosition, player2Position, player, player2, chosenDirection);
    }

    void updateAnimation(const sf::Vector2f& direction, bool isMoving) override {
    }

private:
    float fireDamage = 3.0f;
};


class FactoryEnemy {
public:
    static std::unique_ptr<Enemy> createEnemy(const std::string& type, const std::string& filename, sf::Vector2f initialPosition, bool withFire = false) {
        if (type == "orco") {
            return std::make_unique<Orco>(filename, initialPosition);
        } else if (type == "slime") {
            auto slime = std::make_unique<Slime>(filename, initialPosition);
            if (withFire) {
                return std::make_unique<FireSlimeDecorator>(std::move(slime));
            } else {
                return slime;
            }
        }
        throw std::invalid_argument("Unknown enemy type");
    }
};