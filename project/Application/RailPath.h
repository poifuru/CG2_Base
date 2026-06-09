#pragma once
#include <vector>
#include "struct.h"

class RailPath {
public:
    RailPath();
    ~RailPath();

    void Initialize(const std::vector<Vector3>& controlPoints, float speed);
    void Update();

    // ゲッター
    const Vector3& GetPosition() const { return position_; }
    const Matrix4x4& GetRotationMatrix() const { return rotationMatrix_; }
    bool IsFinished() const { return isFinished_; }

private:
    std::vector<Vector3> controlPoints_;
    float speed_ = 0.005f; // セグメントごとの進む速さ

    size_t currentSegment_ = 0;
    float t_ = 0.0f;

    Vector3 position_ = { 0.0f, 0.0f, 0.0f };
    Matrix4x4 rotationMatrix_ = {};
    bool isFinished_ = false;
};
