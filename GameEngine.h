#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "Buffer.h"
#include "linalg.h"
#include "PlayerCamera.h"

#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>

class Block: public Buffer {
public:

    static constexpr uint8_t Air = 0x0;
    static constexpr uint8_t Dirt = 0x1;
    static constexpr uint8_t Stone = 0x2;
    static constexpr uint8_t Grass = 0x3;

    std::pair<int, int> getTexture(uint8_t block, uint8_t side) {
        switch (block) {
        case Grass:
            if (side == Block::Top) return {1, 0};
            else if (side == Block::Bottom) return {3, 0};
            else return {2, 0};
        case Dirt:
            return {3, 0};
        case Stone:
            return {0, 0};
        default:
            return {12, 1};
        }
    }
    static constexpr uint8_t Front  = 0x1 << 0;
    static constexpr uint8_t Back   = 0x1 << 1;
    static constexpr uint8_t Left   = 0x1 << 2;
    static constexpr uint8_t Right  = 0x1 << 3; 
    static constexpr uint8_t Top    = 0x1 << 4; 
    static constexpr uint8_t Bottom = 0x1 << 5;

    using FaceSet = uint8_t;

    Block(uint8_t block, FaceSet faceSet) {
       
       float N = 16.0f;

        if (faceSet & Top) {
            auto [i, j] = getTexture(block, Top);
            Vertex a = {{-0.5, 0.5, 0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{-0.5, -0.5, 0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{0.5, -0.5, 0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}};
            addQuad(d, c, b, a);

        }

        if (faceSet & Bottom) {
            auto [i, j] = getTexture(block, Bottom);
            Vertex a = {{-0.5, 0.5, -0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{0.5, -0.5, -0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{0.5, 0.5, -0.5}, {i / N, j / N}};
            addQuad(a, b, c, d);

        }

        if (faceSet & Left) {
            auto [i, j] = getTexture(block, Left);
            Vertex a = {{-0.5, 0.5, -0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{-0.5, -0.5, 0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{-0.5, 0.5, 0.5}, {i / N, j / N}};
            addQuad(d, c, b, a);
        }

        if (faceSet & Right) {
            auto [i, j] = getTexture(block, Right);
            Vertex a = {{0.5, 0.5, -0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{0.5, -0.5, 0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}};
            addQuad(a, b, c, d);
        }

        if (faceSet & Back) {
            auto [i, j] = getTexture(block, Back);
            Vertex a = {{0.5, 0.5, -0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{-0.5, 0.5, -0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{-0.5, 0.5, 0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}};
            addQuad(d, c, b, a);
        }

        if (faceSet & Front) {
            auto [i, j] = getTexture(block, Front);
            Vertex a = {{0.5, -0.5, -0.5}, {i / N, (j + 1) / N}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}};
            Vertex c = {{-0.5, -0.5, 0.5}, {(i + 1) / N, j / N}};
            Vertex d = {{0.5, -0.5, 0.5}, {i / N, j / N}};
            addQuad(a, b, c, d);
        }
    }
};

class Biome {
public:
    static constexpr int SEA_LEVEL = 62;
};

class Chunk {
public:
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 256;
    using Location = std::pair<int, int>;

private:
    uint8_t blocks[WIDTH][WIDTH][HEIGHT] = {};
    Location location;
    NativeBuffer buffer_;
    bool modifed_ = true;
    
public:

    Chunk(Location loc) {
        location = loc;
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < WIDTH; y++) {
                const int global_x = x + location.first * WIDTH;
                const int global_y = y + location.second * WIDTH;
                const int height = (int) 5 * (sin(0.08 * global_x) + sin(0.08 * global_y)) + Biome::SEA_LEVEL;
                for (int z = 0; z <= height; z++) {
                    if (z <= height - 4) {
                        blocks[x][y][z] = Block::Stone;
                    } else if (z < height) {
                        blocks[x][y][z] = Block::Dirt;
                    } else if (z == height) {
                        blocks[x][y][z] = Block::Grass;
                    }
                }
            }
        }
    }
    
    Location getLocation() const {
        return location;
    }

    bool isModified() const {
        return modifed_;
    }

    const NativeBuffer& getBuffer() const {
        return buffer_;
    }

    const NativeBuffer& computeBuffer(NativeDevice device, std::array<Chunk*, 4> &neighbors) {
        if (modifed_) {
            Buffer buffer = Buffer();
            
            Chunk *left = neighbors[0];
            Chunk *right = neighbors[1];
            Chunk *front = neighbors[2];
            Chunk *back = neighbors[3];

            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < WIDTH; y++) {
                    for (int z = 0; z < HEIGHT; z++) {
                        if (blocks[x][y][z] == Block::Air) continue;
                        uint8_t visible_sides = 0;
                        
    
                        if (x != WIDTH - 1 && blocks[x + 1][y][z] == Block::Air) visible_sides |= Block::Right;
                        if (x != 0 && blocks[x - 1][y][z] == Block::Air) visible_sides |= Block::Left;
                        if (y != WIDTH - 1 && blocks[x][y + 1][z] == Block::Air) visible_sides |= Block::Back;
                        if (y != 0 && blocks[x][y - 1][z] == Block::Air) visible_sides |= Block::Front;
                        if (z != HEIGHT - 1 && blocks[x][y][z + 1] == Block::Air) visible_sides |= Block::Top;
                        if (z != 0 && blocks[x][y][z - 1] == Block::Air) visible_sides |= Block::Bottom;
                        
                        // Edge Conditions
                        if (x == WIDTH - 1 && right && right->blocks[0][y][z] == Block::Air) visible_sides |= Block::Right;
                        if (x == 0 && left && left->blocks[WIDTH - 1][y][z] == Block::Air) visible_sides |= Block::Left;
                        if (y == WIDTH - 1 && back && back->blocks[x][0][z] == Block::Air) visible_sides |= Block::Back;
                        if (y == 0 && front && front->blocks[x][WIDTH - 1][z] == Block::Air) visible_sides |= Block::Front;

                        Block b{blocks[x][y][z], visible_sides};
                        b.translate(location.first * WIDTH + x, location.second * WIDTH + y, z);
                        buffer += b;
                    }
                }
            }
            buffer_ = NativeBuffer(device, buffer.size());
            buffer_.fill(buffer);
            modifed_ = false;
        }
        return buffer_;
    }
};

class World {
private:

    struct hash {
        size_t operator()(const Chunk::Location& loc) const{
            auto hash1 = std::hash<int>{}(loc.first);
            auto hash2 = std::hash<int>{}(loc.second);
            return hash1 ^ hash2;
        }
    };

    std::unordered_map<Chunk::Location, Chunk, hash> chunks;
    std::vector<PlayerCamera> players;
    
public:

    bool isChunkGenerated(Chunk::Location loc) const {
        auto it = chunks.find(loc);
        return it != chunks.end();
    }

    Chunk* generateChunk(Chunk::Location loc) {
        auto it = chunks.emplace(loc, Chunk(loc)).first;
        return &it->second;
    }

    Chunk* getChunk(Chunk::Location loc) {
        auto it = chunks.find(loc);
        if (it == chunks.end()) {
            return generateChunk(loc);
        } else return &it->second;
    }

    std::vector<Chunk*> getChunksWithinRenderDistance(int d = 4) {
        std::vector<Chunk*> chunks;
        for (int i = -d; i <= d; i++) {
            for (int j = -d; j <= d; j++) {
                if (i * i + j * j <= d * d) {
                    chunks.push_back(getChunk({i, j}));
                }
            }
        }
        return chunks;
    }
    
};

class GameEngine {
public:

private:
    NativeDevice device_;
    World world_;
    std::function<void(const std::vector<NativeBuffer> &buffers)> draw_;

public:

    GameEngine(NativeDevice device, std::function<void(const std::vector<NativeBuffer> &buffers)> draw) {
        device_ = device;
        draw_ = draw;
    }
    
    void update() {};

    void render() {
        std::vector<NativeBuffer> buffers;
        int d = 15;
        for(Chunk *chunk: world_.getChunksWithinRenderDistance(d)) {
            assert(chunk != nullptr);

            if (chunk->isModified()) {
                auto [i, j] = chunk->getLocation();
                std::array<Chunk*, 4> neighbors = {nullptr, nullptr, nullptr, nullptr};

                if ((i - 1) * (i - 1) + j * j <= d * d) {
                    neighbors[0] = world_.getChunk(Chunk::Location(i - 1, j));
                }

                if ((i + 1) * (i + 1) + j * j <= d * d) {
                    neighbors[1] = world_.getChunk(Chunk::Location(i + 1, j));
                }

                if (i * i + (j - 1) * (j - 1) <= d * d) {
                    neighbors[2] = world_.getChunk(Chunk::Location(i, j - 1));
                }

                if (i * i + (j + 1) * (j + 1) <= d * d) {
                    neighbors[3] = world_.getChunk(Chunk::Location(i, j + 1));
                }

                buffers.push_back(chunk->computeBuffer(device_, neighbors));

            } else {
                buffers.push_back(chunk->getBuffer());
            }

        }

        draw_(buffers);
    };

};

#endif /* GAME_ENGINE_H */