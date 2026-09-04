#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cmath> 

// --- STRUTTURA PROIETTILI ---
struct Bullet {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
};

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 05");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    // --- VARIABILI DEL GIOCATORE ---
    sf::RectangleShape player(sf::Vector2f({50.f, 20.f}));
    player.setFillColor(sf::Color::Green);
    
    player.setOrigin({25.f, 10.f});
    // Partenza dal centro dello schermo (movimento libero)
    player.setPosition({400.f, 300.f});

    const float playerSpeed = 5.0f;

    // --- VARIABILI DEI PROIETTILI ---
    std::vector<Bullet> bullets;
    const float bulletSpeed = 15.0f;

    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;

    while (window.isOpen()) {
        
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

        // --- GESTIONE EVENTI ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Input Tastiera per gli stati del gioco
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    std::cout << "Sei nel GAMEPLAY. Usa WASD o FRECCE per muoverti, MOUSE per mirare, CLICK SINISTRO per sparare." << std::endl;
                }
                else if (currentState == GameState::Gameplay) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                        currentState = GameState::GameOver;
                        std::cout << "Sei nel GAME OVER. Premi INVIO per tornare al menu." << std::endl;
                    }
                }
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;
                    
                    // Reset partita
                    player.setPosition({400.f, 300.f});
                    player.setRotation(sf::degrees(0.f));
                    bullets.clear();
                }
            }

            // Input Mouse per lo Sparo 
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (currentState == GameState::Gameplay && mousePressed->button == sf::Mouse::Button::Left) {
                    
                    Bullet newBullet;
                    newBullet.shape = sf::RectangleShape(sf::Vector2f({15.f, 5.f})); // Più stretto e lungo per indicare il senso di marcia
                    newBullet.shape.setFillColor(sf::Color::Yellow);
                    newBullet.shape.setOrigin({7.5f, 2.5f});
                    
                    // Genera il proiettile dal centro del giocatore con la stessa angolazione
                    sf::Vector2f playerPos = player.getPosition();
                    newBullet.shape.setPosition(playerPos);
                    newBullet.shape.setRotation(player.getRotation());

                    // Calcolo Vettore Direzione verso il mouse
                    sf::Vector2f direction = mousePos - playerPos;
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    
                    // Normalizzazione del vettore e applicazione della velocità
                    if (length != 0.f) {
                        newBullet.velocity = { (direction.x / length) * bulletSpeed, (direction.y / length) * bulletSpeed };
                        bullets.push_back(newBullet);
                    }
                }
            }
        }

        // --- AGGIORNAMENTO LOGICA ---
        if (currentState == GameState::Gameplay) {
            
            // 1. Rotazione Navicella
            sf::Vector2f playerPos = player.getPosition();
            float dx = mousePos.x - playerPos.x;
            float dy = mousePos.y - playerPos.y;
            float angleRad = std::atan2(dy, dx);
            player.setRotation(sf::radians(angleRad));

            // 2. Movimento Omnidirezionale 
            sf::Vector2f movement(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                movement.x -= playerSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                movement.x += playerSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                movement.y -= playerSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                movement.y += playerSpeed;
            }
            player.move(movement);

            // 3. Sistema di collisione Navicella-Bordi 
            sf::Vector2f pos = player.getPosition();
            float hw = player.getSize().x / 2.f;
            float hh = player.getSize().y / 2.f;

            if (pos.x < hw) pos.x = hw;
            if (pos.x > 800.f - hw) pos.x = 800.f - hw;
            if (pos.y < hh) pos.y = hh;
            if (pos.y > 600.f - hh) pos.y = 600.f - hh;
            player.setPosition(pos);

            // 4. Aggiornamento posizione e Garbage Collection Proiettili
            for (auto& bullet : bullets) {
                bullet.shape.move(bullet.velocity);
            }

            // Distruzione dei proiettili se escono da qualsiasi lato della finestra
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                sf::Vector2f bPos = it->shape.getPosition();
                if (bPos.x < 0.f || bPos.x > 800.f || bPos.y < 0.f || bPos.y > 600.f) {
                    it = bullets.erase(it);
                } else {
                    ++it;
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
                for (const auto& bullet : bullets) {
                    window.draw(bullet.shape);
                }
                window.draw(player);
                break;
            
            case GameState::GameOver:
                window.clear(sf::Color::Red);
                break;
        }
        
        window.display();
    }
    return 0;
}