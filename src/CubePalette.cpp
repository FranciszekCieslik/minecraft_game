#include "../include/CubePalette.hpp"

CubePalette::CubePalette()
{
    Cube grass("../assets/grass.jpg");
    Cube stone("../assets/stone.jpg");
    m_palette.insert(std::pair<Cube::Type, Cube>(Cube::Type::Stone, std::move(stone)));
    m_palette.insert(std::pair<Cube::Type, Cube>(Cube::Type::Grass, std::move(grass)));
}

const Cube &CubePalette::LookUp(Cube::Type type) const
{
    return m_palette.at(type);
}
