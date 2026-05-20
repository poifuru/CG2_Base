#include "Entity.h"
#include "Primitive.h"

void Entity::DrawHitbox(Vector4 color) {
	auto x = transform_.translate.x;
	auto y = transform_.translate.y;
	auto z = transform_.translate.z;
	auto scaleX = aabbSize_.x;
	auto scaleY = aabbSize_.y;
	auto scaleZ = aabbSize_.z;

	// Box上の頂点8つを定義
	Vector3 leftUpFront = { x - scaleX, y + scaleY, z - scaleZ };		// 左上手前
	Vector3 leftDownFront = { x - scaleX, y - scaleY, z - scaleZ };		//　左下手前
	Vector3 rightDownFront = { x + scaleX, y - scaleY, z - scaleZ };	//　右下手前
	Vector3 rightUpFront = { x + scaleX, y + scaleY, z - scaleZ };		//　右上手前
	Vector3 leftUpBack = { x - scaleX, y + scaleY, z + scaleZ };		//　左上奥
	Vector3 leftDownBack = { x - scaleX, y - scaleY, z + scaleZ };		//　左下奥
	Vector3 rightDownBack = { x + scaleX, y - scaleY, z + scaleZ };		//　右下奥
	Vector3 rightUpBack = { x + scaleX, y + scaleY, z + scaleZ };		//　右上奥

	Primitive::DrawLine(
		leftUpFront.x, leftUpFront.y, leftUpFront.z, leftUpBack.x, leftUpBack.y, leftUpBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		leftUpFront.x, leftUpFront.y, leftUpFront.z, rightUpFront.x, rightUpFront.y, rightUpFront.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightUpBack.x, rightUpBack.y, rightUpBack.z, leftUpBack.x, leftUpBack.y, leftUpBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightUpBack.x, rightUpBack.y, rightUpBack.z, rightUpFront.x, rightUpFront.y, rightUpFront.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		leftUpFront.x, leftUpFront.y, leftUpFront.z, leftDownFront.x, leftDownFront.y, leftDownFront.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		leftUpBack.x, leftUpBack.y, leftUpBack.z, leftDownBack.x, leftDownBack.y, leftDownBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightUpFront.x, rightUpFront.y, rightUpFront.z, rightDownFront.x, rightDownFront.y, rightDownFront.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightUpBack.x, rightUpBack.y, rightUpBack.z, rightDownBack.x, rightDownBack.y, rightDownBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		leftDownFront.x, leftDownFront.y, leftDownFront.z, leftDownBack.x, leftDownBack.y, leftDownBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		leftDownFront.x, leftDownFront.y, leftDownFront.z, rightDownFront.x, rightDownFront.y, rightDownFront.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightDownBack.x, rightDownBack.y, rightDownBack.z, leftDownBack.x, leftDownBack.y, leftDownBack.z, color, camera_->GetVPMatrix()
	);
	Primitive::DrawLine(
		rightDownBack.x, rightDownBack.y, rightDownBack.z, rightDownFront.x, rightDownFront.y, rightDownFront.z, color, camera_->GetVPMatrix()
	);
}