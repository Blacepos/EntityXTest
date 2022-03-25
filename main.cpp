
#define _USE_MATH_DEFINES
#include <SFML/Graphics.hpp>
#include <entityx/entityx.h>
#include <random>
#include <memory>
#include <cmath>
namespace ex = entityx;
using vec2 = sf::Vector2f;



namespace Components {
    struct Body {
        Body(vec2 const& pos, vec2 const& vel) : position(pos), velocity(vel) {};
        vec2 velocity;
        vec2 position;
    };

    // Pointer to a shape since entity data isn't stored there anyways (I don't really know that's just my guess. This is just what the example did)
    using Renderable = std::shared_ptr<sf::Shape>;
}

namespace Systems {
    class MovementSystem : public ex::System<MovementSystem> {
    public:
        void update(ex::EntityManager& entities, ex::EventManager& events, ex::TimeDelta dt) override {
            entities.each<Components::Body>(
                [dt](ex::Entity entity, Components::Body& body) {
                    body.position.x += body.velocity.x * dt;
                    body.position.y += body.velocity.y * dt;
                });
        }
    };

    class RenderSystem : public ex::System<RenderSystem> {
    private:
        sf::RenderTarget& _target;
    public:
        RenderSystem(sf::RenderTarget& target) : _target(target) {}

        void update(ex::EntityManager& entities, ex::EventManager& events, ex::TimeDelta dt) override {
            entities.each<Components::Body, Components::Renderable>(
                [this](ex::Entity entity, Components::Body& body, Components::Renderable& renderable) {
                    // Move the sf::Shape to the body's position and draw
                    renderable->setPosition(body.position);
                    _target.draw(*renderable);
                });
        }
    };
}

class Game : public ex::EntityX {
private:
    std::mt19937_64 _rand;

public:
    Game(sf::RenderTarget& target) {
        // Add systems
        systems.add<Systems::MovementSystem>();
        systems.add<Systems::RenderSystem>(target);

        // Create Random Generator and distributions
        std::random_device rd;
        _rand.seed(rd());
        std::uniform_real_distribution<double> rand_vel(25, 50);
        std::uniform_real_distribution<double> rand_angle(0, 2*M_PI);

        // Create entities
        Components::Renderable shape(new sf::CircleShape(5.f));

        for (int i = 0; i < 1000; ++i) {
            auto e = entities.create();
            double r = rand_vel(_rand);
            double t = rand_angle(_rand);
            e.assign<Components::Body>( vec2(400.0, 300.0), vec2(r * cos(t), r * sin(t)) );

            e.assign<Components::Renderable>(shape);
        }
    }

    void update(ex::TimeDelta dt) {
        systems.update<Systems::MovementSystem>(dt);
    }

    void draw(ex::TimeDelta dt) {
        systems.update<Systems::RenderSystem>(dt);
    }
};

class GameApp {
private:
    sf::RenderWindow _window;
    Game _game;
    sf::Clock _time;
public:
    GameApp() : _window(sf::VideoMode(800, 600), "EntityX Test"), _game(_window) {
        _window.setVerticalSyncEnabled(true);
    }

    void run() {
        float dt;

        while (_window.isOpen()) {
            sf::Event event;
            while (_window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    _window.close();
            }

            dt = _time.restart().asSeconds();
            _game.update(dt);

            _window.clear();
            _game.draw(dt);
            _window.display();
        }
    }
};

int main()
{
    GameApp gameapp;

    gameapp.run();

    return 0;
}
