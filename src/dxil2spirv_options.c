#include <dxil2spirv.h>

static dxil2spirv_options dxil2spirv_default_options = {
    .enableOptimizer = true, 
	.stripName = false
};

const dxil2spirv_options *dxil2spirv_get_default_options() {
  return &dxil2spirv_default_options;
}