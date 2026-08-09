#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

//possibili stati del nostro gioco
enum class GameState {
    MainMenu,
    Gameplay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 02");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;

    while (window.isOpen()) {
        
        // --- 1. GESTIONE EVENTI E INPUT ---
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // pressione dei tasti
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                
                // Transizione: MainMenu -> Gameplay (Premendo Invio)
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    std::cout << "Sei nel GAMEPLAY. Premi ESC per simulare il Game Over." << std::endl;
                }
                // Transizione: Gameplay -> GameOver (Premendo Esc)
                else if (currentState == GameState::Gameplay && keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    currentState = GameState::GameOver;
                    std::cout << "Sei nel GAME OVER. Premi INVIO per tornare al menu." << std::endl;
                }
                // Transizione: GameOver -> MainMenu (Premendo Invio)
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                    std::cout << "Sei nel MAIN MENU. Premi INVIO per giocare." << std::endl;
                }
            }
        }

        // --- 2. AGGIORNAMENTO E RENDERING ---
        switch (currentState) {
            case GameState::MainMenu:
                window.clear(sf::Color::Blue);
                break;
                
            case GameState::Gameplay:
                window.clear(sf::Color::Black);
                break;
                
            case GameState::GameOver:
                window.clear(sf::Color::Red);
                break;
        }

        window.display();
    }

    return 0;
}