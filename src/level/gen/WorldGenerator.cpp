﻿#include "WorldGenerator.h"

namespace WGenerator
{
	WorldGenerator::WorldGenerator(uint t_width, uint t_height, uint t_seed, uint t_minSize, uint t_maxSize)
		: m_width(t_width)
		, m_height(t_height)
		, m_minSize(t_minSize)
		, m_maxSize(t_maxSize)
		, m_seed(t_seed)
	{
		m_generator.setSeed(m_seed);
	}

	void WorldGenerator::generateMap()
	{
		std::shared_ptr<Leaf> root = std::make_shared<Leaf>(IntRectangle(0, 0, m_width, m_height), std::make_shared<Random::Generator<>>(m_generator),
			m_minSize);
		m_leafs.push_back(root);
		bool didSplit = true;
		while (didSplit)
		{
			didSplit = false;
			for (uint i = 0; i < m_leafs.size(); i++)
			{
				if (!m_leafs[i]->leftChild && !m_leafs[i]->rightChild)
				{
					int randomizedValue = m_generator.intInRange(0, 100);
					if (m_leafs[i]->block.width > m_maxSize || m_leafs[i]->block.height > m_maxSize ||
						randomizedValue > 25)
					{
						if (m_leafs[i]->split())
						{
							m_leafs.push_back(m_leafs[i]->leftChild);
							m_leafs.push_back(m_leafs[i]->rightChild);
							didSplit = true;
						}
					}
				}
			}
		}
		root->createRooms();
	}

	std::vector<std::shared_ptr<IntRectangle>> WorldGenerator::getRooms()
	{
		std::vector<std::shared_ptr<IntRectangle>> Rects;
		for (uint i = 0; i < m_leafs.size(); i++)
			if (m_leafs[i]->room)
				Rects.push_back(m_leafs[i]->room);
		return Rects;
	}

	std::vector<std::shared_ptr<IntRectangle>>  WorldGenerator::getRandomSquares()
	{
		std::vector<std::shared_ptr<IntRectangle> > rooms;
		for (uint i = 0; i < m_leafs.size(); i++)
			if (m_leafs[i]->room)
				rooms.push_back(m_leafs[i]->room);
		
		std::vector<std::shared_ptr<IntRectangle>> Rects;
		for (uint i = 0; i < rooms.size(); i++)
		{
			for (int j = 0; j < 10; j++)
			{
				std::shared_ptr<IntRectangle>  rect = std::make_shared<IntRectangle>(IntRectangle());
				rect->width = static_cast<int32>(m_generator.uint64InRange(2, 5));
				rect->height = static_cast<int32>(m_generator.uint64InRange(2, 5));
				rect->x = static_cast<int32>(m_generator.uint64InRange(rooms[i]->x,
				                                                       rooms[i]->x + rooms[i]->width - static_cast<int32>(rect->width / 2)));
				rect->y = static_cast<int32>(m_generator.uint64InRange(rooms[i]->y,
				                                                       rooms[i]->y + rooms[i]->height - static_cast<int32>(rect->height / 2)));
				Rects.push_back(rect);
			}
		}
		return Rects;
	}

	std::vector<std::shared_ptr<IntRectangle>> WorldGenerator::getHalls()
	{
		std::vector<std::shared_ptr<IntRectangle>> Rects;
		for (uint i = 0; i < m_leafs.size(); i++)
			for (uint j = 0; j < m_leafs[i]->halls.size(); j++)
				Rects.push_back(std::make_shared<IntRectangle>(m_leafs[i]->halls[j]));
	
		return Rects;
	}

	std::vector<std::vector<byte>> WorldGenerator::render(std::vector<std::pair<std::vector<std::shared_ptr<IntRectangle>>, byte>> data) const
	{
		std::vector<std::vector<byte>> map(m_width, std::vector<byte>(m_height, 0));
		for (uint i = 0; i < data.size(); i++)
			for (uint j = 0; j < data[i].first.size(); j++)
				for (int32 k = 0; k < data[i].first[j]->width; k++)
					for (int32 l = 0; l < data[i].first[j]->height; l++)
						if (map.size() > static_cast<uint>(data[i].first[j]->x + k))
							if (map[data[i].first[j]->x + k].size() > static_cast<uint>(data[i].first[j]->y + l))
								map[k + data[i].first[j]->x][l + data[i].first[j]->y] = data[i].second;
		
		return map;
	}

	Map WorldGenerator::getMap()
	{
		Map map;
		std::vector<std::pair<std::vector<std::shared_ptr<IntRectangle>>, byte>> data;
		data.push_back(std::make_pair<std::vector<std::shared_ptr<IntRectangle>>, byte>(getRooms(), 1));
		data.push_back(std::make_pair<std::vector<std::shared_ptr<IntRectangle>>, byte>(getRandomSquares(), 1));
		data.push_back(std::make_pair<std::vector<std::shared_ptr<IntRectangle>>, byte>(getHalls(), 1));
		map.tiles = render(data);
		uint numberOfRooms = getRooms().size();
		map.playerPosition = placePlayer(static_cast<uint>(m_generator.uint64InRange(0, numberOfRooms)));
		return map;
	}

	sf::Vector2<uint> WorldGenerator::placePlayer(uint roomId)
	{
		std::vector<std::shared_ptr<IntRectangle>> rooms = getRooms();
		return sf::Vector2<uint>((rooms[roomId]->x * 2 + rooms[roomId]->width) / 2, (rooms[roomId]->y * 2 + rooms[roomId]->height) / 2);
	}
}
