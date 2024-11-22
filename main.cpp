#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common/shader.hpp"


// Globals (initialized in `main`)
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint programID;

// Triangle data
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
};

void initOpenGL()
{
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Generate and bind Vertex Array
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Generate and bind Vertex Buffer
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // Load shaders
    programID = LoadShaders("shaders/vertexshader", "shaders/fragmentshader");

    // Set clear color (background color)
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
}

void render()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use shader program
    glUseProgram(programID);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,        // Attribute 0. Must match the layout in the shader
        3,        // Size
        GL_FLOAT, // Type
        GL_FALSE, // Normalized?
        0,        // Stride
        (void *)0 // Array buffer offset
    );

    // Draw the triangle!
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices -> 1 triangle

    glDisableVertexAttribArray(0);
}

int main()
{
    // Create SFML window and OpenGL context
    sf::Window window(sf::VideoMode(800, 600), "OpenGL Tutorial", sf::Style::Default, sf::ContextSettings(24));

    // Initialize OpenGL
    initOpenGL();

    // Main loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Render the scene
        render();

        // Swap buffers
        window.display();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteProgram(programID);

    return 0;
}