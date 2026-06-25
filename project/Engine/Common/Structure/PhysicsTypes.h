#pragma once

// バネ 
struct Spring {
	// アンカー。固定された端の位置
	Vector3 anchor;
	float naturalLength;		//自然長
	float stiffness;			//剛性。バネ定数k
	float dampingCoefficient;	//減衰係数
};

// ボール 
struct Ball {
	Vector3 position;		//ボールの位置
	Vector3 velocity;		//ボールの速度
	Vector3 acceleration;	//ボールの加速度
	float mass;				//ボールの質量
	float radius;			//ボールの半径
	unsigned int color;		//ボールの色
};

// 振り子 
struct Pendulum {
	Vector3 anchor;				//アンカーポイント。固定された端の位置
	Vector3 position;			//振り子の先端
	float length;				//紐の長さ
	float angle;				//現在の角度
	float angularVelocity;		//角速度ω
	float angularAcceleration;	//角加速度ω
};

// 円錐振り子 
struct ConicalPendulum {
	Vector3 anchor;			//アンカー
	Vector3 position;		//振り子の先端
	float length;			//紐の長さ
	float halfApexAngle;	//円錐の頂角の半分
	float angle;			//現在の角度
	float angularVelocity;	//角速度ω
};