#ifndef BUFFER_H
#define BUFFER_H

#include "ShaderTypes.h"
#include <vector>
#include <Metal/Metal.h>

class Buffer {
private:
    std::vector<Vertex> data_;

protected:
    void addTriangle(const Vertex& a, const Vertex& b, const Vertex& c) {
        data_.push_back(a);
        data_.push_back(b);
        data_.push_back(c);
    }

    void addQuad(Vertex a, Vertex b, Vertex c, Vertex d) {
        data_.push_back(a);
        data_.push_back(b);
        data_.push_back(d);
        data_.push_back(b);
        data_.push_back(c);
        data_.push_back(d);
    }
    
public:

    std::vector<Vertex>::const_iterator begin() const {
        return data_.begin();
    }

    std::vector<Vertex>::const_iterator end() const {
        return data_.end();
    }

    const Vertex *data() const {
        return data_.data();
    }

    size_t size() const {
        return data_.size();
    }

    void operator+=(const Buffer& buffer) {
        data_.insert(data_.end(), buffer.begin(), buffer.end());
    }

    void translate(float x, float y, float z) {
        for (Vertex &v: data_) {
            v.position[0] += x;
            v.position[1] += y;
            v.position[2] += z;
        }
    }
};


using NativeDevice = id<MTLDevice>;

class NativeBuffer {
private:
    id<MTLBuffer> data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
public:
    NativeBuffer() = default;
    NativeBuffer(NativeDevice device, size_t capacity) {
        capacity_ = capacity;
        data_ = [device newBufferWithLength: (capacity * sizeof(Vertex)) 
                        options: MTLResourceOptionCPUCacheModeDefault];
    }

    id<MTLBuffer> data() const {
        return data_;
    }

    int size() const {
        return size_;
    }

    int capacity() const {
        return capacity_;
    }

    void fill(const Buffer &buffer) {
        size_ = buffer.size();
        assert(size_ <= capacity_);
        memcpy([data_ contents], buffer.data(), buffer.size() * sizeof(Vertex));
    }

};

#endif /* BUFFER_H */