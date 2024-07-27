#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "stats.h"

class Player {
public:
    Player(const std::string& filename, sf::Vector2u tileSize, unsigned int numFrames, const int* level, unsigned int width, unsigned int height);
    void handleInput();
    void setHandleInput(sf::Keyboard::Key upKey, sf::Keyboard::Key downKey, sf::Keyboard::Key leftKey, sf::Keyboard::Key rightKey);
    void update();
    void draw(sf::RenderWindow& window);
    void setTexture(const std::string& filename) {
        texture.loadFromFile(filename);
    }
    sf::Vector2f getPosition() const {
        sf::Vector2f camara = sprite.getPosition();
        camara.x += 8.0f;
        camara.y += 8.0f;
        return camara;
    };
    bool checkCollision(const sf::Vector2f& playerPosition, const sf::Vector2f& enemyPosition, const sf::Vector2f& playerSize, const sf::Vector2f& enemySize);
    void takeDamage(float amount);  
    bool isAlive() const {
        return health > 0; 
    }
    bool _isAttacking(){
        return isAttacking;
    }
    void reset() {
        health = 100; 
    }
    void addObserver(std::shared_ptr<Observer> observer) {
        observers.push_back(observer);
    }
    void removeObserver(std::shared_ptr<Observer> observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }
    void notifyObservers() {
        for (const auto& observer : observers) {
            observer->update(health, shield, speed, damage);
        }
    }
    float getDamage(){return damage;}
    sf::Vector2f getSize() const {
    return sf::Vector2f(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
    }
private:
    sf::Sprite sprite;
    sf::Texture texture;
    sf::RectangleShape boundingBox;
    sf::Vector2u tileSize;
    sf::Vector2f position;
    unsigned int numFrames;
    unsigned int currentFrame;
    sf::Clock animationClock;
    sf::Clock movementClock;
    sf::Clock attackClock;  // Añadido para controlar la duración del ataque
    sf::Clock jumpClock;  // Para controlar la duración del salto
    sf::Time animationInterval;
    sf::Time movementInterval;
    sf::Time attackDuration;  // Duración del ataque
    sf::Time jumpDuration;  // Duración del salto
    sf::Vector2f initialPosition;
    int positionRectSprite; //Para el sprite
    const int* level; //Almacenar el mapa
    unsigned int width, height; //Dimension del mapa
    bool isAttacking;  // Añadido para indicar si el jugador está atacando
    bool isRunning;  // Para indicar si el jugador está corriendo
    bool isJumping;  // Para indicar si el jugador está saltando

    float health=100;  // Vida del jugador
    float damage=1;  // Daño que el jugador puede infligir
    float shield=0;  // Escudo del jugador para reducir el daño recibido
    float speed=2.0f;

    sf::Keyboard::Key upKey;
    sf::Keyboard::Key downKey;
    sf::Keyboard::Key leftKey;
    sf::Keyboard::Key rightKey;

    void updateAnimation();
    bool canMoveTo(sf::Vector2f newPosition); //Verificar colision
    void startAttack();  // Añadido para iniciar el ataque
    void updateAttack();  // Añadido para actualizar el estado de ataque
    void startJump();  // Para iniciar el salto
    void updateJump();  // Para actualizar el estado del salto
    std::vector<std::shared_ptr<Observer>> observers;
    void notifyStatsChange();
};

Player::Player(const std::string& filename, sf::Vector2u tileSize, unsigned int numFrames, const int* level, unsigned int width, unsigned int height) 
    : tileSize(tileSize), position(16.f,16.f), numFrames(numFrames), currentFrame(0),
      animationInterval(sf::seconds(0.1f)), movementInterval(sf::seconds(0.2f)),
      attackDuration(sf::seconds(0.001f)),
      jumpDuration(sf::seconds(0.5f)),
      positionRectSprite(32), level(level), width(width), height(height),
      isAttacking(false), isRunning(false), isJumping(false), // Inicializar estados
      health(100), damage(10), shield(5) { // Inicializar atributos
    if (!texture.loadFromFile(filename)) {
        std::cerr << "Error: Imposible cargar la textura " << filename << std::endl;  
        throw std::runtime_error("Imposible cargar la textura");
    }
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(32, 32, tileSize.x/2, tileSize.y/2));
    sprite.setPosition(position.x * tileSize.x, position.y * tileSize.y);
    sprite.setScale(1.0f,1.0f);
    initialPosition = sf::Vector2f(position.x, position.y);
    std::cout << "Player inicializado en la posicion: (" << position.x << ", " << position.y << ")" << std::endl; 
    boundingBox.setSize(sf::Vector2f(64 * 0.15f, 64 * 0.15f));
    boundingBox.setOrigin(boundingBox.getSize() / 2.0f);
    boundingBox.setPosition(initialPosition);
}


void Player::notifyStatsChange() {
    notifyObservers();
}
void Player::setHandleInput(sf::Keyboard::Key up, sf::Keyboard::Key down, sf::Keyboard::Key left, sf::Keyboard::Key right) {
    upKey = up;
    downKey = down;
    leftKey = left;
    rightKey = right;
}

void Player::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        isRunning = true;
        movementInterval = sf::seconds(0.025f);  
    } else {
        isRunning = false;
        movementInterval = sf::seconds(0.05f);  
    }

    if (movementClock.getElapsedTime() > movementInterval) {
        sf::Vector2f newPosition = position;
        if (sf::Keyboard::isKeyPressed(upKey)) {
            positionRectSprite = 272;
            setTexture("resources/Characters/walk.png");
            newPosition.y -= speed;
            movementClock.restart();
        } else if (sf::Keyboard::isKeyPressed(downKey)) {
            positionRectSprite = 192;
            setTexture("resources/Characters/walk.png");
            newPosition.y += speed;
            movementClock.restart();
        } else if (sf::Keyboard::isKeyPressed(leftKey)) {
            positionRectSprite = 112;
            setTexture("resources/Characters/walk.png");
            newPosition.x -= speed;
            movementClock.restart();
        } else if (sf::Keyboard::isKeyPressed(rightKey)) {
            positionRectSprite = 32;
            setTexture("resources/Characters/walk.png");
            newPosition.x += speed;
            movementClock.restart();
        } else {
            setTexture("resources/Characters/idle.png");
        }

        if(canMoveTo(newPosition)) {
            position = newPosition; 
        }
        
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
        startJump();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        startAttack();
    }
    
}

bool Player::canMoveTo(sf::Vector2f newPosition) {
    if (isJumping) {
        return true;
    }
    int xIndex = int(newPosition.x/16+0.5);
    int yIndex = int(newPosition.y/16+0.5);
    
    if (xIndex < 0 || xIndex >= static_cast<int>(width) ||
        yIndex < 0 || yIndex >= static_cast<int>(height)) {
        return false;  
    }

    // Obtener el valor del tile en la nueva posición
    int tileValue = level[xIndex + yIndex * width];
    return tileValue!=0;
}

bool Player::checkCollision(const sf::Vector2f& playerPosition, const sf::Vector2f& enemyPosition, const sf::Vector2f& playerSize, const sf::Vector2f& enemySize) {
    sf::FloatRect playerBounds(playerPosition, playerSize);
    sf::FloatRect enemyBounds(enemyPosition, enemySize);
    return playerBounds.intersects(enemyBounds);
}

void Player::takeDamage(float amount) {
    float actualDamage = amount - shield;
    if (actualDamage < 0) actualDamage = 0;

    health -= actualDamage;
    std::cout << "Player tomo " << actualDamage << " de danio. Vida ahora: " << health << std::endl;

    if (health <= 0) {
        std::cout << "Player vida es 0" << std::endl;
    }
    notifyStatsChange();
}

void Player::startAttack() {
    // Verificar si el jugador no está atacando actualmente y si el tiempo de ataque ha pasado
    if (!isAttacking && attackClock.getElapsedTime() > attackDuration) {
        isAttacking = true;
        attackClock.restart();

    }
}

void Player::updateAttack() {
    if (isAttacking) {
        if (attackClock.getElapsedTime() > attackDuration) {
            isAttacking = false;
            setTexture("resources/Characters/idle.png"); 
        } else {
            setTexture("resources/Characters/damage.png");
            return;
        }
    }
}

void Player::startJump() {
    if (!isJumping) {
        isJumping = true;
        jumpClock.restart();
        std::cout << "Player saltando" << std::endl;
    }
}

void Player::updateJump() {
    if (isJumping) {
        if (jumpClock.getElapsedTime() > jumpDuration) {
            isJumping = false;
            std::cout << "Salto terminado" << std::endl;
        } else {
            setTexture("resources/Characters/jump.png");
            return;
        }
    }
}

void Player::update() {
    sprite.setPosition(position.x, position.y);
    updateAttack();
    updateJump();
    updateAnimation();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

void Player::updateAnimation() {
    if (animationClock.getElapsedTime() > animationInterval) {
        currentFrame = (currentFrame + 1) % numFrames;
        int tu = currentFrame % (texture.getSize().x / tileSize.x);
        sprite.setTextureRect(sf::IntRect(tu * tileSize.x + 32, positionRectSprite, tileSize.x/4, tileSize.y/4));
        animationClock.restart();
    }
}