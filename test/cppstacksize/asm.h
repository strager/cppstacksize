#ifndef CPPSTACKSIZE_ASM_H
#define CPPSTACKSIZE_ASM_H

#include <cppstacksize/base.h>
#include <span>

#define ASM_X86_64(assembly) (assemble_x86_64((assembly)))

namespace cppstacksize {
std::span<const U8> assemble_x86_64(const char* assembly);
}

#endif
