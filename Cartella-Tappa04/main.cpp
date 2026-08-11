#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector> // Necessario per l'array dinamico (std::vector)

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 04");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    // --- VARIABILI DEL GIOCATORE ---
    sf::RectangleShape player(sf::Vector2f({50.f, 20.f})); 
    player.setFillColor(sf::Color::Green);
    player.setPosition({375.f, 550.f}); 
    const float playerSpeed = 5.0f; 

    // --- VARIABILI DEI PROIETTILI ---
    std::vector<sf::RectangleShape> bullets;
    const float bulletSpeed = -10.0f; // Velocità negativa sull'asse Y per andare verso l'alto

    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;

    while (window.isOpen()) {
        
        // --- GESTIONE EVENTI ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    std::cout << "Sei nel GAMEPLAY. Freccia DX/SX per muoverti, SPAZIO per sparare." << std::endl;
                }
                else if (currentState == GameState::Gameplay) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                        currentState = GameState::GameOver;
                        std::cout << "Sei nel GAME OVER. Premi INVIO per tornare al menu." << std::endl;
                    }
                    // --- GESTIONE SPARO ---
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                        sf::RectangleShape bullet(sf::Vector2f({5.f, 15.f}));
                        bullet.setFillColor(sf::Color::Yellow);
                        
                        sf::Vector2f playerPos = player.getPosition();
                        bullet.setPosition({playerPos.x + 22.5f, playerPos.y});
                        
                        bullets.push_back(bullet);
                    }
                }
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;
                    
                    player.setPosition({375.f, 550.f});
                    bullets.clear(); 
                }
            }
        }

        // --- AGGIORNAMENTO LOGICA ---
        if (currentState == GameState::Gameplay) {
            
            // Movimento navicella
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                player.move({-playerSpeed, 0.f});
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                player.move({playerSpeed, 0.f});
            }

            // Collisioni navicella con i bordi
            sf::Vector2f pos = player.getPosition();
            if (pos.x < 0.f) {
                player.setPosition({0.f, pos.y});
            } else if (pos.x + player.getSize().x > 800.f) {
                player.setPosition({800.f - player.getSize().x, pos.y});
            }

            // ---MOVIMENTO E RIMOZIONE PROIETTILI ---
            for (auto& bullet : bullets) {
                bullet.move({0.f, bulletSpeed});
            }

            // Rimuoviamo i proiettili che escono dalla parte superiore dello schermo (Y < 0)
            for (auto it = bullets.begin(); it != bullets.end(); ) {
                if (it->getPosition().y < 0.f) {
                    it = bullets.erase(it); // Elimina e passa al successivo
                } else {
                    ++it; // Prossimo proiettile
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
                    window.draw(bullet);
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