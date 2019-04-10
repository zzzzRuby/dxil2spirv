#pragma once

#include <dxc/DXIL/DXIL.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/ADT/APFloat.h>
#include <iostream>
#include <spirv/unified1/GLSL.std.450.h>
#include <spirv/unified1/spirv.hpp11>

#include "dxil2spirv.h"

void dxil2spirv_optimize(Builder &spvBuilder, std::vector<uint32_t> &spirv);

extern "C" {
	const dxil2spirv_options *dxil2spirv_get_default_options();
}