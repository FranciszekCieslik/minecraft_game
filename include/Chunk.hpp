#pragma once
#include "Cube.hpp"
#include "ShaderProgram.hpp"
#include "PerlinNoise.hpp"
#include "CubePalette.hpp"
#include "AABB.hpp"
#include "Ray.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

template <uint8_t Depth, uint8_t Width, uint8_t Height>
class Chunk
{
	struct CubeData
	{
		Cube::Type m_type{Cube::Type::None};
		bool m_isVisible{true};
	};

	using FlattenData_t = std::array<CubeData, Depth * Width * Height>;

public:
	Chunk(const glm::vec2 &origin, CubePalette &palette);
	inline void Generate(const PerlinNoise &rng, float worldX, float worldZ);
	void Draw(ShaderProgram &shader) const;

	struct HitRecord
	{
		glm::ivec3 m_cubeIndex;
		glm::ivec3 m_neighbourIndex;
	};

	Ray::HitType Hit(const Ray &ray, Ray::time_t min, Ray::time_t max, HitRecord &record) const;
	bool RemoveBlock(uint8_t width, uint8_t height, uint8_t depth);
	bool PlaceBlock(uint8_t width, uint8_t height, uint8_t depth, Cube::Type type);
	glm::vec2 getOrigin() { return m_origin; };

private:
	size_t CoordsToIndex(size_t depth, size_t width, size_t height) const;
	void UpdateVisibility();

	CubePalette &m_palette;
	FlattenData_t m_data;
	glm::vec2 m_origin;
	AABB m_aabb;
};

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline Chunk<Depth, Width, Height>::Chunk(const glm::vec2 &origin, CubePalette &palette) : m_origin(origin), m_palette(palette),
																						   m_aabb(glm::vec3(origin.x, 0, origin.y), glm::vec3(origin.x + Width, Height, origin.y + Depth))
{
	m_data.fill(CubeData{Cube::Type::None, true});
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline void Chunk<Depth, Width, Height>::Generate(const PerlinNoise &rng, float worldX, float worldZ)
{
	for (size_t x = 0; x < Width; ++x)
	{
		for (size_t z = 0; z < Depth; ++z)
		{
			float height = rng.At(glm::vec3(worldX + x, worldZ + z, 0) * 0.1f) * Height;
			size_t maxHeight = static_cast<size_t>(height);
			if (x > 0)
			{
				float leftHeight = rng.At(glm::vec3(worldX + x - 1, worldZ + z, 0) * 0.1f) * Height;
				maxHeight = std::min(maxHeight, static_cast<size_t>(leftHeight + 1));
			}
			if (z > 0)
			{
				float frontHeight = rng.At(glm::vec3(worldX + x, worldZ + z - 1, 0) * 0.1f) * Height;
				maxHeight = std::min(maxHeight, static_cast<size_t>(frontHeight + 1));
			}

			for (size_t y = 0; y < Height; ++y)
			{
				auto &cube = m_data[CoordsToIndex(z, x, y)];
				if (y <= maxHeight)
				{
					cube.m_type = (y == maxHeight) ? Cube::Type::Grass : Cube::Type::Stone;
					cube.m_isVisible = true;
				}
				else
				{
					cube.m_type = Cube::Type::None;
					cube.m_isVisible = false;
				}
			}
		}
	}
	UpdateVisibility();
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline void Chunk<Depth, Width, Height>::Draw(ShaderProgram &shader) const
{
	for (size_t x = 0; x < Width; ++x)
	{
		for (size_t z = 0; z < Depth; ++z)
		{
			for (size_t y = 0; y < Height; ++y)
			{
				const auto &cube = m_data[CoordsToIndex(z, x, y)];
				if (!cube.m_isVisible)
					continue;

				// Pobierz typ bloku i narysuj go
				const Cube &cubeType = m_palette.LookUp(cube.m_type);
				glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_origin.x + x, y, m_origin.y + z));
				shader.setUniform("model", model);
				cubeType.draw();
			}
		}
	}
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline size_t Chunk<Depth, Width, Height>::CoordsToIndex(size_t depth, size_t width, size_t height) const
{
	return height * static_cast<size_t>(Depth) * static_cast<size_t>(Width) + width * static_cast<size_t>(Depth) + depth;
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline void Chunk<Depth, Width, Height>::UpdateVisibility()
{
	for (size_t x = 0; x < Width; ++x)
	{
		for (size_t z = 0; z < Depth; ++z)
		{
			for (size_t y = 0; y < Height; ++y)
			{
				auto &cube = m_data[CoordsToIndex(z, x, y)];
				if (cube.m_type == Cube::Type::None)
				{
					cube.m_isVisible = false;
					continue;
				}

				// Sprawdź sąsiednie bloki
				bool hasVisibleNeighbor = false;
				for (int dx = -1; dx <= 1 && !hasVisibleNeighbor; ++dx)
				{
					for (int dy = -1; dy <= 1 && !hasVisibleNeighbor; ++dy)
					{
						for (int dz = -1; dz <= 1 && !hasVisibleNeighbor; ++dz)
						{
							if (dx == 0 && dy == 0 && dz == 0)
								continue;

							int nx = x + dx, ny = y + dy, nz = z + dz;
							if (nx >= 0 && nx < Width && ny >= 0 && ny < Height && nz >= 0 && nz < Depth)
							{
								auto &neighbor = m_data[CoordsToIndex(nz, nx, ny)];
								if (neighbor.m_type == Cube::Type::None)
								{
									hasVisibleNeighbor = true;
								}
							}
						}
					}
				}
				cube.m_isVisible = hasVisibleNeighbor;
			}
		}
	}
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline Ray::HitType Chunk<Depth, Width, Height>::Hit(const Ray &ray, Ray::time_t min, Ray::time_t max, HitRecord &record) const
{
	AABB::HitRecord chunkRecord;
	if (m_aabb.Hit(ray, min, max, chunkRecord) == Ray::HitType::Miss)
	{
		return Ray::HitType::Miss;
	}

	float closestDistance = std::numeric_limits<float>::max();
	Ray::HitType hitType = Ray::HitType::Miss;

	glm::vec3 chunkOrigin = glm::vec3(m_origin.x, 0.0f, m_origin.y); // Przesunięcie chunku w świecie

	for (uint8_t x = 0; x < Width; ++x)
	{
		for (uint8_t y = 0; y < Height; ++y)
		{
			for (uint8_t z = 0; z < Depth; ++z)
			{
				auto &cube = m_data[CoordsToIndex(z, x, y)];

				if (cube.m_type == Cube::Type::None)
					continue;

				// Uwzględniamy pozycję chunku w świecie
				glm::vec3 cubeMin = chunkOrigin + glm::vec3(x, y, z);
				glm::vec3 cubeMax = cubeMin + glm::vec3(1.0f);
				AABB cubeAABB(cubeMin, cubeMax);

				// Test ray against the cube
				AABB::HitRecord cubeRecord;
				if (cubeAABB.Hit(ray, min, max, cubeRecord) == Ray::HitType::Hit)
				{
					float distance = glm::distance(ray.Origin(), cubeRecord.m_point);
					if (distance < closestDistance)
					{
						closestDistance = distance;
						record.m_cubeIndex = glm::ivec3(glm::floor(cubeRecord.m_point) - chunkOrigin);
						hitType = Ray::HitType::Hit;
					}
				}
			}
		}
	}

	if (hitType == Ray::HitType::Hit)
	{
		std::cout << "Hit! Block at: " << record.m_cubeIndex.x << ", " << record.m_cubeIndex.y << ", " << record.m_cubeIndex.z << std::endl;
	}

	return hitType;
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline bool Chunk<Depth, Width, Height>::RemoveBlock(uint8_t x, uint8_t y, uint8_t z)
{
	if (x >= Width || y >= Height || z >= Depth)
		return false; // Out of bounds

	CubeData &cube = m_data[z * Width * Height + y * Width + x];
	if (cube.m_type == Cube::Type::None)
		return false; // No block to remove

	cube.m_type = Cube::Type::None;
	cube.m_isVisible = false;
	UpdateVisibility(); // Zaktualizuj widoczność bloków

	return true;
}

template <uint8_t Depth, uint8_t Width, uint8_t Height>
inline bool Chunk<Depth, Width, Height>::PlaceBlock(uint8_t x, uint8_t y, uint8_t z, Cube::Type type)
{
	if (x >= Width || y >= Height || z >= Depth)
		return false; // Out of bounds

	CubeData &cube = m_data[z * Width * Height + y * Width + x];
	if (cube.m_type != Cube::Type::None)
		return false; // Block already exists

	cube.m_type = type;
	cube.m_isVisible = true;
	UpdateVisibility(); // Zaktualizuj widoczność bloków

	return true;
}
