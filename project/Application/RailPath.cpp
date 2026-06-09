#include "RailPath.h"
#include "MathFunction.h"
#include <algorithm>

RailPath::RailPath() {
    rotationMatrix_ = Math::MakeIdentity4x4();
}

RailPath::~RailPath() {}

void RailPath::Initialize(const std::vector<Vector3>& controlPoints, float speed) {
    controlPoints_ = controlPoints;
    speed_ = speed;
    currentSegment_ = 0;
    t_ = 0.0f;
    position_ = { 0.0f, 0.0f, 0.0f };
    rotationMatrix_ = Math::MakeIdentity4x4();
    isFinished_ = false;

    if (controlPoints_.size() >= 4) {
        position_ = Math::CatmullRom(
            controlPoints_[0],
            controlPoints_[1],
            controlPoints_[2],
            controlPoints_[3],
            0.0f
        );
    }
}

void RailPath::Update() {
    if (controlPoints_.size() < 4 || isFinished_) {
        return;
    }

    // 進行率 t を進める
    t_ += speed_;
    if (t_ >= 1.0f) {
        t_ -= 1.0f;
        currentSegment_++;
        // 制御点の数的にこれ以上進めない場合、終了
        if (currentSegment_ >= controlPoints_.size() - 3) {
            currentSegment_ = controlPoints_.size() - 4;
            t_ = 1.0f;
            isFinished_ = true;
        }
    }

    // 現在のセグメントの4つの制御点を取得
    const Vector3& p0 = controlPoints_[currentSegment_];
    const Vector3& p1 = controlPoints_[currentSegment_ + 1];
    const Vector3& p2 = controlPoints_[currentSegment_ + 2];
    const Vector3& p3 = controlPoints_[currentSegment_ + 3];

    // 現在の位置を計算
    position_ = Math::CatmullRom(p0, p1, p2, p3, t_);

    // 進行方向（接線）を求める
    // 少し先の位置との差分から方向ベクトルを作る
    float nextT = t_ + 0.01f;
    size_t nextSegment = currentSegment_;
    if (nextT >= 1.0f) {
        if (nextSegment < controlPoints_.size() - 4) {
            nextT -= 1.0f;
            nextSegment++;
        } else {
            nextT = 1.0f;
        }
    }

    Vector3 nextPos = Math::CatmullRom(
        controlPoints_[nextSegment],
        controlPoints_[nextSegment + 1],
        controlPoints_[nextSegment + 2],
        controlPoints_[nextSegment + 3],
        nextT
    );

    Vector3 direction = Math::Subtract(nextPos, position_);
    if (Math::Length(direction) > 0.001f) {
        direction = Math::Normalize(direction);
    } else {
        direction = { 0.0f, 0.0f, 1.0f }; // デフォルトの前方向
    }

    // 標準のZ方向（0, 0, 1）から進行方向への回転行列を計算
    rotationMatrix_ = Math::DirectionToDirection({ 0.0f, 0.0f, 1.0f }, direction);
}
