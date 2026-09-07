#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <random>

struct Bullet {
    sf::Sprite sprite;
    sf::Vector2f velocity;
};

struct Enemy {
    sf::Sprite sprite;
    float speed;
    int hp;
    bool isShooter;
    sf::Clock shootClock;
};

struct PowerUp {
    sf::Sprite sprite;
    int type; // 1: Spread Shot, 2: Shield
    sf::Clock lifespan;
};

struct Star {
    sf::CircleShape shape;
    float speed;
};

struct Particle {
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    sf::Clock lifeClock;
    float maxLife;
};

enum class GameState {
    MainMenu,
    Gameplay,
    Paused,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Space Invaders - Tappa Finale (Sprite & Texture)");
    window.setFramerateLimit(60);

    GameState currentState = GameState::MainMenu;

    // --- CARICAMENTO RISORSE (Font, Texture, Audio) ---
    sf::Font font;
    if (!font.openFromFile("../Cartella-risorse/font.ttf")) {
        std::cerr << "Attenzione: Impossibile caricare ../Cartella-risorse/font.ttf" << std::endl;
    }

    sf::Texture playerTex, enemyTex, shooterTex, tankTex, bulletTex, pupSpreadTex, pupShieldTex;
    bool texturesLoaded = true;

    if (!playerTex.loadFromFile("../Cartella-risorse/player.png") ||
        !enemyTex.loadFromFile("../Cartella-risorse/enemy.png") ||
        !shooterTex.loadFromFile("../Cartella-risorse/shooter.png") ||
        !tankTex.loadFromFile("../Cartella-risorse/tank.png") ||
        !bulletTex.loadFromFile("../Cartella-risorse/bullet.png") ||
        !pupSpreadTex.loadFromFile("../Cartella-risorse/powerup_spread.png") ||
        !pupShieldTex.loadFromFile("../Cartella-risorse/powerup_shield.png")) {
        texturesLoaded = false;
        std::cerr << "Avviso: Impossibile caricare alcune texture. Assicurati che i file PNG siano in Cartella-risorse." << std::endl;
    }

    sf::SoundBuffer shootBuffer, explosionBuffer;
    bool audioLoaded = true;
    if (!shootBuffer.loadFromFile("../Cartella-risorse/shoot.wav") ||
        !explosionBuffer.loadFromFile("../Cartella-risorse/explosion.wav")) {
        audioLoaded = false;
    }
    
    sf::Sound shootSound(shootBuffer);
    sf::Sound explosionSound(explosionBuffer);

    // --- UI HUD ---
    sf::Text statsText(font);
    statsText.setCharacterSize(20);
    statsText.setFillColor(sf::Color::White);
    statsText.setPosition({10.f, 10.f});

    sf::Text timerText(font);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::Cyan);
    timerText.setPosition({350.f, 10.f});

    sf::Text centerMessage(font);
    centerMessage.setCharacterSize(28);
    centerMessage.setFillColor(sf::Color::Yellow);

    int killScore = 0;
    int totalScore = 0;
    int lives = 3;
    float finalTimeRecorded = 0.f;
    
    sf::Clock survivalClock;
    sf::Clock enemySpawnClock;
    sf::Clock invulnerabilityClock;
    bool isInvulnerable = false;
    
    // --- SCREEN SHAKE ---
    sf::Clock shakeClock;
    float shakeDuration = 0.f;
    float shakeIntensity = 5.f;

    // --- STELLE BACKGROUND ---
    std::vector<Star> stars;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> disStarX(0.f, 800.f);
    std::uniform_real_distribution<float> disStarY(0.f, 600.f);
    std::uniform_real_distribution<float> disStarSpeed(0.5f, 3.0f);
    
    for(int i = 0; i < 100; ++i) {
        Star s;
        float radius = (disStarSpeed(gen) > 2.0f) ? 2.f : 1.f;
        s.shape = sf::CircleShape(radius);
        s.shape.setFillColor(sf::Color(255, 255, 255, 150));
        s.shape.setPosition({disStarX(gen), disStarY(gen)});
        s.speed = disStarSpeed(gen);
        stars.push_back(s);
    }

    std::vector<PowerUp> powerUps;
    sf::Clock powerUpSpawnClock;
    bool hasSpreadShot = false;
    bool hasShield = false;
    sf::Clock spreadShotTimer;
    
    sf::CircleShape shieldVisual(50.f);
    shieldVisual.setFillColor(sf::Color(0, 191, 255, 70));
    shieldVisual.setOutlineColor(sf::Color::Cyan);
    shieldVisual.setOutlineThickness(2.f);
    shieldVisual.setOrigin({50.f, 50.f});

    const float powerUpSpawnInterval = 10.0f;
    std::vector<Particle> particles;

    // --- GIOCATORE ---
    sf::Sprite player(playerTex);
    if (texturesLoaded) {
        sf::Vector2u pSize = playerTex.getSize();
        player.setOrigin({static_cast<float>(pSize.x) / 2.f, static_cast<float>(pSize.y) / 2.f});
        player.setScale({90.f / static_cast<float>(pSize.x), 90.f / static_cast<float>(pSize.y)});
    }
    player.setPosition({400.f, 300.f});
    const float playerSpeed = 5.0f;

    std::vector<Bullet> bullets;       
    std::vector<Bullet> enemyBullets;  
    const float bulletSpeed = 15.0f;
    const float enemyBulletSpeed = 7.0f;

    std::vector<Enemy> enemies;
    const float baseEnemySpeed = 2.0f;
    
    std::uniform_real_distribution<float> disX(50.f, 750.f);
    std::uniform_real_distribution<float> disY(50.f, 550.f);
    std::uniform_int_distribution<int> disEdge(0, 3);
    std::uniform_int_distribution<int> disEnemyType(1, 100);
    std::uniform_int_distribution<int> disPowerUpType(1, 2);

    auto triggerShake = [&](float duration) {
        shakeDuration = duration;
        shakeClock.restart();
    };

    auto spawnExplosion = [&](sf::Vector2f pos, sf::Color col) {
        if (audioLoaded) explosionSound.play();
        std::uniform_real_distribution<float> disVel(-4.f, 4.f);
        std::uniform_real_distribution<float> disLife(0.3f, 0.8f);
        for (int i = 0; i < 12; ++i) {
            Particle p;
            p.shape = sf::RectangleShape(sf::Vector2f({6.f, 6.f}));
            p.shape.setFillColor(col);
            p.shape.setOrigin({3.f, 3.f});
            p.shape.setPosition(pos);
            p.velocity = {disVel(gen), disVel(gen)};
            p.maxLife = disLife(gen);
            p.lifeClock.restart();
            particles.push_back(p);
        }
    };

    while (window.isOpen()) {
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (currentState == GameState::MainMenu && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::Gameplay;
                    player.setPosition({400.f, 300.f});
                    bullets.clear();
                    enemyBullets.clear();
                    enemies.clear();
                    powerUps.clear();
                    particles.clear();
                    killScore = 0;
                    totalScore = 0;
                    lives = 3;
                    hasSpreadShot = false;
                    hasShield = false;
                    isInvulnerable = false;
                    player.setColor(sf::Color::White);
                    survivalClock.restart();
                    enemySpawnClock.restart();
                    powerUpSpawnClock.restart();
                }
                else if (currentState == GameState::Gameplay) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                        currentState = GameState::GameOver;
                        finalTimeRecorded = survivalClock.getElapsedTime().asSeconds();
                    }
                    else if (keyPressed->scancode == sf::Keyboard::Scancode::P) {
                        currentState = GameState::Paused;
                    }
                }
                else if (currentState == GameState::Paused) {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::P) {
                        currentState = GameState::Gameplay;
                    }
                }
                else if (currentState == GameState::GameOver && keyPressed->scancode == sf::Keyboard::Scancode::Enter) {
                    currentState = GameState::MainMenu;
                }
            }

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (currentState == GameState::Gameplay && mousePressed->button == sf::Mouse::Button::Left) {
                    if (audioLoaded) shootSound.play();
                    sf::Vector2f playerPos = player.getPosition();
                    
                    float aimAngle = std::atan2(mousePos.y - playerPos.y, mousePos.x - playerPos.x);
                    float noseOffset = 35.f;
                    sf::Vector2f nosePos = playerPos + sf::Vector2f(std::cos(aimAngle), std::sin(aimAngle)) * noseOffset;
                    
                    auto spawnBullet = [&](float angleOffset) {
                        Bullet newBullet{ sf::Sprite(bulletTex), {0.f, 0.f} };
                        if (texturesLoaded) {
                            sf::Vector2u bSize = bulletTex.getSize();
                            newBullet.sprite.setOrigin({static_cast<float>(bSize.x) / 2.f, static_cast<float>(bSize.y) / 2.f});
                            newBullet.sprite.setScale({30.f / static_cast<float>(bSize.x), 30.f / static_cast<float>(bSize.y)});
                        }
                        newBullet.sprite.setPosition(nosePos);
                        
                        float finalAngle = aimAngle + angleOffset;
                        // CORRETTO: la rotazione del proiettile segue esattamente l'angolo di movimento
                        newBullet.sprite.setRotation(sf::radians(finalAngle));
                        newBullet.velocity = { std::cos(finalAngle) * bulletSpeed, std::sin(finalAngle) * bulletSpeed };
                        bullets.push_back(newBullet);
                    };

                    if (hasSpreadShot) {
                        spawnBullet(0.f); spawnBullet(-0.25f); spawnBullet(0.25f);
                    } else {
                        spawnBullet(0.f);
                    }
                }
            }
        }

        if (currentState != GameState::Paused) {
            for (auto& star : stars) {
                star.shape.move({0.f, star.speed});
                if (star.shape.getPosition().y > 600.f) {
                    star.shape.setPosition({disStarX(gen), 0.f});
                }
            }
        }

        if (currentState == GameState::Gameplay) {
            float timeElapsed = survivalClock.getElapsedTime().asSeconds();
            int timeBonus = static_cast<int>(timeElapsed * 5.0f);
            totalScore = killScore + timeBonus;

            if (isInvulnerable && invulnerabilityClock.getElapsedTime().asSeconds() > 2.0f) {
                isInvulnerable = false;
                player.setColor(sf::Color::White);
            }
            if (isInvulnerable) {
                if ((invulnerabilityClock.getElapsedTime().asMilliseconds() / 200) % 2 == 0) 
                    player.setColor(sf::Color(255, 255, 255, 100));
                else 
                    player.setColor(sf::Color::White);
            }

            statsText.setString("Vite: " + std::to_string(lives) + "\nScore: " + std::to_string(totalScore) + (hasShield ? " [SCUDO ATTIVO]" : ""));
            
            int minutes = static_cast<int>(timeElapsed) / 60;
            int seconds = static_cast<int>(timeElapsed) % 60;
            std::string timeStr = (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
            timerText.setString(timeStr);

            if (hasSpreadShot && spreadShotTimer.getElapsedTime().asSeconds() > 5.0f) hasSpreadShot = false;

            sf::Vector2f playerPos = player.getPosition();
            
            float playerAimAngle = std::atan2(mousePos.y - playerPos.y, mousePos.x - playerPos.x);
            player.setRotation(sf::radians(playerAimAngle + 1.5707963f));
            shieldVisual.setPosition(playerPos);

            sf::Vector2f movement(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) movement.x -= playerSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) movement.x += playerSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) movement.y -= playerSpeed;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) movement.y += playerSpeed;
            player.move(movement);

            sf::Vector2f pos = player.getPosition();
            if (pos.x < 35.f) pos.x = 35.f; if (pos.x > 765.f) pos.x = 765.f;
            if (pos.y < 35.f) pos.y = 35.f; if (pos.y > 565.f) pos.y = 565.f;
            player.setPosition(pos);

            float currentSpawnInterval = std::max(0.6f, 2.5f - (timeElapsed * 0.02f));

            if (enemySpawnClock.getElapsedTime().asSeconds() > currentSpawnInterval && enemies.size() < 25) {
                int typeRoll = disEnemyType(gen);
                sf::Sprite enemySprite(enemyTex);
                float eSpeed = baseEnemySpeed + (timeElapsed * 0.01f);
                int eHp = 1;
                bool eIsShooter = false;
                float targetSize = 60.f;

                if (typeRoll <= 15) { 
                    enemySprite = sf::Sprite(shooterTex);
                    eSpeed = baseEnemySpeed * 0.8f;
                    eHp = 2;
                    eIsShooter = true;
                    targetSize = 60.f;
                } 
                else if (typeRoll <= 35) { 
                    enemySprite = sf::Sprite(tankTex);
                    eSpeed = baseEnemySpeed * 0.5f;
                    eHp = 3;
                    eIsShooter = false;
                    targetSize = 85.f;
                } 
                else { 
                    enemySprite = sf::Sprite(enemyTex);
                    eSpeed = baseEnemySpeed + (timeElapsed * 0.01f);
                    eHp = 1;
                    eIsShooter = false;
                    targetSize = 60.f;
                }
                
                if (texturesLoaded) {
                    sf::Vector2u eSize = enemySprite.getTexture().getSize();
                    enemySprite.setOrigin({static_cast<float>(eSize.x) / 2.f, static_cast<float>(eSize.y) / 2.f});
                    enemySprite.setScale({targetSize / static_cast<float>(eSize.x), targetSize / static_cast<float>(eSize.y)});
                }

                int edge = disEdge(gen);
                float spawnX = 0, spawnY = 0;
                switch (edge) {
                    case 0: spawnX = disX(gen); spawnY = -50.f; break;
                    case 1: spawnX = 850.f; spawnY = disY(gen); break;
                    case 2: spawnX = disX(gen); spawnY = 650.f; break;
                    case 3: spawnX = -50.f; spawnY = disY(gen); break;
                }
                enemySprite.setPosition({spawnX, spawnY});
                
                Enemy newEnemy{ enemySprite, eSpeed, eHp, eIsShooter, sf::Clock() };
                newEnemy.shootClock.restart();
                enemies.push_back(newEnemy);
                enemySpawnClock.restart();
            }

            for (auto& enemy : enemies) {
                sf::Vector2f ePos = enemy.sprite.getPosition();
                sf::Vector2f dirToPlayer = player.getPosition() - ePos;
                float dist = std::sqrt(dirToPlayer.x * dirToPlayer.x + dirToPlayer.y * dirToPlayer.y);
                
                if (enemy.isShooter && dist < 250.f) {
                    if (enemy.shootClock.getElapsedTime().asSeconds() > 1.5f) {
                        Bullet eb{ sf::Sprite(bulletTex), {0.f, 0.f} };
                        if (texturesLoaded) {
                            sf::Vector2u bSize = bulletTex.getSize();
                            eb.sprite.setOrigin({static_cast<float>(bSize.x) / 2.f, static_cast<float>(bSize.y) / 2.f});
                            eb.sprite.setScale({22.f / static_cast<float>(bSize.x), 22.f / static_cast<float>(bSize.y)});
                        }
                        eb.sprite.setPosition(ePos);
                        float eAngle = std::atan2(dirToPlayer.y, dirToPlayer.x);
                        eb.sprite.setRotation(sf::radians(eAngle));
                        eb.velocity = { std::cos(eAngle) * enemyBulletSpeed, std::sin(eAngle) * enemyBulletSpeed };
                        enemyBullets.push_back(eb);
                        enemy.shootClock.restart();
                    }
                } else if (dist > 0) {
                    enemy.sprite.move({(dirToPlayer.x / dist) * enemy.speed, (dirToPlayer.y / dist) * enemy.speed});
                }
                enemy.sprite.setRotation(sf::radians(std::atan2(dirToPlayer.y, dirToPlayer.x) + 1.5707963f));
            }

            if (powerUpSpawnClock.getElapsedTime().asSeconds() > powerUpSpawnInterval) {
                int pType = disPowerUpType(gen);
                sf::Sprite pupSprite(pType == 1 ? pupSpreadTex : pupShieldTex);
                if (texturesLoaded) {
                    sf::Vector2u pSize = pupSprite.getTexture().getSize();
                    pupSprite.setOrigin({static_cast<float>(pSize.x) / 2.f, static_cast<float>(pSize.y) / 2.f});
                    pupSprite.setScale({40.f / static_cast<float>(pSize.x), 40.f / static_cast<float>(pSize.y)});
                }
                pupSprite.setPosition({disX(gen), disY(gen)});
                
                PowerUp pup{ pupSprite, pType, sf::Clock() };
                pup.lifespan.restart();
                powerUps.push_back(pup);
                powerUpSpawnClock.restart();
            }

            for (auto it = powerUps.begin(); it != powerUps.end(); ) {
                if (player.getGlobalBounds().findIntersection(it->sprite.getGlobalBounds())) {
                    if (it->type == 1) {
                        hasSpreadShot = true;
                        spreadShotTimer.restart();
                    } else {
                        hasShield = true;
                    }
                    it = powerUps.erase(it);
                } else if (it->lifespan.getElapsedTime().asSeconds() > 7.0f) {
                    it = powerUps.erase(it);
                } else ++it;
            }

            for (auto pIt = particles.begin(); pIt != particles.end(); ) {
                pIt->shape.move(pIt->velocity);
                if (pIt->lifeClock.getElapsedTime().asSeconds() > pIt->maxLife) {
                    pIt = particles.erase(pIt);
                } else {
                    ++pIt;
                }
            }

            auto handlePlayerDamage = [&]() {
                if (hasShield) {
                    hasShield = false;
                    triggerShake(0.3f);
                } else if (!isInvulnerable) {
                    lives -= 1;
                    isInvulnerable = true;
                    invulnerabilityClock.restart();
                    triggerShake(0.4f);
                    if (lives <= 0) {
                        currentState = GameState::GameOver;
                        finalTimeRecorded = survivalClock.getElapsedTime().asSeconds();
                    }
                }
            };

            for (auto it = enemyBullets.begin(); it != enemyBullets.end(); ) {
                it->sprite.move(it->velocity);
                bool hit = false;
                if (player.getGlobalBounds().findIntersection(it->sprite.getGlobalBounds())) {
                    handlePlayerDamage();
                    hit = true;
                }
                sf::Vector2f bPos = it->sprite.getPosition();
                if (hit || bPos.x < 0.f || bPos.x > 800.f || bPos.y < 0.f || bPos.y > 600.f) it = enemyBullets.erase(it);
                else ++it;
            }

            for (auto it = bullets.begin(); it != bullets.end(); ) {
                it->sprite.move(it->velocity);
                bool bulletDestroyed = false;

                for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
                    if (it->sprite.getGlobalBounds().findIntersection(eIt->sprite.getGlobalBounds())) {
                        eIt->hp -= 1;
                        bulletDestroyed = true;
                        
                        if (eIt->hp <= 0) {
                            spawnExplosion(eIt->sprite.getPosition(), eIt->isShooter ? sf::Color::Yellow : sf::Color::Red);
                            if (eIt->isShooter) killScore += 20;
                            else if (eIt->speed > 2.5f) killScore += 10;
                            else killScore += 30;
                            eIt = enemies.erase(eIt);
                        }
                        break;
                    } else ++eIt;
                }

                sf::Vector2f bPos = it->sprite.getPosition();
                if (bulletDestroyed || bPos.x < 0.f || bPos.x > 800.f || bPos.y < 0.f || bPos.y > 600.f) {
                    it = bullets.erase(it);
                } else ++it;
            }

            for (auto eIt = enemies.begin(); eIt != enemies.end(); ) {
                if (player.getGlobalBounds().findIntersection(eIt->sprite.getGlobalBounds())) {
                    spawnExplosion(eIt->sprite.getPosition(), sf::Color::Red);
                    handlePlayerDamage();
                    eIt = enemies.erase(eIt); 
                } else ++eIt;
            }
        }

        // --- RENDERING ---
        sf::View view = window.getDefaultView();
        if (shakeClock.getElapsedTime().asSeconds() < shakeDuration) {
            std::uniform_real_distribution<float> disShake(-shakeIntensity, shakeIntensity);
            view.setCenter({400.f + disShake(gen), 300.f + disShake(gen)});
        } else {
            view.setCenter({400.f, 300.f});
        }
        window.setView(view);

        window.clear(sf::Color::Black);
        
        switch (currentState) {
            case GameState::MainMenu:
                window.clear(sf::Color::Blue);
                break;

            case GameState::Gameplay:
            case GameState::Paused:
                for (const auto& star : stars) window.draw(star.shape);
                for (const auto& pup : powerUps) window.draw(pup.sprite);
                for (const auto& bullet : enemyBullets) window.draw(bullet.sprite);
                for (const auto& bullet : bullets) window.draw(bullet.sprite);
                for (const auto& enemy : enemies) window.draw(enemy.sprite);
                
                if (hasShield) window.draw(shieldVisual);
                window.draw(player);

                for (const auto& p : particles) window.draw(p.shape);

                window.draw(statsText);
                window.draw(timerText);

                if (currentState == GameState::Paused) {
                    centerMessage.setString("GIOCO IN PAUSA\nPremi 'P' per riprendere");
                    centerMessage.setPosition({250.f, 250.f});
                    window.draw(centerMessage);
                }
                break;
            
            case GameState::GameOver:
                window.clear(sf::Color::Red);
                centerMessage.setString("GAME OVER\nPunteggio Finale: " + std::to_string(totalScore) + 
                                        "\nTempo Sopravvissuto: " + std::to_string(static_cast<int>(finalTimeRecorded)) + "s" +
                                        "\n\nPremi INVIO per tornare al menu");
                centerMessage.setPosition({200.f, 220.f});
                window.draw(centerMessage);
                break;
        }
        
        window.display();
    }
    return 0;
}