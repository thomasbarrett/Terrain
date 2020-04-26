class PlayerCamera {
private:
    float x_ = 0.0f;
    float y_ = 0.0f;
    float z_ = 0.0f;
    float theta_ = 0.0f;

public:
    PlayerCamera() = default;

    float x() const { return x_; }
    float y() const { return y_; }
    float z() const { return z_; }
    float theta() const { return theta_; }

    void rotateTheta(float dtheta) {
        theta_ += dtheta;
    }

    void moveForwards(float dx) {
        simd::float4 velocity = {0, 1, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ += velocity.x;
        y_ += velocity.y;
    }

    void moveBackwards(float dx) {
        simd::float4 velocity = {0, 1, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ -= velocity.x;
        y_ -= velocity.y;
    }

    void moveLeft(float dx) {
        simd::float4 velocity = {1, 0, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ += velocity.x;
        y_ += velocity.y;
    }

    void moveRight(float dx) {
        simd::float4 velocity = {1, 0, 0, 1};
        matrix_float4x4 rot = matrix_from_rotation(-theta_ * M_PI / 180.0, 0, 0, 1);
        velocity = simd_mul(rot, velocity);
        x_ -= velocity.x;
        y_ -= velocity.y;
    }

    void moveUp(float dz) {
        z_ += dz;
    }

    void moveDown(float dz) {
        z_ -= dz;
    }
};