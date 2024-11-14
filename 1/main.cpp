#include <SFML/Graphics.hpp>
#include "shaders.cpp"

int main()
{
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 24;
    contextSettings.stencilBits = 8;
    contextSettings.majorVersion = 3;
    contextSettings.minorVersion = 3;

    // Create SFML window
    sf::Window window(sf::VideoMode(800, 600), "OpenGL with SFML", sf::Style::Default, contextSettings);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Load and compile shaders
    std::string vertexShaderSource = LoadShaderSource("vertexshader.glsl");
    std::string fragmentShaderSource = LoadShaderSource("fragmentshader.glsl");
    GLuint vertexShaderId = CreateShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CreateShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    // Link shader program
    GLuint programId = CreateProgram(vertexShaderId, fragmentShaderId);

    // Create vertex buffer and vertex array objects
    auto [vbo, vao] = CreateVertexBufferObject();

    // Main rendering loop
    while (window.isOpen())
    {
        // Event handling
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear screen and set background color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render
        glUseProgram(programId);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Display
        window.display();
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(programId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return 0;
}
