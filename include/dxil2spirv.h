#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
#include <vector>
extern "C" {
#endif

typedef struct dxil2spirv_options {
	bool stripName;
	bool enableOptimizer;
} dxil2spirv_options;

typedef struct dxil2spirv_result dxil2spirv_result;

dxil2spirv_result *dxil2spirv_convert(const uint8_t *dxil, 
	size_t dxilSize, const dxil2spirv_options* option);
void dxil2spirv_free(dxil2spirv_result *result);
size_t dxil2spirv_result_size(dxil2spirv_result *result);
uint32_t *dxil2spirv_result_data(dxil2spirv_result *result);

#if defined(__cplusplus)
}

namespace dxil2spirv
{
	typedef dxil2spirv_options options;
	void convert(std::vector<uint32_t>& spirv, const uint8_t* dxil,
		size_t dxilSize, const options* options = nullptr);
}
#endif