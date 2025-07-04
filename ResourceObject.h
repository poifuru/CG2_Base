#pragma once
#include <d3d12.h>

class ResourceObject{
private:	//メンバ変数
	ID3D12Resource* resource_;

public:	//メンバ関数
	//コンストラクタ
	ResourceObject(ID3D12Resource* resource)
		:resource_(resource)
	{}
	//デストラクタ
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}
	ID3D12Resource* Get() { return resource_; }
};

