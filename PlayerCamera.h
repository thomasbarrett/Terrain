#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

class PlayerCamera {
private:
    float x_ = 0.0f;
    float y_ = 0.0f;
    float z_ = 0.0f;
    float theta_ = 0.0f;
    float v_ = 100.0;
public:
    PlayerCamera(float x, float y, float z, float theta) {
        x_ = x;
        y_ = y;
        z_ = z;
        theta_ = theta;
    };

    float x() const { return x_; }
    float y() const { return y_; }
    float z() const { return z_; }
    float theta() const { return theta_; }

    void rotateTheta(float dtheta) {
        theta_ += dtheta;
    }

    void moveForwards(float dt) {
        simd::float4 velocity = {0, v_, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ += dt * velocity.x;
        y_ += dt * velocity.y;
    }

    void moveBackwards(float dt) {
        simd::float4 velocity = {0, v_, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ -= dt * velocity.x;
        y_ -= dt * velocity.y;
    }

    void moveLeft(float dt) {
        simd::float4 velocity = {v_, 0, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ -= dt * velocity.x;
        y_ -= dt * velocity.y;
    }

    void moveRight(float dt) {
        simd::float4 velocity = {v_, 0, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ += dt * velocity.x;
        y_ += dt * velocity.y;
    }

    void moveUp(float dt) {
        z_ += dt * v_;
    }

    void moveDown(float dt) {
        z_ -= dt * v_;
    }
};

#endif /* PLAYER_CAMERA_H */