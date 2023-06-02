#include <array>
#include <cppstacksize/base.h>
#include <cppstacksize/codeview-constants.h>
#include <string_view>

namespace cppstacksize {
namespace {
constexpr std::pair<U32, U8> type_to_size[] = {
    {T_VOID, 0},    //
    {T_CHAR, 1},    //
    {T_SHORT, 2},   //
    {T_LONG, 4},    //
    {T_QUAD, 8},    //
    {T_UCHAR, 1},   //
    {T_USHORT, 2},  //
    {T_ULONG, 4},   //
    {T_UQUAD, 8},   //
    {T_BOOL08, 1},  //
    {T_BOOL16, 2},  //
    {T_BOOL32, 4},  //
    {T_BOOL64, 8},  //
    {T_REAL32, 4},  //
    {T_REAL64, 8},  //
    {T_RCHAR, 1},   //
    {T_WCHAR, 4},   //
    {T_INT4, 4},    //
    {T_UINT4, 4},   //

    {T_PVOID, 8},  // FIXME(strager): sizeof(std::nullptr_t) is arch-dependent.
};

#define X_TYPE_TO_NAME               \
  X(T_VOID, u8"void")                \
  X(T_CHAR, u8"signed char")         \
  X(T_SHORT, u8"short")              \
  X(T_LONG, u8"long")                \
  X(T_QUAD, u8"long long")           \
  X(T_UCHAR, u8"unsigned char")      \
  X(T_USHORT, u8"unsigned short")    \
  X(T_ULONG, u8"unsigned long")      \
  X(T_UQUAD, u8"unsigned long long") \
  X(T_BOOL08, u8"bool")              \
  X(T_BOOL16, u8"bool(u16)")         \
  X(T_BOOL32, u8"bool(u32)")         \
  X(T_BOOL64, u8"bool(u64)")         \
  X(T_REAL32, u8"float")             \
  X(T_REAL64, u8"double")            \
  X(T_RCHAR, u8"char")               \
  X(T_WCHAR, u8"wchar_t")            \
  X(T_INT4, u8"int")                 \
  X(T_UINT4, u8"unsigned")           \
                                     \
  X(T_PVOID, u8"std::nullptr_t")

constexpr bool is_non_pointer_type(U32 type_id) { return type_id < 0x80; }

constexpr std::array<U8, 0x680> make_special_type_size_map() {
  std::array<U8, 0x680> size_map;
  for (U8& size : size_map) {
    size = 0xff;
  }
  for (auto [type_id, size] : type_to_size) {
    size_map[type_id] = size;
  }
#define X(type_id, _name)                     \
  if (is_non_pointer_type(type_id)) {         \
    U32 pointer_64_type_id = 0x600 | type_id; \
    size_map[pointer_64_type_id] = 8;         \
  }
  X_TYPE_TO_NAME
#undef X
  return size_map;
}

constexpr std::array<std::u8string_view, 0x680> make_special_type_name_map() {
  std::array<std::u8string_view, 0x680> name_map;
#define X(type_id, name)                        \
  name_map[type_id] = name;                     \
  if (is_non_pointer_type(type_id)) {           \
    U32 pointer_64_type_id = 0x600 | type_id;   \
    name_map[pointer_64_type_id] = name u8" *"; \
  }
  X_TYPE_TO_NAME
#undef X
  return name_map;
}
}

std::array<U8, 0x680> special_type_size_map = make_special_type_size_map();
std::array<std::u8string_view, 0x680> special_type_name_map =
    make_special_type_name_map();
}
