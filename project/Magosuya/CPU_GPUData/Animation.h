#pragma once
#include <vector>
#include <map>
#include <string>
#include "struct.h"

// キーフレーム構造体(Vector3やQuaternionに対応させる)
template<typename tValue>
struct Keyframe {
	float time;
	tValue value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template<typename tValue>
// AnimationCurve構造体
struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

// NodeAnimation構造体
struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

// Animation構造体
struct Animation {
	float duration;	// アニメーション全体の尺
	// NodeAnimationの集合。Node名で引けるようにする(map)
	std::map<std::string, NodeAnimation> nodeAnimations;
};

// アニメーション用の関数
namespace AnimationFunc {

	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);

	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);

}