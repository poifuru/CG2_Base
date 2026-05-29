#include <cassert>
#include <fstream>
#include "RootSignatureManager.h"
#include "DxCommon.h"
#include "LogManager.h"

void RootSignatureManager::Initialize (DxCommon* dxCommon) {
	device_ = dxCommon->GetDevice ();
	commandList_ = dxCommon->GetCommandList ();

	//===RootSigTypeごとの定義===///
#pragma region standard3D
	//環境マップ用(t0)PS
	standard3DDescriptorRanges[0].BaseShaderRegister = 0;
	standard3DDescriptorRanges[0].NumDescriptors = 1;
	standard3DDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//Texture用(t1)PS
	standard3DDescriptorRanges[1].BaseShaderRegister = 1;
	standard3DDescriptorRanges[1].NumDescriptors = 1;		//数は1つ
	standard3DDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	standard3DDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	// === 共通バッファ === //
	// カメラのワールド座標を送る用(b0)PS
	standard3DRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	standard3DRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DRootParameters[0].Descriptor.ShaderRegister = 0;

	// ライトバッファ(b1)PS
	standard3DRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	standard3DRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DRootParameters[1].Descriptor.ShaderRegister = 1;

	//環境マップ用(t0)PS
	standard3DRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DRootParameters[2].DescriptorTable.pDescriptorRanges = &standard3DDescriptorRanges[0];
	standard3DRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	// ====== //

	// 行列バッファ(b0)VS
	standard3DRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	standard3DRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	standard3DRootParameters[3].Descriptor.ShaderRegister = 0;

	// マテリアルバッファ(b2)PS
	standard3DRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	standard3DRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	standard3DRootParameters[4].Descriptor.ShaderRegister = 2;						//レジスタ番号とバインド

	//Texture用(t1)PS
	standard3DRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DescriptorTableを使う
	standard3DRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	standard3DRootParameters[5].DescriptorTable.pDescriptorRanges = &standard3DDescriptorRanges[1];	//Tableの中身の配列を指定
	standard3DRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//Sampler
	standard3DStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	standard3DStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	standard3DStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	standard3DStaticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	standard3DStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
#pragma endregion

#pragma region standard3DInstance
	//Texture用
	standard3DInstanceDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	standard3DInstanceDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	standard3DInstanceDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	standard3DInstanceDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//DirectionalLight用
	standard3DInstanceDescriptorRanges[1].BaseShaderRegister = 1; // register(t1)に対応
	standard3DInstanceDescriptorRanges[1].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//PointLight用
	standard3DInstanceDescriptorRanges[2].BaseShaderRegister = 2; // register(t2)に対応
	standard3DInstanceDescriptorRanges[2].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//PointLight用
	standard3DInstanceDescriptorRanges[3].BaseShaderRegister = 3; // register(t3)に対応
	standard3DInstanceDescriptorRanges[3].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//RectLight用
	standard3DInstanceDescriptorRanges[4].BaseShaderRegister = 4; // register(t4)に対応
	standard3DInstanceDescriptorRanges[4].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//matrixBuffer用
	standard3DInstanceDescriptorRanges[5].BaseShaderRegister = 0; // register(t0)に対応
	standard3DInstanceDescriptorRanges[5].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//MatrixPalette用(ダミー)
	standard3DInstanceDescriptorRanges[6].BaseShaderRegister = 5; // register(t5)に対応
	standard3DInstanceDescriptorRanges[6].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//環境マップ用
	standard3DInstanceDescriptorRanges[7].BaseShaderRegister = 5; // register(t5)に対応
	standard3DInstanceDescriptorRanges[7].NumDescriptors = 1;
	standard3DInstanceDescriptorRanges[7].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	standard3DInstanceDescriptorRanges[7].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//MatrixBufferInstance(b0)
	standard3DInstanceRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//DiscriptorTableを使う
	standard3DInstanceRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	standard3DInstanceRootParameters[0].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[5];	//Tableの中身の配列を指定
	standard3DInstanceRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//(b1)
	standard3DInstanceRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	standard3DInstanceRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	standard3DInstanceRootParameters[1].Descriptor.ShaderRegister = 1;						//レジスタ番号とバインド

	//カメラのワールド座標を送る用(b2)
	standard3DInstanceRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	standard3DInstanceRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[2].Descriptor.ShaderRegister = 2;

	//ライトの個数を送る用(b3)
	standard3DInstanceRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	standard3DInstanceRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[3].Descriptor.ShaderRegister = 3;

	//Texture用のディスクリプタテーブル(t0)
	standard3DInstanceRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DiscriptorTableを使う
	standard3DInstanceRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	standard3DInstanceRootParameters[4].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[0];	//Tableの中身の配列を指定
	standard3DInstanceRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//DirLight一覧用のディスクリプタテーブル(t1用)
	standard3DInstanceRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[5].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[1]; // Range[1]を指す
	standard3DInstanceRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;

	//PointLight一覧用のディスクリプタテーブル(t2用)
	standard3DInstanceRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[6].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[2]; // Range[2]を指す
	standard3DInstanceRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;

	//PointLight一覧用のディスクリプタテーブル(t3用)
	standard3DInstanceRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[7].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[3]; // Range[3]を指す
	standard3DInstanceRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;

	//RectLight一覧用のディスクリプタテーブル(t4用)
	standard3DInstanceRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[8].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[4]; // Range[4]を指す
	standard3DInstanceRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;

	//MatrixPalette用(t5, ダミー)
	standard3DInstanceRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	standard3DInstanceRootParameters[9].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[6]; // Range[6]を指す
	standard3DInstanceRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;

	//環境マップ用(t5)
	standard3DInstanceRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	standard3DInstanceRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	standard3DInstanceRootParameters[10].DescriptorTable.pDescriptorRanges = &standard3DInstanceDescriptorRanges[7]; // Range[7]を指す
	standard3DInstanceRootParameters[10].DescriptorTable.NumDescriptorRanges = 1;

	//Sampler
	standard3DInstanceStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	standard3DInstanceStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DInstanceStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DInstanceStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	standard3DInstanceStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	standard3DInstanceStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	standard3DInstanceStaticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	standard3DInstanceStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
#pragma endregion

#pragma region skinningStandard3D
	//環境マップ用(t0)PS
	skinningStandard3DDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	skinningStandard3DDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	skinningStandard3DDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	skinningStandard3DDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//テクスチャ用(t1)PS
	skinningStandard3DDescriptorRanges[1].BaseShaderRegister = 1; // register(t1)に対応
	skinningStandard3DDescriptorRanges[1].NumDescriptors = 1;
	skinningStandard3DDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	skinningStandard3DDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//MatrixPalette用(t0)VS
	skinningStandard3DDescriptorRanges[2].BaseShaderRegister = 0; // register(t0)に対応
	skinningStandard3DDescriptorRanges[2].NumDescriptors = 1;
	skinningStandard3DDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	skinningStandard3DDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//カメラのワールド座標を送る用(b0)PS
	skinningStandard3DRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	skinningStandard3DRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skinningStandard3DRootParameters[0].Descriptor.ShaderRegister = 0;

	//ライトの個数を送る用(b1)PS
	skinningStandard3DRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	skinningStandard3DRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skinningStandard3DRootParameters[1].Descriptor.ShaderRegister = 1;

	//環境マップ用(t0)PS
	skinningStandard3DRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	skinningStandard3DRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skinningStandard3DRootParameters[2].DescriptorTable.pDescriptorRanges = &skinningStandard3DDescriptorRanges[0];
	skinningStandard3DRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	//行列バッファ(b0)VS
	skinningStandard3DRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	skinningStandard3DRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	skinningStandard3DRootParameters[3].Descriptor.ShaderRegister = 0;

	//マテリアルバッファ(b2)PS
	skinningStandard3DRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	skinningStandard3DRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	skinningStandard3DRootParameters[4].Descriptor.ShaderRegister = 2;						//レジスタ番号とバインド

	//Texture用のディスクリプタテーブル(t1)PS
	skinningStandard3DRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	//DiscriptorTableを使う
	skinningStandard3DRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	skinningStandard3DRootParameters[5].DescriptorTable.pDescriptorRanges = &skinningStandard3DDescriptorRanges[1];
	skinningStandard3DRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//MatrixPalette用(t0)VS
	skinningStandard3DRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	skinningStandard3DRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	skinningStandard3DRootParameters[6].DescriptorTable.pDescriptorRanges = &skinningStandard3DDescriptorRanges[2];
	skinningStandard3DRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;

	//Sampler
	skinningStandard3DStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	skinningStandard3DStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skinningStandard3DStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skinningStandard3DStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skinningStandard3DStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	skinningStandard3DStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	skinningStandard3DStaticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	skinningStandard3DStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
#pragma endregion

#pragma region Sprite
	//Texture用
	spriteDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	spriteDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	spriteDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	spriteDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//matrixBuffer(b0)
	spriteRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	spriteRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	//VertexShaderで使う
	spriteRootParameters[0].Descriptor.ShaderRegister = 0;				//レジスタ番号とバインド

	//materialBuffer(b1)
	spriteRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	spriteRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	spriteRootParameters[1].Descriptor.ShaderRegister = 1;				//レジスタ番号とバインド

	//Texture(t0)
	spriteRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;		//ディスクリプタテーブルを使う
	spriteRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	//PixelShaderで使う
	spriteRootParameters[2].DescriptorTable.pDescriptorRanges = &spriteDescriptorRanges[0];
	spriteRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	//Sampler(s0)
	spriteStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	//バイリニアフィルタ
	spriteStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	spriteStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	spriteStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	spriteStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	//比較しない
	spriteStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;	//ありったけのmipmapを使う
	spriteStaticSamplers[0].ShaderRegister = 0;	//レジスタ番号0を使う
	spriteStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
#pragma endregion

#pragma region Particle
	//位置情報のインスタンスバッファ(t0)VS
	particleDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	particleDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	particleDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	particleDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算
	
	//textureDescriptorRange(t0)PS
	textureDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	textureDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	textureDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	textureDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//RootParameter
	//位置情報のインスタンスバッファ(t0)VS
	particleRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;					//DescriptorTableを使う
	particleRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;							//VertexShaderで使う
	particleRootParameters[0].DescriptorTable.pDescriptorRanges = &particleDescriptorRanges[0];					//t0(SRV)を指定
	particleRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//マテリアルバッファ(b1)PS
	particleRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;		//CBVを使う
	particleRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
	particleRootParameters[1].Descriptor.ShaderRegister = 1;						//b1を指定

	//Texture(t0)PS
	particleRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;				//DiscriptorTableを使う
	particleRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;							//PixelShaderで使う
	particleRootParameters[2].DescriptorTable.pDescriptorRanges = &textureDescriptorRanges[0];				//t0(SRV)を指定
	particleRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;	//Tableで利用する数

	//Sampler
	particleStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;				//バイリニアフィルタ
	particleStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	particleStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;			//比較しない
	particleStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;							//ありったけのmipmapを使う
	particleStaticSamplers[0].ShaderRegister = 0;									//s0を指定
	particleStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		//PixelShaderで使う
#pragma endregion

#pragma region LineMesh
	//DescriptorRange
	//行列データのインスタンシング用(VSで使うt0レジスタ用)
	lineMeshDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	lineMeshDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	lineMeshDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	lineMeshDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//頂点データのインスタンシング用(VSで使うt1レジスタ用)
	lineMeshDescriptorRanges[1].BaseShaderRegister = 1;	// t1
	lineMeshDescriptorRanges[1].NumDescriptors = 1;
	lineMeshDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	lineMeshDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
	//CBV(b0)用のrootParametor
	lineMeshRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;			//CBVを使う
	lineMeshRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;		//Vertex Shaderを使う
	lineMeshRootParameters[0].Descriptor.ShaderRegister = 0;							//b0番を使う(CBV)
	
	//StructuedBuffer(VSのt0)用のDescriptorTable
	lineMeshRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;					//DescriptorTableを使う
	lineMeshRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;							//VertexShaderで使う
	lineMeshRootParameters[1].DescriptorTable.pDescriptorRanges = lineMeshDescriptorRanges;						//t0(SRV)を指定
	lineMeshRootParameters[1].DescriptorTable.NumDescriptorRanges = _countof (lineMeshDescriptorRanges);		//Tableで利用する数
#pragma endregion

#pragma region CubeMesh
	//DescriptorRange
	//行列データのインスタンシング用(VSで使うt0レジスタ用)
	cubeMeshDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる
	cubeMeshDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	cubeMeshDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	cubeMeshDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//頂点データのインスタンシング用(VSで使うt1レジスタ用)
	cubeMeshDescriptorRanges[1].BaseShaderRegister = 1;	// t1
	cubeMeshDescriptorRanges[1].NumDescriptors = 1;
	cubeMeshDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	cubeMeshDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//CBV(b0)用のrootParametor
	cubeMeshRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;			//CBVを使う
	cubeMeshRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;		//Vertex Shaderを使う
	cubeMeshRootParameters[0].Descriptor.ShaderRegister = 0;							//b0番を使う(CBV)

	//StructuedBuffer(VSのt0)用のDescriptorTable
	cubeMeshRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;					//DescriptorTableを使う
	cubeMeshRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;	
	cubeMeshRootParameters[1].DescriptorTable.pDescriptorRanges = cubeMeshDescriptorRanges; //VertexShaderで使う
	cubeMeshRootParameters[1].DescriptorTable.NumDescriptorRanges = _countof (cubeMeshDescriptorRanges);		//Tableで利用する数
#pragma endregion

#pragma region Skybox
	//DescriptorRange
	skyboxDescriptorRanges[0].BaseShaderRegister = 0;	//0から始まる (t0)
	skyboxDescriptorRanges[0].NumDescriptors = 1;		//数は1つ
	skyboxDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	//SRVを使う
	skyboxDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;	//Offsetを自動計算

	//CBV(b0, VS)用のrootParameter : TransformationMatrix
	skyboxRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	skyboxRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	skyboxRootParameters[0].Descriptor.ShaderRegister = 0;

	//CBV(b0, PS)用のrootParameter : Material
	skyboxRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	skyboxRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skyboxRootParameters[1].Descriptor.ShaderRegister = 0;

	//DescriptorTable(t0, PS)用のrootParameter : TextureCube
	skyboxRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	skyboxRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	skyboxRootParameters[2].DescriptorTable.pDescriptorRanges = skyboxDescriptorRanges;
	skyboxRootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(skyboxDescriptorRanges);

	//Sampler
	skyboxStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	skyboxStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skyboxStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skyboxStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	skyboxStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	skyboxStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	skyboxStaticSamplers[0].ShaderRegister = 0;
	skyboxStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#pragma endregion

#pragma region PostProcess
	// Texture用のディスクリプタレンジ(t0)
	postProcessDescriptorRanges[0].BaseShaderRegister = 0;	// 0から始まる
	postProcessDescriptorRanges[0].NumDescriptors = 1;		// 数は1つ
	postProcessDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;	// SRVを使う
	postProcessDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Texture用のディスクリプタテーブル(t0)
	postProcessRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	postProcessRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	postProcessRootParameters[0].DescriptorTable.pDescriptorRanges = &postProcessDescriptorRanges[0];
	postProcessRootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(postProcessDescriptorRanges);

	// Sampler (s0)
	postProcessStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;	// バイリニアフィルタ
	postProcessStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	postProcessStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	postProcessStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	postProcessStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	// 比較しない
	postProcessStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	postProcessStaticSamplers[0].ShaderRegister = 0;	// レジスタ番号0を使う
	postProcessStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;		// PixelShaderで使う
#pragma endregion
}

uint32_t RootSignatureManager::GetOrCreateRootSignature (RootSigType type) {
	HRESULT hr;

	//まずRootSigTypeをハッシュ代わりにチェック
	if(m_TypeToID.count(type)) {
		return m_TypeToID.at(type);
	}

	//無ければRootSigTypeからdescを作成
	D3D12_ROOT_SIGNATURE_DESC desc = CreateRootSigDesc (type);

	//ハッシュがヒットしなかったら新規作成
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	//RootSignatureのシリアライズ(バイナリ変換)
	hr = D3D12SerializeRootSignature (
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);
	//失敗したら
	if (FAILED (hr)) {
		//ログを出力
		if(errorBlob) { // errorBlobがあるときだけログを出す
			LogManager::GetInstance()->LogManager::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert (false && "RootSignature serialization failed!");
		return 0;
	}

	//デバイスでRootSignatureを作成
	ComPtr<ID3D12RootSignature> newRootSig;
	assert(device_);	//デバイスがnullptrじゃないかチェック
	hr = device_->CreateRootSignature (
		0,		//ノードマスク(単一GPUなら0)
		signatureBlob->GetBufferPointer (),
		signatureBlob->GetBufferSize (),
		IID_PPV_ARGS (newRootSig.GetAddressOf ())
	);
	assert (SUCCEEDED (hr) && "Root Signature creation failed!");
	if (FAILED (hr)) return 0;

	//キャッシュに登録
	//IDを割り当てた後インクリメント
	uint32_t newID = m_NextID++;

	m_RootSigCache[newID] = newRootSig;	//実体データの登録
	m_TypeToID[type] = newID;			//逆引きハッシュの登録

	return newID;
}

ID3D12RootSignature* RootSignatureManager::GetRootSignature (uint32_t rootSigID) const {
	// IDが存在するかチェック
	if (m_RootSigCache.count (rootSigID)) {
		//存在していたらそれを返す
		return m_RootSigCache.at (rootSigID).Get ();
	}
	// 見つからなかったらassertでnullptrを返す
	assert (false && "RootSignature ID not found in cache!");
	return nullptr;
}

void RootSignatureManager::SetRootSignature (uint32_t rootSigID) {
	auto rootSig = GetRootSignature (rootSigID);
	commandList_->SetGraphicsRootSignature (rootSig);
}

D3D12_ROOT_SIGNATURE_DESC RootSignatureManager::CreateRootSigDesc (RootSigType type) {
	D3D12_ROOT_SIGNATURE_DESC desc = {};

	//作る設定を分岐させる
	switch (type) {
	case RootSigType::Standard3D:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParameter作成。複数設定できるので配列
		desc.pParameters = standard3DRootParameters;				//ルートパラメータ配列へのポインタ
		desc.NumParameters = _countof (standard3DRootParameters);	//配列の長さ

		//Sampler
		desc.pStaticSamplers = standard3DStaticSamplers;
		desc.NumStaticSamplers = _countof (standard3DStaticSamplers);

		break;

	case RootSigType::Standard3DInstance:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParameter作成。複数設定できるので配列
		desc.pParameters = standard3DInstanceRootParameters;				//ルートパラメータ配列へのポインタ
		desc.NumParameters = _countof(standard3DInstanceRootParameters);	//配列の長さ

		//Sampler
		desc.pStaticSamplers = standard3DInstanceStaticSamplers;
		desc.NumStaticSamplers = _countof(standard3DInstanceStaticSamplers);
		break;

	case RootSigType::SkinningStandard3D:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParameter作成。複数設定できるので配列
		desc.pParameters = skinningStandard3DRootParameters;				//ルートパラメータ配列へのポインタ
		desc.NumParameters = _countof(skinningStandard3DRootParameters);	//配列の長さ

		//Sampler
		desc.pStaticSamplers = skinningStandard3DStaticSamplers;
		desc.NumStaticSamplers = _countof(skinningStandard3DStaticSamplers);

		break;

	case RootSigType::Sprite:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParameter作成。複数設定できるので配列
		desc.pParameters = spriteRootParameters;				//ルートパラメータ配列へのポインタ
		desc.NumParameters = _countof(spriteRootParameters);	//配列の長さ

		//Sampler
		desc.pStaticSamplers = spriteStaticSamplers;
		desc.NumStaticSamplers = _countof(spriteStaticSamplers);
		break;

	case RootSigType::Particle:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParameter作成。複数設定できるので配列
		desc.pParameters = particleRootParameters;				//ルートパラメータ配列へのポインタ
		desc.NumParameters = _countof (particleRootParameters);	//配列の長さ

		//Sampler
		desc.pStaticSamplers = particleStaticSamplers;
		desc.NumStaticSamplers = _countof (particleStaticSamplers);
		break;

	case RootSigType::LineMesh:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParametor
		desc.pParameters = lineMeshRootParameters;
		desc.NumParameters = _countof (lineMeshRootParameters);

		//Sampler
		desc.pStaticSamplers = nullptr;
		desc.NumStaticSamplers = 0;
		break;

	case RootSigType::CubeMesh:
		desc.pParameters = cubeMeshRootParameters;
		desc.NumParameters = _countof (cubeMeshRootParameters);
		//静的サンプラーはなし
		desc.pStaticSamplers = nullptr;
		desc.NumStaticSamplers = 0;
		//InputLayoutを使用するように設定
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		break;

	case RootSigType::PostProcess:
		desc.pParameters = postProcessRootParameters;
		desc.NumParameters = _countof (postProcessRootParameters);
		desc.pStaticSamplers = postProcessStaticSamplers;
		desc.NumStaticSamplers = _countof (postProcessStaticSamplers);
		// フルスクリーントライアングルでは頂点バッファを使わないためInputLayoutを許可しない（しても害はないが、今回は不要）
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		break;

	case RootSigType::Skybox:
		//RootSignature
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		//RootParametor
		desc.pParameters = skyboxRootParameters;
		desc.NumParameters = _countof(skyboxRootParameters);

		//Sampler
		desc.pStaticSamplers = skyboxStaticSamplers;
		desc.NumStaticSamplers = _countof(skyboxStaticSamplers);
		break;
	}

	return desc;
}
