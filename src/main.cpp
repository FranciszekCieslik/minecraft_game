#include "../include/Camera.hpp"
#include "../include/Chunk.hpp"
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
  shaders.setUniform("projection", camera.Projection());

  // Cube cube("../assets/grass_debug.jpg");
  // Inicjalizowanie chunku
  // Inicjalizowanie palety Cube (bloków)
  CubePalette palette;

  // Inicjalizowanie generatora PerlinNoise
  PerlinNoise perlin;

  // Tworzenie chunków w okolicy gracza
  const size_t chunkSize = 16; // przykładowy rozmiar chunków
  Chunk<chunkSize, chunkSize, chunkSize> chunk(glm::vec2(0, 0), palette);
  chunk.Generate(perlin);

  sf::Vector2i windowCenter(window.getSize().x / 2, window.getSize().y / 2);
  sf::Vector2i mousePosition = sf::Mouse::getPosition();
  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // RayTracing dla niszczenia i tworzenia bloków
  Ray::HitType hitType;
  Chunk<chunkSize, chunkSize, chunkSize>::HitRecord hitRecord;

  // Clock start
  sf::Clock clock;

  //                 /z   / -x /pion(y)
  chunk.PlaceBlock(8.0f, 8.0f, 8.0f, Cube::Type::Coord);
  chunk.PlaceBlock(10.0f, 8.0f, 8.0f, Cube::Type::Stone);

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
      } // add and remove blocks
      else if (event.type == sf::Event::MouseButtonPressed)
      {
        // hit
        hitType = chunk.Hit(Ray(camera.m_position, camera.m_front), -6.0f, 6.0f, hitRecord);
        if (hitType == Ray::HitType::Hit)
        {
          if (event.mouseButton.button == sf::Mouse::Left)
          {
            chunk.RemoveBlock(hitRecord.m_cubeIndex.z, hitRecord.m_cubeIndex.x, hitRecord.m_cubeIndex.y);
          }
          else if (event.mouseButton.button == sf::Mouse::Right)
          {
          
            glm::vec3 hitCubeCenter = glm::vec3(hitRecord.m_cubeIndex) + glm::vec3(0.5f);
            glm::vec3 direction = glm::normalize(hitCubeCenter - camera.m_position);
            glm::ivec3 neighborOffset(
                (direction.x > 0.5f) ? 1 : ((direction.x < -0.5f) ? -1 : 0),
                (direction.y > 0.5f) ? 1 : ((direction.y < -0.5f) ? -1 : 0),
                (direction.z > 0.5f) ? 1 : ((direction.z < -0.5f) ? -1 : 0));

            hitRecord.m_neighbourIndex = hitRecord.m_cubeIndex - neighborOffset;

            chunk.PlaceBlock(hitRecord.m_neighbourIndex.z, hitRecord.m_neighbourIndex.x, hitRecord.m_neighbourIndex.y, Cube::Type::Stone);
          }
        }
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

    // cube.draw();
    chunk.Draw(shaders);

    window.display();
  }

  return 0;
}
//g++ -o main main.cpp Camera.cpp Cube.cpp ShaderProgram.cpp PerlinNoise.cpp CubePalette.cpp AABB.cpp Ray.cpp -lGLEW -lGL -lsfml-window -lsfml-graphics -lsfml-system -I/usr/include/glm -std=c++20