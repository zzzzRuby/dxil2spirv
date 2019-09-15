#pragma once

#include <cstdint>
#include <spirv/unified1/spirv.hpp11>

namespace dxil2spirv
{
	namespace spirv
	{
		class Handle
		{
		public:
			constexpr Handle() : _(0) {}
			Handle& operator++() { _++; return *this; }
			operator bool() const { return _; }
			operator uint32_t() const { return _; }
		private:
			uint32_t _;
		public:
			static const Handle Null;
		};

		static inline bool operator<(Handle a, Handle b) { return (uint32_t)a < (uint32_t)b; }
		static inline bool operator>(Handle a, Handle b) { return (uint32_t)a > (uint32_t)b; }
		static inline bool operator<=(Handle a, Handle b) { return (uint32_t)a <= (uint32_t)b; }
		static inline bool operator>=(Handle a, Handle b) { return (uint32_t)a >= (uint32_t)b; }
		static inline bool operator==(Handle a, Handle b) { return (uint32_t)a == (uint32_t)b; }
		static inline bool operator!=(Handle a, Handle b) { return (uint32_t)a != (uint32_t)b; }
	}
}