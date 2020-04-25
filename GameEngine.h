#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "Buffer.h"

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
                for (int z = 0; z < HEIGHT; z++) {
                    if (z <= Biome::SEA_LEVEL - 4) {
                        blocks[x][y][z] = Block::Stone;
                    } else if (z < Biome::SEA_LEVEL) {
                        blocks[x][y][z] = Block::Dirt;
                    } else if (z == Biome::SEA_LEVEL) {
                        blocks[x][y][z] = Block::Grass;
                    }
                }
            }
        }
    }

    const NativeBuffer& getBuffer(NativeDevice device) {
        if (modifed_) {
            Buffer buffer = Buffer();
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < WIDTH; y++) {
                    for (int z = 0; z < HEIGHT; z++) {
                        if (blocks[x][y][z] == Block::Air) continue;
                        uint8_t visible_sides = 0;
                        if (x == WIDTH - 1|| blocks[x + 1][y][z] == Block::Air) visible_sides |= Block::Right;
                        if (x == 0 || blocks[x - 1][y][z] == Block::Air) visible_sides |= Block::Left;
                        if (y == WIDTH - 1|| blocks[x][y + 1][z] == Block::Air) visible_sides |= Block::Back;
                        if (y == 0 || blocks[x][y - 1][z] == Block::Air) visible_sides |= Block::Front;
                        if (z == HEIGHT - 1 || blocks[x][y][z + 1] == Block::Air) visible_sides |= Block::Top;
                        if (z == 0 || blocks[x][y][z - 1] == Block::Air) visible_sides |= Block::Bottom;
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

public:

    bool isChunkGenerated(Chunk::Location loc) const {
        auto it = chunks.find(loc);
        return it != chunks.end();
    }

    const Chunk& generateChunk(Chunk::Location loc) {
        auto it = chunks.emplace(loc, Chunk(loc)).first;
        return it->second;
    }

    const Chunk& getChunk(Chunk::Location loc) const {
        auto it = chunks.find(loc);
        if (it == chunks.end()) {
            throw std::runtime_error{"error: chunk has not been generated yet"};
        } else return it->second;
    }
};

class GameEngine {
public:

private:
    NativeDevice nativeDevice;
    std::vector<Chunk> chunks;
    std::function<void(const std::vector<NativeBuffer> &buffers)> draw;

public:

    GameEngine(NativeDevice device, std::function<void(const std::vector<NativeBuffer> &buffers)> draw) {
        this->nativeDevice = device;
        this->draw = draw;
        for (int i = -3; i < 3; i++) {
            for (int j = -3; j < 3; j++) {
                Chunk::Location loc{i, j};
                chunks.push_back(Chunk{loc});
            }
        }
    }
    
    void update() {};

    void render() {
        std::vector<NativeBuffer> buffers;
        for(Chunk &chunk: chunks) {
            buffers.push_back(chunk.getBuffer(nativeDevice));
        }
        draw(buffers);
    
    };

};

#endif /* GAME_ENGINE_H */