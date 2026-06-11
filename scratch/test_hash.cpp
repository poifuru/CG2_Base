#include <iostream>
#include <cstdint>
#include <iomanip>

template<typename T>
uint64_t hash_combine_simple (uint64_t h, T val) {
	return ((h << 5) | (h >> 59)) ^ (uint64_t)val;
}

uint64_t ComputeHash (uint32_t ps_id) {
	uint64_t hash = 0;

	// VS_ID
	hash = hash_combine_simple (hash, 1);
	// PS_ID
	hash = hash_combine_simple (hash, ps_id);
	// RootSignatureID
	hash = hash_combine_simple (hash, 1);
	// InputLayoutID
	hash = hash_combine_simple (hash, 2);
	// BlendMode
	hash = hash_combine_simple (hash, 0);

	// CullMode
	hash = hash_combine_simple (hash, 1);
	// FillMode
	hash = hash_combine_simple (hash, 3);

	// DepthEnable
	hash = hash_combine_simple (hash, 0);
	// DepthWriteMask
	hash = hash_combine_simple (hash, 0);
	// DepthFunc
	hash = hash_combine_simple (hash, 2);

	// PrimitiveTopologyType
	hash = hash_combine_simple (hash, 3);
	// RTVFormat
	hash = hash_combine_simple (hash, 29);
	// NumRenderTargets
	hash = hash_combine_simple (hash, 1);
	// DSVFormat
	hash = hash_combine_simple (hash, 45);

	// SampleCount
	hash = hash_combine_simple (hash, 1);
	// SampleMask
	hash = hash_combine_simple (hash, 0xffffffff);

	return hash;
}

int main() {
    uint64_t hash_cg = ComputeHash(2);
    uint64_t hash_vig = ComputeHash(3);
    std::cout << "ColorGrading Hash: " << std::hex << hash_cg << std::endl;
    std::cout << "Vignette     Hash: " << std::hex << hash_vig << std::endl;
    std::cout << "Conflict? " << (hash_cg == hash_vig ? "YES" : "NO") << std::endl;
    return 0;
}
