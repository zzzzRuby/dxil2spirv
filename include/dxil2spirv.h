#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct dxil2spirv_options {
	bool stripName;
	bool enableOptimizer;
	bool enableWrapperFunction;
} dxil2spirv_options;

typedef struct dxil2spirv_result dxil2spirv_result;

dxil2spirv_result *dxil2spirv(const uint8_t *dxil, size_t dxilSize);
void dxil2spirv_free(dxil2spirv_result *result);
size_t dxil2spirv_result_size(dxil2spirv_result *result);
uint32_t *dxil2spirv_result_data(dxil2spirv_result *result);

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)

#include <vector>

void dxil2spirv(std::vector<uint32_t> &spirv, const uint8_t *dxil,
	size_t dxilSize, const dxil2spirv_options *options = nullptr);
#endif