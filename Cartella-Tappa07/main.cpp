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

struct PowerUp {
    sf::RectangleShape shape;
    sf::Clock lifespan;
};

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 07");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    // --- CARICAMENTO FONT ---
    sf::Font font;
    if (!font.openFromFile("../Cartella-risorse/font.ttf")) {
        std::cerr << "Attenzione: Impossibile caricare ../Cartella-risorse/font.ttf" << std::endl;
    }

    // --- UI: TIMER E PUNTEGGIO ---
    sf::Text timerText(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition({10.f, 10.f});

    int score = 0;
    sf::Text scoreText(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setPosition({630.f, 10.f}); // In alto a destra
    
    sf::Clock survivalClock;
    sf::Clock enemySpawnClock;
    
    // --- GESTIONE POWER-UP ---
    std::vector<PowerUp> powerUps;
    sf::Clock powerUpSpawnClock;
    bool hasSpreadShot = false;
    sf::Clock spreadShotTimer;
    const float powerUpSpawnInterval = 10.0f; // Un power-up ogni 10 secondi

    // --- VARIABILI GIOCATORE ---
    sf::RectangleShape player(sf::Vector2f({50.f, 20.f}));
    player.setFillColor(sf::Color::Green);
    player.setOrigin({25.f, 10.f});
    player.setPosition({400.f, 300.f});
    const float playerSpeed = 5.0f;

    // --- VARIABILI PROIETTILI ---
    std::vector<Bullet> bullets;
    const float bulletSpeed = 15.0f;

    // --- VARIABILI NEMICI ---
    std::vector<Enemy> enemies;
    const float baseEnemySpeed = 2.0f;
    const float spawnInterval = 2.5f; 
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disX(50.f, 750.f);
    std::uniform_real_distribution<float> disY(50.f, 550.f);
    std::uniform_int_distribution<int> disEdge(0, 3);

    std::cout << "Sei nel MAIN MENU. Clicca sulla finestra e premi INVIO per giocare." << std::endl;

    while (window.isOpen()) {
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    player.setPosition({400.f, 300.f});
                    player.setRotation(sf::degrees(0.f));
                    bullets.clear();
                    enemies.clear();
                    powerUps.clear();
                    score = 0;
                    hasSpreadShot = false;
                    survivalClock.restart();
                    enemySpawnClock.restart();
                    powerUpSpawnClock.restart();
                }
                else if (currentState == GameState::Gameplay && keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    currentState = GameState::GameOver;
                }
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                }
            }

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (currentState == GameState::Gameplay && mousePressed->button == sf::Mouse::Button::Left) {
                    
                    sf::Vector2f playerPos = player.getPosition();
                    sf::Vector2f direction = mousePos - playerPos;
                    float baseAngle = std::atan2(direction.y, direction.x);
                    
                    // Funzione lambda per creare un singolo proiettile con un dato angolo
                    auto spawnBullet = [&](float angleOffset) {
                        Bullet newBullet;
                        newBullet.shape = sf::RectangleShape(sf::Vector2f({15.f, 5.f}));
                        newBullet.shape.setFillColor(sf::Color::Yellow);
                        newBullet.shape.setOrigin({7.5f, 2.5f});
                        newBullet.shape.setPosition(playerPos);
                        
                        float finalAngle = baseAngle + angleOffset;
                        newBullet.shape.setRotation(sf::radians(finalAngle));
                        newBullet.velocity = { std::cos(finalAngle) * bulletSpeed, std::sin(finalAngle) * bulletSpeed };
                        bullets.push_back(newBullet);
                    };

                    if (hasSpreadShot) {
                        spawnBullet(0.f);          // Centro
                        spawnBullet(-0.25f);       // Sinistra (~15 gradi)
                        spawnBullet(0.25f);        // Destra (~15 gradi)
                    } else {
                        spawnBullet(0.f);          // Sparo standard singolo
                    }
                }
            }
        }

        if (currentState == GameState::Gameplay) {
            float timeElapsed = survivalClock.getElapsedTime().asSeconds();
            timerText.setString("Tempo: " + std::to_string(static_cast<int>(timeElapsed)) + "s");
            scoreText.setString("Score: " + std::to_string(score));

            // Disattiva il power-up dopo 5 secondi
            if (hasSpreadShot && spreadShotTimer.getElapsedTime().asSeconds() > 5.0f) {
                hasSpreadShot = false;
            }

            // 1. Movimento Navicella
            sf::Vector2f playerPos = player.getPosition();
            float dx = mousePos.x - playerPos.x;
            float dy = mousePos.y - playerPos.y;
            player.setRotation(sf::radians(std::atan2(dy, dx)));

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

            // 2. Generazione e Inseguimento Nemici
            if (enemySpawnClock.getElapsedTime().asSeconds() > spawnInterval && enemies.size() < 15) {
                Enemy newEnemy;
                newEnemy.shape = sf::RectangleShape(sf::Vector2f({30.f, 30.f}));
                newEnemy.shape.setFillColor(sf::Color::Red);
                newEnemy.shape.setOrigin({15.f, 15.f});
                newEnemy.speed = baseEnemySpeed + (timeElapsed * 0.01f);
                
                int edge = disEdge(gen);
                float spawnX = 0, spawnY = 0;
                switch (edge) {
                    case 0: spawnX = disX(gen); spawnY = -30.f; break;
                    case 1: spawnX = 830.f; spawnY = disY(gen); break;
                    case 2: spawnX = disX(gen); spawnY = 630.f; break;
                    case 3: spawnX = -30.f; spawnY = disY(gen); break;
                }
                newEnemy.shape.setPosition({spawnX, spawnY});
                enemies.push_back(newEnemy);
                enemySpawnClock.restart();
            }

            for (auto& enemy : enemies) {
                sf::Vector2f ePos = enemy.shape.getPosition();
                sf::Vector2f dirToPlayer = player.getPosition() - ePos;
                float dist = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
                if (dist > 0) {
                    enemy.shape.move({(dirToPlayer.x / dist) * enemy.speed, (dirToPlayer.y / dist) * enemy.speed});
                }
                enemy.shape.setRotation(sf::radians(std::atan2(dirToPlayer.y, dirToPlayer.x)));
            }

            // 3. Generazione Power-Ups
            if (powerUpSpawnClock.getElapsedTime().asSeconds() > powerUpSpawnInterval) {
                PowerUp pup;
                pup.shape = sf::RectangleShape(sf::Vector2f({20.f, 20.f}));
                pup.shape.setFillColor(sf::Color::Cyan); // Cubo azzurro
                pup.shape.setPosition({disX(gen), disY(gen)});
                pup.lifespan.restart();
                powerUps.push_back(pup);
                powerUpSpawnClock.restart();
            }

            // Raccolta Power-up (Collisione Giocatore-PowerUp)
            for (auto it = powerUps.begin(); it != powerUps.end(); ) {
                if (player.getGlobalBounds().findIntersection(it->shape.getGlobalBounds())) {
                    hasSpreadShot = true;
                    spreadShotTimer.restart();
                    it = powerUps.erase(it);
                } 
                else if (it->lifespan.getElapsedTime().asSeconds() > 7.0f) { // Scompare dopo 7 secondi
                    it = powerUps.erase(it);
                } 
                else {
                    ++it;
                }
            }

            // 4. Collisioni Proiettili-Nemici
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                it->shape.move(it->velocity);
                bool bulletDestroyed = false;

                for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
                    if (it->shape.getGlobalBounds().findIntersection(eIt->shape.getGlobalBounds())) {
                        eIt = enemies.erase(eIt);
                        score += 10; // Aggiunge 10 punti per ogni nemico
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

            // 5. Collisioni Giocatore-Nemico (Game Over)
            for (const auto& enemy : enemies) {
                if (player.getGlobalBounds().findIntersection(enemy.shape.getGlobalBounds())) {
                    currentState = GameState::GameOver;
                    std::cout << "GAME OVER! Punteggio: " << score << " | Tempo: " << static_cast<int>(timeElapsed) << "s" << std::endl;
                    break;
                }
            }
        }

        // --- RENDERING ---
        switch (currentState) {
            case GameState::MainMenu:
                window.clear(sf::Color::Blue);
                break;
            
            case GameState::Gameplay:
                window.clear(sf::Color::Black);
                for (const auto& pup : powerUps) window.draw(pup.shape);
                for (const auto& bullet : bullets) window.draw(bullet.shape);
                for (const auto& enemy : enemies) window.draw(enemy.shape);
                window.draw(player);
                window.draw(timerText);
                window.draw(scoreText);
                break;
            
            case GameState::GameOver:
                window.clear(sf::Color::Red);
                break;
        }
        
        window.display();
    }
    return 0;
}