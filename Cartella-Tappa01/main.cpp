#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

int main() {
    // Creazione della finestra
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa 01");
    
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        
        // Gestione degli eventi
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Pulizia dello schermo
        window.clear(sf::Color::Black);


        // Visualizzazione a schermo
        window.display();
    }

    return 0;
}