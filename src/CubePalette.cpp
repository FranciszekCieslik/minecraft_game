#include "../include/CubePalette.hpp"

CubePalette::CubePalette()
{
    Cube grass("../assets/grass.jpg");
    Cube stone("../assets/stone.jpg");
    Cube coord("../assets/grass_debug.jpg");

    m_palette.insert(std::pair<Cube::Type, Cube>(Cube::Type::Stone, std::move(stone)));
    m_palette.insert(std::pair<Cube::Type, Cube>(Cube::Type::Grass, std::move(grass)));
    m_palette.insert(std::pair<Cube::Type, Cube>(Cube::Type::Coord, std::move(coord)));
}

const Cube &CubePalette::LookUp(Cube::Type type) const
{
    return m_palette.at(type);
}
