#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 03");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    // --- 1. VARIABILI DEL GIOCATORE ---
    sf::RectangleShape player(sf::Vector2f({50.f, 20.f})); 
    player.setFillColor(sf::Color::Green);
    
    // Posizioniamola in basso al centro della finestra (800x600)
    // X = 375 (perché 400 - metà della larghezza 25)
    // Y = 550 (vicino al fondo)
    player.setPosition({375.f, 550.f}); 
    
    const float playerSpeed = 5.0f; // Velocità di movimento

    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;

    while (window.isOpen()) {
        
        // --- 2. GESTIONE EVENTI (Cambio schermate) ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    std::cout << "Sei nel GAMEPLAY. Usa le FRECCE DESTRA/SINISTRA per muoverti. Premi ESC per il Game Over." << std::endl;
                }
                else if (currentState == GameState::Gameplay && keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    currentState = GameState::GameOver;
                    std::cout << "Sei nel GAME OVER. Premi INVIO per tornare al menu." << std::endl;
                }
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;
                    
                    // Resettiamo la posizione del giocatore quando rinizia la partita
                    player.setPosition({375.f, 550.f});
                }
            }
        }

        // --- 3. INPUT IN TEMPO REALE E LOGICA DI GIOCO ---
        if (currentState == GameState::Gameplay) {
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                player.move({-playerSpeed, 0.f});
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                player.move({playerSpeed, 0.f});
            }

            // Evitiamo che la navicella esca dai bordi laterali
            sf::Vector2f pos = player.getPosition();
            
            // Limite sinistro
            if (pos.x < 0.f) {
                player.setPosition({0.f, pos.y});
            } 
            // Limite destro (800 meno la larghezza del giocatore)
            else if (pos.x + player.getSize().x > 800.f) {
                player.setPosition({800.f - player.getSize().x, pos.y});
            }
        }

        // --- 4. RENDERING GRAFICO ---
        switch (currentState) {
            case GameState::MainMenu:
                window.clear(sf::Color::Blue);
                break;
                
            case GameState::Gameplay:
                window.clear(sf::Color::Black);
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