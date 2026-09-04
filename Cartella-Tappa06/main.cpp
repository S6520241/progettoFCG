#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <random>

struct Bullet {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
};

struct Enemy {
    sf::RectangleShape shape;
    float speed;
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 06");
    window.setFramerateLimit(60);

    // --- CARICAMENTO FONT ---
    sf::Font font;
    if (!font.openFromFile("../Cartella-risorse/font.ttf")) {
        std::cerr << "Attenzione: Impossibile caricare ../Cartella-risorse/font.ttf. Assicurati che il file esista!" << std::endl;
    }

    // --- VARIABILI TESTO E TIMER ---
    sf::Text timerText(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition({10.f, 10.f});
    
    sf::Clock survivalClock;
    sf::Clock enemySpawnClock;

    // --- VARIABILI DEL GIOCATORE ---
    sf::RectangleShape player(sf::Vector2f({50.f, 20.f}));
    player.setFillColor(sf::Color::Green);
    player.setOrigin({25.f, 10.f});
    player.setPosition({400.f, 300.f});
    const float playerSpeed = 5.0f;

    // --- VARIABILI DEI PROIETTILI ---
    std::vector<Bullet> bullets;
    const float bulletSpeed = 15.0f;

    // --- VARIABILI DEI NEMICI ---
    std::vector<Enemy> enemies;
    const float baseEnemySpeed = 2.0f;
    const float spawnInterval = 1.5f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(50.f, 750.f);
    std::uniform_real_distribution<float> disY(50.f, 550.f);
    std::uniform_int_distribution<int> disEdge(0, 3);

    std::cout << "GIOCO AVVIATO DIRETTAMENTE IN GAMEPLAY. Usa WASD/Frecce per muoverti, Mouse per mirare, Click sinistro per sparare." << std::endl;

    while (window.isOpen()) {
        
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

        // --- GESTIONE EVENTI ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close(); // Premi ESC per uscire direttamente
                }
            }

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    Bullet newBullet;
                    newBullet.shape = sf::RectangleShape(sf::Vector2f({15.f, 5.f}));
                    newBullet.shape.setFillColor(sf::Color::Yellow);
                    newBullet.shape.setOrigin({7.5f, 2.5f});
                    
                    sf::Vector2f playerPos = player.getPosition();
                    newBullet.shape.setPosition(playerPos);
                    newBullet.shape.setRotation(player.getRotation());

                    sf::Vector2f direction = mousePos - playerPos;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    
                    if (length != 0.f) {
                        newBullet.velocity = { (direction.x / length) * bulletSpeed, (direction.y / length) * bulletSpeed };
                        bullets.push_back(newBullet);
                    }
                }
            }
        }

        // --- AGGIORNAMENTO LOGICA (Sempre attiva) ---
        float timeElapsed = survivalClock.getElapsedTime().asSeconds();
        timerText.setString("Tempo: " + std::to_string(static_cast<int>(timeElapsed)) + "s");

        // 1. Rotazione e Movimento Navicella
        sf::Vector2f playerPos = player.getPosition();
        float dx = mousePos.x - playerPos.x;
        float dy = mousePos.y - playerPos.y;
        float angleRad = std::atan2(dy, dx);
        player.setRotation(sf::radians(angleRad));

        sf::Vector2f movement(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= playerSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += playerSpeed;
        player.move(movement);

        sf::Vector2f pos = player.getPosition();
        float hw = player.getSize().x / 2.f;
        float hh = player.getSize().y / 2.f;
        if (pos.x < hw) pos.x = hw;
        if (pos.x > 800.f - hw) pos.x = 800.f - hw;
        if (pos.y < hh) pos.y = hh;
        if (pos.y > 600.f - hh) pos.y = 600.f - hh;
        player.setPosition(pos);

        // 2. Generazione Nemici
        if (enemySpawnClock.getElapsedTime().asSeconds() > spawnInterval) {
            Enemy newEnemy;
            newEnemy.shape = sf::RectangleShape(sf::Vector2f({30.f, 30.f}));
            newEnemy.shape.setFillColor(sf::Color::Red);
            newEnemy.shape.setOrigin({15.f, 15.f});
            newEnemy.speed = baseEnemySpeed + (timeElapsed * 0.02f);
            
            int edge = disEdge(gen);
            float spawnX = 0, spawnY = 0;
            switch (edge) {
                case 0: spawnX = disX(gen); spawnY = 30.f; break;
                case 1: spawnX = 770.f; spawnY = disY(gen); break;
                case 2: spawnX = disX(gen); spawnY = 570.f; break;
                case 3: spawnX = 30.f; spawnY = disY(gen); break;
            }
            newEnemy.shape.setPosition({spawnX, spawnY});
            enemies.push_back(newEnemy);
            
            std::cout << "DEBUG: Nemico generato! Totale: " << enemies.size() << std::endl;
            enemySpawnClock.restart();
        }

        // 3. Inseguimento Nemici
        for (auto& enemy : enemies) {
            sf::Vector2f ePos = enemy.shape.getPosition();
            sf::Vector2f dirToPlayer = player.getPosition() - ePos;
            float dist = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
            
            if (dist > 0) {
                enemy.shape.move({(dirToPlayer.x / dist) * enemy.speed, (dirToPlayer.y / dist) * enemy.speed});
            }
            float eAngleRad = std::atan2(dirToPlayer.y, dirToPlayer.x);
            enemy.shape.setRotation(sf::radians(eAngleRad));
        }

        // 4. Collisioni Proiettili
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            it->shape.move(it->velocity);
            bool bulletDestroyed = false;

            for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
                if (it->shape.getGlobalBounds().findIntersection(eIt->shape.getGlobalBounds())) {
                    eIt = enemies.erase(eIt);
                    bulletDestroyed = true;
                    break;
                } else {
                    ++eIt;
                }
            }

            sf::Vector2f bPos = it->shape.getPosition();
            if (bulletDestroyed || bPos.x < 0.f || bPos.x > 800.f || bPos.y < 0.f || bPos.y > 600.f) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }

        // --- RENDERING ---
        window.clear(sf::Color::Black);
        for (const auto& bullet : bullets) {
            window.draw(bullet.shape);
        }
        for (const auto& enemy : enemies) {
            window.draw(enemy.shape);
        }
        window.draw(player);
        window.draw(timerText);
        
        window.display();
    }
    return 0;
}