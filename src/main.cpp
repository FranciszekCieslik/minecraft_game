#include "Camera.cpp"
#include "Cube.cpp"
#include "ShaderProgram.cpp"
#include <SFML/Window.hpp>
#include <SFML/Window/Context.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <utility>
#include <GL/glew.h>

int main()
{
  sf::ContextSettings contextSettings;
  contextSettings.depthBits = 24;
  contextSettings.stencilBits = 8;
  contextSettings.majorVersion = 3;
  contextSettings.minorVersion = 3;

  sf::Window window(sf::VideoMode(800, 600), "Maincraft", sf::Style::Default,
                    contextSettings);
  window.setActive(true);
  window.setMouseCursorGrabbed(true);
  window.setMouseCursorVisible(false);

  // Initialize GLEW
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  glViewport(0, 0, static_cast<GLsizei>(window.getSize().x), static_cast<GLsizei>(window.getSize().y));

  Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), -90.0f, 0.0f);

  ShaderProgram shaders;
  GLuint programId = shaders.getProgramId();
  if (programId == 0)
  {
    std::cerr << "Failed to create shader program" << std::endl;
    return -1;
  }

  shaders.use();

  Cube cube("../assets/grass_debug.jpg");

  shaders.setUniform("projection", camera.Projection());

  sf::Clock clock;
  sf::Vector2i windowCenter(window.getSize().x / 2, window.getSize().y / 2);
  sf::Vector2i mousePosition = sf::Mouse::getPosition();

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  while (window.isOpen())
  {
    const float dt = clock.restart().asSeconds();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed)
      {
        window.close();
      }
      else if (event.type == sf::Event::Resized)
      {
        glViewport(0, 0, event.size.width, event.size.height);
      }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
      camera.MoveForward(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
      camera.MoveBackward(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
      camera.MoveLeft(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
      camera.MoveRight(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
    {
      camera.MoveUp(dt);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
    {
      camera.MoveDown(dt);
    }

    const sf::Vector2i newMousePosition = sf::Mouse::getPosition();
    camera.Rotate(newMousePosition - mousePosition);
    mousePosition = newMousePosition;

    shaders.setUniform("view", camera.View());
    shaders.setUniform("projection", camera.Projection());

    cube.draw();

    window.display();
  }

  return 0;
}
