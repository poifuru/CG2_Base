#pragma once
#include "header.h"
#include "externals.h"

//共通のインターフェースを提供する基底クラス
//これを継承させてGraphicsPipelineStateやComputePipelineStateでポリモーフィズムを使えるようにする
class PipelineState {
public:
	virtual ~PipelineState() = default;
	virtual void Create(ID3D12Device* device) = 0;

	ID3D12PipelineState* Get() const { return pipelineState.Get(); }

protected:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
};

class GraphicsPipelineState : public PipelineState {
public:
	void SetRootSignature();

	void SetInputLayout();

	void SetBlendState();

	void SetRastaerizerState();

	void SetShaders();

	void Create();

private:

};

