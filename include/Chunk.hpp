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
	void Generate(const PerlinNoise &rng);
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
inline void Chunk<Depth, Width, Height>::Generate(const PerlinNoise &rng)
{
	for (size_t x = 0; x < Width; ++x)
	{
		for (size_t z = 0; z < Depth; ++z)
		{
			// Generuj wysokość w oparciu o Perlin Noise
			float height = rng.At(glm::vec3(x, z, 0) * 0.1f) * Height; // Skalowanie 0.1f dla gładkiego szumu
			size_t maxHeight = static_cast<size_t>(height);

			// Wypełnij kolumnę blokami
			for (size_t y = 0; y < Height; ++y)
			{
				auto &cube = m_data[CoordsToIndex(z, x, y)];
				if (y <= maxHeight)
				{
					// Ustal typ bloku w zależności od wysokości
					if (y == maxHeight)
					{
						cube.m_type = Cube::Type::Grass;
					}
					else // if (y < maxHeight - 3)
					{
						cube.m_type = Cube::Type::Stone;
					}
					// else
					// {
					// 	cube.m_type = Cube::Type::Dirt;
					// }
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
	UpdateVisibility(); // Zaktualizuj widoczność bloków
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
		return Ray::HitType::Miss; // Early exit if the ray misses the chunk
	}

	//	calculate chunk shift
	//	for x,y,z in Width, Height, Depth
	// Iterate over all blocks in the chunk

	float closestDistance = std::numeric_limits<float>::max(); // Przechowuje najbliższy dystans

	Ray::HitType hitType = Ray::HitType::Miss;
	for (uint8_t x = 0; x < Width; ++x)
	{
		for (uint8_t y = 0; y < Height; ++y)
		{
			for (uint8_t z = 0; z < Depth; ++z)
			{
				const CubeData &cube = m_data[z * Width * Height + y * Width + x];
				if (cube.m_type == Cube::Type::None)
					continue;

				// Define the cube's AABB
				glm::vec3 cubeMin = m_aabb.Min() + glm::vec3(x, y, z);
				glm::vec3 cubeMax = cubeMin + glm::vec3(1.0f);
				AABB cubeAABB(cubeMin, cubeMax);

				// Test ray against the cube
				AABB::HitRecord cubeRecord;
				if (cubeAABB.Hit(ray, min, max, cubeRecord) == Ray::HitType::Hit)
				{
					// record.m_cubeIndex = glm::ivec3(glm::floor(cubeRecord.m_point));

					// max = cubeRecord.m_time; // Update max to find the nearest hit
					// hitType = Ray::HitType::Hit;

					// // Early exit if only the closest hit is required
					// if (max == min)
					// 	return hitType;

					float distance = glm::distance(ray.Origin(), cubeRecord.m_point);
					if (distance < closestDistance)
					{
						closestDistance = distance;
						record.m_cubeIndex = glm::ivec3(glm::floor(cubeRecord.m_point));
						hitType = Ray::HitType::Hit;
					}
				}
			}
		}
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
