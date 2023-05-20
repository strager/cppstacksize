// CodeView signatures:
export let CV_SIGNATURE_C13 = 4;

// Subsection types:
export let DEBUG_S_SYMBOLS = 0xf1;

// Calling conventions:
export let CV_CALL_NEAR_C = 0x00;

// Type types:
export let LF_PROCEDURE = 0x1008;
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
export let T_NOTYPE = 0x0000;
export let T_ABS = 0x0001;
export let T_SEGMENT = 0x0002;
export let T_VOID = 0x0003;
export let T_CURRENCY = 0x0004;
export let T_NBASICSTR = 0x0005;
export let T_FBASICSTR = 0x0006;
export let T_NOTTRANS = 0x0007;
export let T_HRESULT = 0x0008;
export let T_CHAR = 0x0010;
export let T_SHORT = 0x0011;
export let T_LONG = 0x0012;
export let T_QUAD = 0x0013;
export let T_OCT = 0x0014;
export let T_UCHAR = 0x0020;
export let T_USHORT = 0x0021;
export let T_ULONG = 0x0022;
export let T_UQUAD = 0x0023;
export let T_UOCT = 0x0024;
export let T_BOOL08 = 0x0030;
export let T_BOOL16 = 0x0031;
export let T_BOOL32 = 0x0032;
export let T_BOOL64 = 0x0033;
export let T_REAL32 = 0x0040;
export let T_REAL64 = 0x0041;
export let T_REAL80 = 0x0042;
export let T_REAL128 = 0x0043;
export let T_REAL48 = 0x0044;
export let T_REAL32PP = 0x0045;
export let T_REAL16 = 0x0046;
export let T_CPLX32 = 0x0050;
export let T_CPLX64 = 0x0051;
export let T_CPLX80 = 0x0052;
export let T_CPLX128 = 0x0053;
export let T_BIT = 0x0060;
export let T_PASCHAR = 0x0061;
export let T_BOOL32FF = 0x0062;
export let T_INT1 = 0x0068;
export let T_UINT1 = 0x0069;
export let T_RCHAR = 0x0070;
export let T_WCHAR = 0x0071;
export let T_INT2 = 0x0072;
export let T_UINT2 = 0x0073;
export let T_INT4 = 0x0074;
export let T_UINT4 = 0x0075;
export let T_INT8 = 0x0076;
export let T_UINT8 = 0x0077;
export let T_INT16 = 0x0078;
export let T_UINT16 = 0x0079;
export let T_CHAR16 = 0x007a;
export let T_CHAR32 = 0x007b;

export let specialTypeSizeMap = {
  [T_NOTYPE]: "T_NOTYPE",
  [T_ABS]: "T_ABS",
  [T_SEGMENT]: "T_SEGMENT",
  [T_VOID]: "T_VOID",
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
};

export let specialTypeNameMap = {
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
};
