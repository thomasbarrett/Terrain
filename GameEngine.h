#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "Buffer.h"
#include "linalg.h"
#include "PlayerCamera.h"
#include "Perlin.h"

#include <chrono>
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include <future>

int max(int a, int b) {
    return a > b ? a : b;
}

template<typename R> bool is_ready(std::future<R> const& f) { 
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

class Block: public Buffer {
public:

    static constexpr uint8_t Air = 0x0;
    static constexpr uint8_t Dirt = 0x1;
    static constexpr uint8_t Stone = 0x2;
    static constexpr uint8_t Grass = 0x3;
    static constexpr uint8_t Snow = 0x4;
    static constexpr uint8_t Sand = 0x5;
    static constexpr uint8_t Water = 0x6;
    static constexpr uint8_t Ice = 0x7;

    std::pair<int, int> getTexture(uint8_t block, uint8_t side) {
        switch (block) {
        case Grass:
            if (side == Block::Top) return {1, 0};
            else if (side == Block::Bottom) return {3, 0};
            else return {2, 0};
        case Dirt:
            return {3, 0};
        case Snow:
            return {4, 8};
        case Sand:
            return {14, 0};
        case Stone:
            return {0, 0};
        case Water:
            return {12, 15};
        case Ice:
            return {11, 15};
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
            int i, j; std::tie(i, j) = getTexture(block, Top);
            Vertex a = {{-0.5, 0.5, 0.5}, {i / N, (j + 1) / N}, {0.0, 1.0, 0.0}};
            Vertex b = {{-0.5, -0.5, 0.5}, {(i + 1) / N, (j + 1) / N}, {0.0, 1.0, 0.0}};
            Vertex c = {{0.5, -0.5, 0.5}, {(i + 1) / N, j / N}, {0.0, 1.0, 0.0}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}, {0.0, 1.0, 0.0}};
            addQuad(d, c, b, a);
        }

        if (faceSet & Bottom) {
            int i, j; std::tie(i, j) = getTexture(block, Bottom);
            Vertex a = {{-0.5, 0.5, -0.5}, {i / N, (j + 1) / N}, {0.0, -1.0, 0.0}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}, {0.0, -1.0, 0.0}};
            Vertex c = {{0.5, -0.5, -0.5}, {(i + 1) / N, j / N}, {0.0, -1.0, 0.0}};
            Vertex d = {{0.5, 0.5, -0.5}, {i / N, j / N}, {0.0, -1.0, 0.0}};
            addQuad(a, b, c, d);

        }

        if (faceSet & Left) {
            int i, j; std::tie(i, j) = getTexture(block, Left);
            Vertex a = {{-0.5, 0.5, -0.5}, {i / N, (j + 1) / N}, {-1.0, 0.0, 0.0}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}, {-1.0, 0.0, 0.0}};
            Vertex c = {{-0.5, -0.5, 0.5}, {(i + 1) / N, j / N}, {-1.0, 0.0, 0.0}};
            Vertex d = {{-0.5, 0.5, 0.5}, {i / N, j / N}, {-1.0, 0.0, 0.0}};
            addQuad(d, c, b, a);
        }

        if (faceSet & Right) {
            int i, j; std::tie(i, j) = getTexture(block, Right);
            Vertex a = {{0.5, 0.5, -0.5}, {i / N, (j + 1) / N}, {1.0, 0.0, 0.0}};
            Vertex b = {{0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}, {1.0, 0.0, 0.0}};
            Vertex c = {{0.5, -0.5, 0.5}, {(i + 1) / N, j / N}, {1.0, 0.0, 0.0}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}, {1.0, 0.0, 0.0}};
            addQuad(a, b, c, d);
        }

        if (faceSet & Back) {
            int i, j; std::tie(i, j) = getTexture(block, Back);
            Vertex a = {{0.5, 0.5, -0.5}, {i / N, (j + 1) / N}, {0.0, 0.0, 1.0}};
            Vertex b = {{-0.5, 0.5, -0.5}, {(i + 1) / N, (j + 1) / N},{0.0, 0.0, 1.0}};
            Vertex c = {{-0.5, 0.5, 0.5}, {(i + 1) / N, j / N}, {0.0, 0.0, 1.0}};
            Vertex d = {{0.5, 0.5, 0.5}, {i / N, j / N}, {0.0, 0.0, 1.0}};
            addQuad(d, c, b, a);
        }

        if (faceSet & Front) {
            int i, j; std::tie(i, j) = getTexture(block, Front);
            Vertex a = {{0.5, -0.5, -0.5}, {i / N, (j + 1) / N}, {0.0, 0.0, -1.0}};
            Vertex b = {{-0.5, -0.5, -0.5}, {(i + 1) / N, (j + 1) / N}, {0.0, 0.0, -1.0}};
            Vertex c = {{-0.5, -0.5, 0.5}, {(i + 1) / N, j / N}, {0.0, 0.0, -1.0}};
            Vertex d = {{0.5, -0.5, 0.5}, {i / N, j / N}, {0.0, 0.0, -1.0}};
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
    uint8_t blocks[WIDTH + 2][WIDTH + 2][HEIGHT] = {};
    Location location;
    std::future<NativeBuffer> future_buffer_;
    NativeBuffer buffer_;
    bool modified_ = true;
    bool loaded_ = false;
    std::chrono::steady_clock::time_point loaded_time_;

public:

    static int MountainBiomeHeight(int global_x, int global_y) {
        float noise = perlin2d(global_x + 9134, global_y + 2514, 0.02, 3);
        return Biome::SEA_LEVEL + 20 + (int) (72.0 / (1.0 + exp(-10.0 * (noise - 0.5))));
    }

    static int SnowBiomeHeight(int global_x, int global_y) {
        float noise = perlin2d(global_x, global_y, 0.025, 2);
        return Biome::SEA_LEVEL + 40 * (noise - 0.5);
    }

    static int GrassBiomeHeight(int global_x, int global_y) {
        float noise = perlin2d(global_x, global_y, 0.025, 2);
        return Biome::SEA_LEVEL + 40 * (noise - 0.5);
    }

    static int SandBiomeHeight(int global_x, int global_y) {
        float noise = perlin2d(global_x, global_y, 0.015, 1);
        return Biome::SEA_LEVEL + 40 * noise;
    }

    Chunk(Location loc) {
        location = loc;
        for (int x = 0; x < WIDTH + 2; x++) {
            for (int y = 0; y < WIDTH + 2; y++) {
                const int global_x = x - 1 + location.first * WIDTH;
                const int global_y = y - 1 + location.second * WIDTH;

                float noise1 = perlin2d(global_x, global_y, 0.002, 3);
                float noise2 = perlin2d(global_x + 5231, global_y + 8152, 0.002, 3);

                
                float mountain = 0.0;
                float snow = 0.0;
                float grass = 0.0;
                float sand = 0.0;

                if (noise1 >= 0.6 && noise2 >= 0.6) {
                    mountain = 1.0;
                } else if (noise1 >= 0.6 && noise2 >= 0.4) {
                    mountain = abs(noise2 - 0.4) / 0.2;
                } else if (noise1 >= 0.4 && noise2 >= 0.6) {
                    mountain = abs(noise1 - 0.4) / 0.2;
                } else if (noise1 >= 0.4 && noise2 >= 0.4) {
                    mountain = abs(noise1 - 0.4) * abs(noise2 - 0.4) / 0.04;
                }

                if (noise1 >= 0.6 && noise2 <= 0.4) {
                    snow = 1.0;
                } else if (noise1 >= 0.6 && noise2 <= 0.6) {
                    snow = abs(noise2 - 0.6) / 0.2;
                } else if (noise1 >= 0.4 && noise2 <= 0.4) {
                    snow = abs(noise1 - 0.4) / 0.2;
                } else if (noise1 >= 0.4 && noise2 <= 0.6) {
                    snow = abs(noise2 - 0.6) * abs(noise1 - 0.4) / 0.04;
                }

                if (noise1 <= 0.4 && noise2 >= 0.6) {
                    grass = 1.0;
                } else if (noise1 <= 0.4 && noise2 >= 0.4) {
                    grass = abs(noise2 - 0.4) / 0.2;
                } else if (noise1 <= 0.6 && noise2 >= 0.6) {
                    grass = abs(noise1 - 0.6) / 0.2;
                } else if (noise1 <= 0.6 && noise2 >= 0.4) {
                    grass = abs(noise2 - 0.4) *  abs(noise1 - 0.6) / 0.04;
                }

                if (noise1 <= 0.4 && noise2 <= 0.4) {
                    sand = 1.0;
                } else if (noise1 <= 0.4 && noise2 <= 0.6) {
                    sand = abs(noise2 - 0.6) / 0.2;
                } else if (noise1 <= 0.6 && noise2 <= 0.4) {
                    sand = abs(noise1 - 0.6) / 0.2;
                } else if (noise1 <= 0.6 && noise2 <= 0.6) {
                    sand = abs(noise1 - 0.6) * abs(noise2 - 0.6) / 0.04;
                }

                
                float sum = mountain + snow + grass + sand;
                mountain /= sum;
                snow /= sum;
                grass /= sum;
                sand /= sum;
                

                int height = mountain * MountainBiomeHeight(global_x, global_y)
                           + snow * SnowBiomeHeight(global_x, global_y)
                           + grass * GrassBiomeHeight(global_x, global_y)
                           + sand * SandBiomeHeight(global_x, global_y);
                
                
               //int height = 1;

                if (noise1 >= 0.5 && noise2 >= 0.5) {
                    float snow_height_noise = perlin2d(global_x + 4123, global_y + 6461, 0.0, 3);
                    int snow_height = 40 * snow_height_noise - 20;
                    for (int z = 0; z <= height; z++) {
                        if (z == height && z > Biome::SEA_LEVEL + 60 + snow_height) {
                            blocks[x][y][z] = Block::Snow;
                        } else if (z == height) {
                            blocks[x][y][z] = Block::Grass;
                        } else {
                            blocks[x][y][z] = Block::Stone;
                        }
                    }
                } else if (noise1 >= 0.5 && noise2 <= 0.5) {
                    for (int z = 0; z <= max(height, Biome::SEA_LEVEL); z++) {
                        if (z > height) {
                            blocks[x][y][z] = Block::Ice;
                        } else if (z <= height - 4) {  
                            blocks[x][y][z] = Block::Stone;
                        } else if (z < height) {
                            blocks[x][y][z] = Block::Dirt;
                        } else {
                            blocks[x][y][z] = Block::Snow;
                        }
                    }
                } else if (noise1 <= 0.5 && noise2 >= 0.5) {
                    for (int z = 0; z <= max(height, Biome::SEA_LEVEL); z++) {
                        if (z > height) {
                            blocks[x][y][z] = Block::Water;
                        } else if (z <= height - 4) {  
                            blocks[x][y][z] = Block::Stone;
                        } else if (z < height) {
                            blocks[x][y][z] = Block::Dirt;
                        } else {
                            blocks[x][y][z] = Block::Grass;
                        }
                    }
                } else if (noise1 <= 0.5 && noise2 <= 0.5) {
                    for (int z = 0; z <= height; z++) {
                        if (z <= height - 4) {  
                            blocks[x][y][z] = Block::Stone;
                        } else if (z < height) {
                            blocks[x][y][z] = Block::Sand;
                        } else {
                            blocks[x][y][z] = Block::Sand;
                        }
                    }
                }
            }
        }
    }
    
    Location getLocation() const {
        return location;
    }


    float secondsSinceFirstLoaded() const {
        typedef std::chrono::duration<float> seconds;
        auto now = std::chrono::steady_clock::now(); 
        auto elapsed = std::chrono::duration_cast<seconds>(now - loaded_time_);
        float dt = elapsed.count();
        return dt;
    }

    void setModified() {
        modified_ = true;
    }

    bool isModified() const {
        return modified_;
    }

   NativeBuffer* getBuffer() {
        if (loaded_) {
            buffer_.secondsSinceFirstLoaded_ = secondsSinceFirstLoaded();
            return &buffer_;
        }
        if (future_buffer_.valid() && is_ready(future_buffer_)) {
            buffer_ = future_buffer_.get();
            loaded_ = true;
            buffer_.secondsSinceFirstLoaded_ = secondsSinceFirstLoaded();
            return &buffer_;
        } else return nullptr;
    }

    void computeBuffer(NativeDevice device) {
        if (modified_) {
            future_buffer_ = std::async([=]() {
                Buffer buffer = Buffer();

                for (int x = 1; x <= WIDTH; x++) {
                    for (int y = 1; y <= WIDTH; y++) {
                        for (int z = 0; z < HEIGHT; z++) {
                            if (blocks[x][y][z] == Block::Air) continue;
                            uint8_t visible_sides = 0;
                            
                            if (blocks[x + 1][y][z] == Block::Air) visible_sides |= Block::Right;
                            if (blocks[x - 1][y][z] == Block::Air) visible_sides |= Block::Left;
                            if (blocks[x][y + 1][z] == Block::Air) visible_sides |= Block::Back;
                            if (blocks[x][y - 1][z] == Block::Air) visible_sides |= Block::Front;
                            if (blocks[x][y][z + 1] == Block::Air) visible_sides |= Block::Top;
                            if (blocks[x][y][z - 1] == Block::Air) visible_sides |= Block::Bottom;

                            Block b{blocks[x][y][z], visible_sides};
                            b.translate(location.first * WIDTH + x - 1, location.second * WIDTH + y - 1, z);
                            buffer += b;
                        }
                    }
                }
                NativeBuffer buffer_ = NativeBuffer(device, buffer.size());
                buffer_.fill(buffer);
                loaded_time_ = std::chrono::steady_clock::now();
                return buffer_;
            });
            
            modified_ = false;
        }
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

    World() {
        PlayerCamera camera(0, 0, Biome::SEA_LEVEL + 50, 0);
        players.push_back(camera);
    }

    bool isChunkGenerated(Chunk::Location loc) const {
        auto it = chunks.find(loc);
        return it != chunks.end();
    }

    Chunk* generateChunk(Chunk::Location loc) {
        auto it = chunks.emplace(loc, Chunk(loc)).first;
        return &it->second;
    }

    PlayerCamera& playerCamera() {
        return players[0];
    }

    Chunk* getChunk(Chunk::Location loc) {
        auto it = chunks.find(loc);
        if (it == chunks.end()) {
            return generateChunk(loc);
        } else return &it->second;
    }

    std::vector<Chunk*> getChunksWithinRenderDistance(int d = 4) {
        std::vector<Chunk*> chunks;

        int a = (int) playerCamera().x() / Chunk::WIDTH;
        int b = (int) playerCamera().y() / Chunk::WIDTH;

        for (int i = -d; i <= d; i++) {
            for (int j = -d; j <= d; j++) {
                if (i * i + j * j <= d * d) {
                    chunks.push_back(getChunk({i + a, j + b}));
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

    bool forwards = false;
    bool backwards = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

public:

    PlayerCamera& playerCamera() {
        return world_.playerCamera();
    }    

    void setDevice(NativeDevice device) {
        device_ = device;
    }

    void setDrawFunction(std::function<void(const std::vector<NativeBuffer> &buffers)> draw) {
        draw_ = draw;
    }

    bool onKeyPress(char c) {
        if (c == 'w') {
            forwards = true;
            return true;
        } else if (c == 'a') {
            left = true;
            return true;
        } else if (c == 's') {
            backwards = true;
            return true;
        } else if (c == 'd') {
            right = true;
            return true;
        } else if (c == 'z') {
            up = true;
            return true;
        } else if (c == 'x') {
            down = true;
            return true;
        } else return false;
    }

    bool onKeyRelease(char c) {
        if (c == 'w') {
            forwards = false;
            return true;
        } else if (c == 'a') {
            left = false;
            return true;
        } else if (c == 's') {
            backwards = false;
            return true;
        } else if (c == 'd') {
            right = false;
            return true;
        } else if (c == 'z') {
            up = false;
            return true;
        } else if (c == 'x') {
            down = false;
            return true;
        } else return false;
    }

    void onMouseDragged(int dx, int dy) {

    }

    void update() {
        typedef std::chrono::duration<float> seconds;
        static auto start = std::chrono::steady_clock::now();
        auto end = std::chrono::steady_clock::now(); 
        auto elapsed = std::chrono::duration_cast<seconds>(end - start);
        start = end;
        float dt = elapsed.count();

        if (forwards) playerCamera().moveForwards(dt);
        if (left) playerCamera().moveLeft(dt);
        if (backwards) playerCamera().moveBackwards(dt);
        if (right) playerCamera().moveRight(dt);
        if (up) playerCamera().moveUp(dt);
        if (down) playerCamera().moveDown(dt);
    }
    void render() {
        std::vector<NativeBuffer> buffers;

        update();

        int d = 10;

        for(Chunk *chunk: world_.getChunksWithinRenderDistance(d)) {
            if (chunk->isModified()) {
                chunk->computeBuffer(device_);
            } else {
                NativeBuffer* buffer = chunk->getBuffer();
                if (buffer != nullptr) {
                    buffers.push_back(*buffer);
                }
            }

        }

        draw_(buffers);
    };

};

#endif /* GAME_ENGINE_H */