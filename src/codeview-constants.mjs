// CodeView signatures:
export let CV_SIGNATURE_C13 = 4;

// Subsection types:
export let DEBUG_S_SYMBOLS = 0xf1;

// Calling conventions:
export let CV_CALL_NEAR_C = 0x00;

// Pointer types:
export let CV_PTR_64 = 0xc;

// Type types:
export let LF_MODIFIER = 0x1001;
export let LF_POINTER = 0x1002;
export let LF_PROCEDURE = 0x1008;
export let LF_STRUCTURE = 0x1505;
export let LF_ENUM = 0x1507;
export let LF_TYPESERVER2 = 0x1515;
export let LF_FUNC_ID = 0x1601;

// Symbol types:
export let S_END = 0x0006;
export let S_FRAMEPROC = 0x1012;
export let S_REGREL32 = 0x1111;
export let S_GPROC32 = 0x1110;
export let S_PROC_ID_END = 0x114f;
export let S_GPROC32_ID = 0x1147;

// Special types:
export let T_NOTYPE = 0x00;
export let T_ABS = 0x01;
export let T_SEGMENT = 0x02;
export let T_VOID = 0x03;
export let T_CURRENCY = 0x04;
export let T_NBASICSTR = 0x05;
export let T_FBASICSTR = 0x06;
export let T_NOTTRANS = 0x07;
export let T_HRESULT = 0x08;
export let T_CHAR = 0x10;
export let T_SHORT = 0x11;
export let T_LONG = 0x12;
export let T_QUAD = 0x13;
export let T_OCT = 0x14;
export let T_UCHAR = 0x20;
export let T_USHORT = 0x21;
export let T_ULONG = 0x22;
export let T_UQUAD = 0x23;
export let T_UOCT = 0x24;
export let T_BOOL08 = 0x30;
export let T_BOOL16 = 0x31;
export let T_BOOL32 = 0x32;
export let T_BOOL64 = 0x33;
export let T_REAL32 = 0x40;
export let T_REAL64 = 0x41;
export let T_REAL80 = 0x42;
export let T_REAL128 = 0x43;
export let T_REAL48 = 0x44;
export let T_REAL32PP = 0x45;
export let T_REAL16 = 0x46;
export let T_CPLX32 = 0x50;
export let T_CPLX64 = 0x51;
export let T_CPLX80 = 0x52;
export let T_CPLX128 = 0x53;
export let T_BIT = 0x60;
export let T_PASCHAR = 0x61;
export let T_BOOL32FF = 0x62;
export let T_INT1 = 0x68;
export let T_UINT1 = 0x69;
export let T_RCHAR = 0x70;
export let T_WCHAR = 0x71;
export let T_INT2 = 0x72;
export let T_UINT2 = 0x73;
export let T_INT4 = 0x74;
export let T_UINT4 = 0x75;
export let T_INT8 = 0x76;
export let T_UINT8 = 0x77;
export let T_INT16 = 0x78;
export let T_UINT16 = 0x79;
export let T_CHAR16 = 0x7a;
export let T_CHAR32 = 0x7b;
export let T_PVOID = 0x0103;

export let specialTypeSizeMap = {
  [T_NOTYPE]: "T_NOTYPE",
  [T_ABS]: "T_ABS",
  [T_SEGMENT]: "T_SEGMENT",
  [T_VOID]: 0,
  [T_CURRENCY]: "T_CURRENCY",
  [T_NBASICSTR]: "T_NBASICSTR",
  [T_FBASICSTR]: "T_FBASICSTR",
  [T_NOTTRANS]: "T_NOTTRANS",
  [T_HRESULT]: "T_HRESULT",
  [T_CHAR]: 1,
  [T_SHORT]: 2,
  [T_LONG]: 4,
  [T_QUAD]: 8,
  [T_OCT]: "T_OCT",
  [T_UCHAR]: 1,
  [T_USHORT]: 2,
  [T_ULONG]: 4,
  [T_UQUAD]: 8,
  [T_UOCT]: "T_UOCT",
  [T_BOOL08]: "T_BOOL08",
  [T_BOOL16]: "T_BOOL16",
  [T_BOOL32]: "T_BOOL32",
  [T_BOOL64]: "T_BOOL64",
  [T_REAL32]: 4,
  [T_REAL64]: 8,
  [T_REAL80]: "T_REAL80",
  [T_REAL128]: "T_REAL128",
  [T_REAL48]: "T_REAL48",
  [T_REAL32PP]: "T_REAL32PP",
  [T_REAL16]: "T_REAL16",
  [T_CPLX32]: "T_CPLX32",
  [T_CPLX64]: "T_CPLX64",
  [T_CPLX80]: "T_CPLX80",
  [T_CPLX128]: "T_CPLX128",
  [T_BIT]: "T_BIT",
  [T_PASCHAR]: "T_PASCHAR",
  [T_BOOL32FF]: "T_BOOL32FF",
  [T_INT1]: "T_INT1",
  [T_UINT1]: "T_UINT1",
  [T_RCHAR]: 1,
  [T_WCHAR]: 4,
  [T_INT2]: "T_INT2",
  [T_UINT2]: "T_UINT2",
  [T_INT4]: 4,
  [T_UINT4]: 4,
  [T_INT8]: "T_INT8",
  [T_UINT8]: "T_UINT8",
  [T_INT16]: "T_INT16",
  [T_UINT16]: "T_UINT16",
  [T_CHAR16]: "T_CHAR16",
  [T_CHAR32]: "T_CHAR32",

  [T_PVOID]: 8, // FIXME(strager): sizeof(std::nullptr_t) is arch-dependent.
};

export let specialTypeNameMap = {
  [T_VOID]: "void",
  [T_CHAR]: "signed char",
  [T_SHORT]: "short",
  [T_LONG]: "long",
  [T_QUAD]: "long long",
  [T_UCHAR]: "unsigned char",
  [T_USHORT]: "unsigned short",
  [T_ULONG]: "unsigned long",
  [T_UQUAD]: "unsigned long long",
  [T_REAL32]: "float",
  [T_REAL64]: "double",
  [T_RCHAR]: "char",
  [T_WCHAR]: "wchar_t",
  [T_INT4]: "int",
  [T_UINT4]: "unsigned",

  [T_PVOID]: "std::nullptr_t",
};

for (let typeID in specialTypeNameMap) {
  typeID = +typeID;
  let isNonPointerType = typeID < 0x80;
  if (isNonPointerType) {
    let pointer64TypeID = 0x600 | typeID;
    specialTypeSizeMap[pointer64TypeID] = 8;
    specialTypeNameMap[pointer64TypeID] = `${specialTypeNameMap[typeID]} *`;
  }
}
