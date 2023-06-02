#pragma once

#include <array>
#include <cppstacksize/base.h>
#include <string_view>

namespace cppstacksize {
// CodeView signatures:
enum {
  CV_SIGNATURE_C13 = 4,
};

// Subsection types:
enum {
  DEBUG_S_SYMBOLS = 0xf1,
};

// Calling conventions:
enum {
  CV_CALL_NEAR_C = 0x00,
};

// Pointer types:
enum {
  CV_PTR_64 = 0xc,
};

// Type types:
enum {
  LF_MODIFIER = 0x1001,
  LF_POINTER = 0x1002,
  LF_PROCEDURE = 0x1008,
  LF_MFUNCTION = 0x1009,
  LF_ARRAY = 0x1503,
  LF_CLASS = 0x1504,
  LF_STRUCTURE = 0x1505,
  LF_UNION = 0x1506,
  LF_ENUM = 0x1507,
  LF_TYPESERVER2 = 0x1515,
  LF_FUNC_ID = 0x1601,
  LF_MFUNC_ID = 0x1602,
};

// Symbol types:
enum {
  S_END = 0x0006,
  S_BLOCK32 = 0x1103,
  S_FRAMEPROC = 0x1012,
  S_REGREL32 = 0x1111,
  S_GPROC32 = 0x1110,
  S_PROC_ID_END = 0x114f,
  S_GPROC32_ID = 0x1147,
};

// Special types:
enum {
  T_NOTYPE = 0x00,
  T_ABS = 0x01,
  T_SEGMENT = 0x02,
  T_VOID = 0x03,
  T_CURRENCY = 0x04,
  T_NBASICSTR = 0x05,
  T_FBASICSTR = 0x06,
  T_NOTTRANS = 0x07,
  T_HRESULT = 0x08,
  T_CHAR = 0x10,
  T_SHORT = 0x11,
  T_LONG = 0x12,
  T_QUAD = 0x13,
  T_OCT = 0x14,
  T_UCHAR = 0x20,
  T_USHORT = 0x21,
  T_ULONG = 0x22,
  T_UQUAD = 0x23,
  T_UOCT = 0x24,
  T_BOOL08 = 0x30,
  T_BOOL16 = 0x31,
  T_BOOL32 = 0x32,
  T_BOOL64 = 0x33,
  T_REAL32 = 0x40,
  T_REAL64 = 0x41,
  T_REAL80 = 0x42,
  T_REAL128 = 0x43,
  T_REAL48 = 0x44,
  T_REAL32PP = 0x45,
  T_REAL16 = 0x46,
  T_CPLX32 = 0x50,
  T_CPLX64 = 0x51,
  T_CPLX80 = 0x52,
  T_CPLX128 = 0x53,
  T_BIT = 0x60,
  T_PASCHAR = 0x61,
  T_BOOL32FF = 0x62,
  T_INT1 = 0x68,
  T_UINT1 = 0x69,
  T_RCHAR = 0x70,
  T_WCHAR = 0x71,
  T_INT2 = 0x72,
  T_UINT2 = 0x73,
  T_INT4 = 0x74,
  T_UINT4 = 0x75,
  T_INT8 = 0x76,
  T_UINT8 = 0x77,
  T_INT16 = 0x78,
  T_UINT16 = 0x79,
  T_CHAR16 = 0x7a,
  T_CHAR32 = 0x7b,
  T_PVOID = 0x0103,
};

extern std::array<U8, 0x680> special_type_size_map;
extern std::array<std::u8string_view, 0x680> special_type_name_map;
}
