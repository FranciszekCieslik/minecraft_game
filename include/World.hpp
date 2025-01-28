#pragma once

#include <vector>
#include "Chunk.hpp"

template <size_t chunkSize, size_t worldSize>
class World
{
public:
    World()
    {
        int x{0}, y{0};
        for (int i{0}; i < worldSize * worldSize; i++)
        {
            // index = y * width + x;
            y = i / worldSize; // Wiersz
            x = i % worldSize; // Kolumna
            m_chunks.push_back(Chunk<chunkSize, chunkSize, chunkSize>(glm::vec2(x * chunkSize, y * chunkSize), palette));
            m_chunks.back().Generate(perlin);
        }
        m_chunk = &m_chunks.front();
    };

    void Draw(ShaderProgram &shader)
    {
        if (visible_chunks.empty())
            return;

        for (auto chunk : visible_chunks)
        {
            chunk->Draw(shader);
        }
    };

    void getChunk(glm::vec3 &cameraPosition)
    {
        int x = std::floor(cameraPosition.x / chunkSize);
        int y = std::floor(cameraPosition.z / chunkSize);
        int index = y * worldSize + x;
        if (index >= 0 || index < worldSize * worldSize)
        {
            m_chunk = &m_chunks[index];
        }
    };

    void updateVisibleChunks(glm::vec3 &cameraPosition)
    {
        getChunk(cameraPosition);
        if (!m_chunk)
            return;
        auto chunks = getNeighbors(m_chunk->getOrigin());
        chunks.push_back(m_chunk);
        visible_chunks.assign(chunks.begin(), chunks.end());
    };

public:
    Chunk<chunkSize, chunkSize, chunkSize> *m_chunk;

private:
    CubePalette palette;
    PerlinNoise perlin;
    std::vector<Chunk<chunkSize, chunkSize, chunkSize>> m_chunks;
    std::vector<Chunk<chunkSize, chunkSize, chunkSize> *> visible_chunks;

    // Chunk<chunkSize, chunkSize, chunkSize> chunk(glm::vec2(0, 0), palette);
    std::vector<Chunk<chunkSize, chunkSize, chunkSize> *> getNeighbors(glm::vec2 pos)
    {
        size_t nx;
        size_t ny;
        std::vector<Chunk<chunkSize, chunkSize, chunkSize> *> neighbors;
        std::vector<std::pair<int, int>> directions = {
            {-1, -1}, {0, -1}, {1, -1}, // górny rząd
            {-1, 0},
            {1, 0}, // środkowy rząd (bez środka)
            {-1, 1},
            {0, 1},
            {1, 1} // dolny rząd
        };

        for (auto [dx, dy] : directions)
        {
            nx = (pos.x / chunkSize) + dx;
            ny = (pos.y / chunkSize) + dy;

            if (nx >= 0 && nx < worldSize && ny >= 0 && ny < worldSize)
            {
                size_t index = ny * worldSize + nx;
                neighbors.push_back(&m_chunks[index]);
            }
        }

        return neighbors;
    }
};
