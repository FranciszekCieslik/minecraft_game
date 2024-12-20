#pragma once
#include <array>
#include <string>
#include <GL/glew.h>

class Cube
{
public:
  enum class Type
  {
    None,
    Grass,
    Stone,
    Coord
  };

  Cube(const std::string &texturePath);

  Cube() = delete;
  Cube(const Cube &) = delete;
  Cube &operator=(const Cube &) = delete;
  Cube(Cube &&) noexcept;
  Cube &operator=(Cube &&) noexcept;
  ~Cube();

  GLuint Vbo() const { return m_vbo; }
  GLuint Vao() const { return m_vao; }
  void draw() const;
  GLuint Texture() const { return m_texture; }

private:
  GLuint m_vbo{0};
  GLuint m_vao{0};
  GLuint m_ebo{0};
  GLuint m_texture{0};

  static std::array<float, 6 * 6 * 5> s_vertices;

  GLuint createTexture(const std::string &texturePath);
};
