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
export let T_PVOID = 0x0103;
export let T_PCHAR = 0x0110;
export let T_PSHORT = 0x0111;
export let T_PLONG = 0x0112;
export let T_PQUAD = 0x0113;
export let T_POCT = 0x0114;
export let T_PUCHAR = 0x0120;
export let T_PUSHORT = 0x0121;
export let T_PULONG = 0x0122;
export let T_PUQUAD = 0x0123;
export let T_PUOCT = 0x0124;
export let T_PBOOL08 = 0x0130;
export let T_PBOOL16 = 0x0131;
export let T_PBOOL32 = 0x0132;
export let T_PBOOL64 = 0x0133;
export let T_PREAL32 = 0x0140;
export let T_PREAL64 = 0x0141;
export let T_PREAL80 = 0x0142;
export let T_PREAL128 = 0x0143;
export let T_PREAL48 = 0x0144;
export let T_PREAL32PP = 0x0145;
export let T_PREAL16 = 0x0146;
export let T_PCPLX32 = 0x0150;
export let T_PCPLX64 = 0x0151;
export let T_PCPLX80 = 0x0152;
export let T_PCPLX128 = 0x0153;
export let T_PINT1 = 0x0168;
export let T_PUINT1 = 0x0169;
export let T_PRCHAR = 0x0170;
export let T_PWCHAR = 0x0171;
export let T_PINT2 = 0x0172;
export let T_PUINT2 = 0x0173;
export let T_PINT4 = 0x0174;
export let T_PUINT4 = 0x0175;
export let T_PINT8 = 0x0176;
export let T_PUINT8 = 0x0177;
export let T_PINT16 = 0x0178;
export let T_PUINT16 = 0x0179;
export let T_PCHAR16 = 0x017a;
export let T_PCHAR32 = 0x017b;
export let T_NCVPTR = 0x01f0;
export let T_PFVOID = 0x0203;
export let T_PFCHAR = 0x0210;
export let T_PFSHORT = 0x0211;
export let T_PFLONG = 0x0212;
export let T_PFQUAD = 0x0213;
export let T_PFOCT = 0x0214;
export let T_PFUCHAR = 0x0220;
export let T_PFUSHORT = 0x0221;
export let T_PFULONG = 0x0222;
export let T_PFUQUAD = 0x0223;
export let T_PFUOCT = 0x0224;
export let T_PFBOOL08 = 0x0230;
export let T_PFBOOL16 = 0x0231;
export let T_PFBOOL32 = 0x0232;
export let T_PFBOOL64 = 0x0233;
export let T_PFREAL32 = 0x0240;
export let T_PFREAL64 = 0x0241;
export let T_PFREAL80 = 0x0242;
export let T_PFREAL128 = 0x0243;
export let T_PFREAL48 = 0x0244;
export let T_PFREAL32PP = 0x0245;
export let T_PFREAL16 = 0x0246;
export let T_PFCPLX32 = 0x0250;
export let T_PFCPLX64 = 0x0251;
export let T_PFCPLX80 = 0x0252;
export let T_PFCPLX128 = 0x0253;
export let T_PFINT1 = 0x0268;
export let T_PFUINT1 = 0x0269;
export let T_PFRCHAR = 0x0270;
export let T_PFWCHAR = 0x0271;
export let T_PFINT2 = 0x0272;
export let T_PFUINT2 = 0x0273;
export let T_PFINT4 = 0x0274;
export let T_PFUINT4 = 0x0275;
export let T_PFINT8 = 0x0276;
export let T_PFUINT8 = 0x0277;
export let T_PFINT16 = 0x0278;
export let T_PFUINT16 = 0x0279;
export let T_PFCHAR16 = 0x027a;
export let T_PFCHAR32 = 0x027b;
export let T_FCVPTR = 0x02f0;
export let T_PHVOID = 0x0303;
export let T_PHCHAR = 0x0310;
export let T_PHSHORT = 0x0311;
export let T_PHLONG = 0x0312;
export let T_PHQUAD = 0x0313;
export let T_PHOCT = 0x0314;
export let T_PHUCHAR = 0x0320;
export let T_PHUSHORT = 0x0321;
export let T_PHULONG = 0x0322;
export let T_PHUQUAD = 0x0323;
export let T_PHUOCT = 0x0324;
export let T_PHBOOL08 = 0x0330;
export let T_PHBOOL16 = 0x0331;
export let T_PHBOOL32 = 0x0332;
export let T_PHBOOL64 = 0x0333;
export let T_PHREAL32 = 0x0340;
export let T_PHREAL64 = 0x0341;
export let T_PHREAL80 = 0x0342;
export let T_PHREAL128 = 0x0343;
export let T_PHREAL48 = 0x0344;
export let T_PHREAL32PP = 0x0345;
export let T_PHREAL16 = 0x0346;
export let T_PHCPLX32 = 0x0350;
export let T_PHCPLX64 = 0x0351;
export let T_PHCPLX80 = 0x0352;
export let T_PHCPLX128 = 0x0353;
export let T_PHINT1 = 0x0368;
export let T_PHUINT1 = 0x0369;
export let T_PHRCHAR = 0x0370;
export let T_PHWCHAR = 0x0371;
export let T_PHINT2 = 0x0372;
export let T_PHUINT2 = 0x0373;
export let T_PHINT4 = 0x0374;
export let T_PHUINT4 = 0x0375;
export let T_PHINT8 = 0x0376;
export let T_PHUINT8 = 0x0377;
export let T_PHINT16 = 0x0378;
export let T_PHUINT16 = 0x0379;
export let T_PHCHAR16 = 0x037a;
export let T_PHCHAR32 = 0x037b;
export let T_HCVPTR = 0x03f0;
export let T_32PVOID = 0x0403;
export let T_32PHRESULT = 0x0408;
export let T_32PCHAR = 0x0410;
export let T_32PSHORT = 0x0411;
export let T_32PLONG = 0x0412;
export let T_32PQUAD = 0x0413;
export let T_32POCT = 0x0414;
export let T_32PUCHAR = 0x0420;
export let T_32PUSHORT = 0x0421;
export let T_32PULONG = 0x0422;
export let T_32PUQUAD = 0x0423;
export let T_32PUOCT = 0x0424;
export let T_32PBOOL08 = 0x0430;
export let T_32PBOOL16 = 0x0431;
export let T_32PBOOL32 = 0x0432;
export let T_32PBOOL64 = 0x0433;
export let T_32PREAL32 = 0x0440;
export let T_32PREAL64 = 0x0441;
export let T_32PREAL80 = 0x0442;
export let T_32PREAL128 = 0x0443;
export let T_32PREAL48 = 0x0444;
export let T_32PREAL32PP = 0x0445;
export let T_32PREAL16 = 0x0446;
export let T_32PCPLX32 = 0x0450;
export let T_32PCPLX64 = 0x0451;
export let T_32PCPLX80 = 0x0452;
export let T_32PCPLX128 = 0x0453;
export let T_32PINT1 = 0x0468;
export let T_32PUINT1 = 0x0469;
export let T_32PRCHAR = 0x0470;
export let T_32PWCHAR = 0x0471;
export let T_32PINT2 = 0x0472;
export let T_32PUINT2 = 0x0473;
export let T_32PINT4 = 0x0474;
export let T_32PUINT4 = 0x0475;
export let T_32PINT8 = 0x0476;
export let T_32PUINT8 = 0x0477;
export let T_32PINT16 = 0x0478;
export let T_32PUINT16 = 0x0479;
export let T_32PCHAR16 = 0x047a;
export let T_32PCHAR32 = 0x047b;
export let T_32NCVPTR = 0x04f0;
export let T_32PFVOID = 0x0503;
export let T_32PFCHAR = 0x0510;
export let T_32PFSHORT = 0x0511;
export let T_32PFLONG = 0x0512;
export let T_32PFQUAD = 0x0513;
export let T_32PFOCT = 0x0514;
export let T_32PFUCHAR = 0x0520;
export let T_32PFUSHORT = 0x0521;
export let T_32PFULONG = 0x0522;
export let T_32PFUQUAD = 0x0523;
export let T_32PFUOCT = 0x0524;
export let T_32PFBOOL08 = 0x0530;
export let T_32PFBOOL16 = 0x0531;
export let T_32PFBOOL32 = 0x0532;
export let T_32PFBOOL64 = 0x0533;
export let T_32PFREAL32 = 0x0540;
export let T_32PFREAL64 = 0x0541;
export let T_32PFREAL80 = 0x0542;
export let T_32PFREAL128 = 0x0543;
export let T_32PFREAL48 = 0x0544;
export let T_32PFREAL32PP = 0x0545;
export let T_32PFREAL16 = 0x0546;
export let T_32PFCPLX32 = 0x0550;
export let T_32PFCPLX64 = 0x0551;
export let T_32PFCPLX80 = 0x0552;
export let T_32PFCPLX128 = 0x0553;
export let T_32PFINT1 = 0x0568;
export let T_32PFUINT1 = 0x0569;
export let T_32PFRCHAR = 0x0570;
export let T_32PFWCHAR = 0x0571;
export let T_32PFINT2 = 0x0572;
export let T_32PFUINT2 = 0x0573;
export let T_32PFINT4 = 0x0574;
export let T_32PFUINT4 = 0x0575;
export let T_32PFINT8 = 0x0576;
export let T_32PFUINT8 = 0x0577;
export let T_32PFINT16 = 0x0578;
export let T_32PFUINT16 = 0x0579;
export let T_32PFCHAR16 = 0x057a;
export let T_32PFCHAR32 = 0x057b;
export let T_32FCVPTR = 0x05f0;
export let T_64PVOID = 0x0603;
export let T_64PHRESULT = 0x0608;
export let T_64PCHAR = 0x0610;
export let T_64PSHORT = 0x0611;
export let T_64PLONG = 0x0612;
export let T_64PQUAD = 0x0613;
export let T_64POCT = 0x0614;
export let T_64PUCHAR = 0x0620;
export let T_64PUSHORT = 0x0621;
export let T_64PULONG = 0x0622;
export let T_64PUQUAD = 0x0623;
export let T_64PUOCT = 0x0624;
export let T_64PBOOL08 = 0x0630;
export let T_64PBOOL16 = 0x0631;
export let T_64PBOOL32 = 0x0632;
export let T_64PBOOL64 = 0x0633;
export let T_64PREAL32 = 0x0640;
export let T_64PREAL64 = 0x0641;
export let T_64PREAL80 = 0x0642;
export let T_64PREAL128 = 0x0643;
export let T_64PREAL48 = 0x0644;
export let T_64PREAL32PP = 0x0645;
export let T_64PREAL16 = 0x0646;
export let T_64PCPLX32 = 0x0650;
export let T_64PCPLX64 = 0x0651;
export let T_64PCPLX80 = 0x0652;
export let T_64PCPLX128 = 0x0653;
export let T_64PINT1 = 0x0668;
export let T_64PUINT1 = 0x0669;
export let T_64PRCHAR = 0x0670;
export let T_64PWCHAR = 0x0671;
export let T_64PINT2 = 0x0672;
export let T_64PUINT2 = 0x0673;
export let T_64PINT4 = 0x0674;
export let T_64PUINT4 = 0x0675;
export let T_64PINT8 = 0x0676;
export let T_64PUINT8 = 0x0677;
export let T_64PINT16 = 0x0678;
export let T_64PUINT16 = 0x0679;
export let T_64PCHAR16 = 0x067a;
export let T_64PCHAR32 = 0x067b;
export let T_64NCVPTR = 0x06f0;

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
  [T_PVOID]: "T_PVOID",
  [T_PCHAR]: "T_PCHAR",
  [T_PSHORT]: "T_PSHORT",
  [T_PLONG]: "T_PLONG",
  [T_PQUAD]: "T_PQUAD",
  [T_POCT]: "T_POCT",
  [T_PUCHAR]: "T_PUCHAR",
  [T_PUSHORT]: "T_PUSHORT",
  [T_PULONG]: "T_PULONG",
  [T_PUQUAD]: "T_PUQUAD",
  [T_PUOCT]: "T_PUOCT",
  [T_PBOOL08]: "T_PBOOL08",
  [T_PBOOL16]: "T_PBOOL16",
  [T_PBOOL32]: "T_PBOOL32",
  [T_PBOOL64]: "T_PBOOL64",
  [T_PREAL32]: "T_PREAL32",
  [T_PREAL64]: "T_PREAL64",
  [T_PREAL80]: "T_PREAL80",
  [T_PREAL128]: "T_PREAL128",
  [T_PREAL48]: "T_PREAL48",
  [T_PREAL32PP]: "T_PREAL32PP",
  [T_PREAL16]: "T_PREAL16",
  [T_PCPLX32]: "T_PCPLX32",
  [T_PCPLX64]: "T_PCPLX64",
  [T_PCPLX80]: "T_PCPLX80",
  [T_PCPLX128]: "T_PCPLX128",
  [T_PINT1]: "T_PINT1",
  [T_PUINT1]: "T_PUINT1",
  [T_PRCHAR]: "T_PRCHAR",
  [T_PWCHAR]: "T_PWCHAR",
  [T_PINT2]: "T_PINT2",
  [T_PUINT2]: "T_PUINT2",
  [T_PINT4]: "T_PINT4",
  [T_PUINT4]: "T_PUINT4",
  [T_PINT8]: "T_PINT8",
  [T_PUINT8]: "T_PUINT8",
  [T_PINT16]: "T_PINT16",
  [T_PUINT16]: "T_PUINT16",
  [T_PCHAR16]: "T_PCHAR16",
  [T_PCHAR32]: "T_PCHAR32",
  [T_NCVPTR]: "T_NCVPTR",
  [T_PFVOID]: "T_PFVOID",
  [T_PFCHAR]: "T_PFCHAR",
  [T_PFSHORT]: "T_PFSHORT",
  [T_PFLONG]: "T_PFLONG",
  [T_PFQUAD]: "T_PFQUAD",
  [T_PFOCT]: "T_PFOCT",
  [T_PFUCHAR]: "T_PFUCHAR",
  [T_PFUSHORT]: "T_PFUSHORT",
  [T_PFULONG]: "T_PFULONG",
  [T_PFUQUAD]: "T_PFUQUAD",
  [T_PFUOCT]: "T_PFUOCT",
  [T_PFBOOL08]: "T_PFBOOL08",
  [T_PFBOOL16]: "T_PFBOOL16",
  [T_PFBOOL32]: "T_PFBOOL32",
  [T_PFBOOL64]: "T_PFBOOL64",
  [T_PFREAL32]: "T_PFREAL32",
  [T_PFREAL64]: "T_PFREAL64",
  [T_PFREAL80]: "T_PFREAL80",
  [T_PFREAL128]: "T_PFREAL128",
  [T_PFREAL48]: "T_PFREAL48",
  [T_PFREAL32PP]: "T_PFREAL32PP",
  [T_PFREAL16]: "T_PFREAL16",
  [T_PFCPLX32]: "T_PFCPLX32",
  [T_PFCPLX64]: "T_PFCPLX64",
  [T_PFCPLX80]: "T_PFCPLX80",
  [T_PFCPLX128]: "T_PFCPLX128",
  [T_PFINT1]: "T_PFINT1",
  [T_PFUINT1]: "T_PFUINT1",
  [T_PFRCHAR]: "T_PFRCHAR",
  [T_PFWCHAR]: "T_PFWCHAR",
  [T_PFINT2]: "T_PFINT2",
  [T_PFUINT2]: "T_PFUINT2",
  [T_PFINT4]: "T_PFINT4",
  [T_PFUINT4]: "T_PFUINT4",
  [T_PFINT8]: "T_PFINT8",
  [T_PFUINT8]: "T_PFUINT8",
  [T_PFINT16]: "T_PFINT16",
  [T_PFUINT16]: "T_PFUINT16",
  [T_PFCHAR16]: "T_PFCHAR16",
  [T_PFCHAR32]: "T_PFCHAR32",
  [T_FCVPTR]: "T_FCVPTR",
  [T_PHVOID]: "T_PHVOID",
  [T_PHCHAR]: "T_PHCHAR",
  [T_PHSHORT]: "T_PHSHORT",
  [T_PHLONG]: "T_PHLONG",
  [T_PHQUAD]: "T_PHQUAD",
  [T_PHOCT]: "T_PHOCT",
  [T_PHUCHAR]: "T_PHUCHAR",
  [T_PHUSHORT]: "T_PHUSHORT",
  [T_PHULONG]: "T_PHULONG",
  [T_PHUQUAD]: "T_PHUQUAD",
  [T_PHUOCT]: "T_PHUOCT",
  [T_PHBOOL08]: "T_PHBOOL08",
  [T_PHBOOL16]: "T_PHBOOL16",
  [T_PHBOOL32]: "T_PHBOOL32",
  [T_PHBOOL64]: "T_PHBOOL64",
  [T_PHREAL32]: "T_PHREAL32",
  [T_PHREAL64]: "T_PHREAL64",
  [T_PHREAL80]: "T_PHREAL80",
  [T_PHREAL128]: "T_PHREAL128",
  [T_PHREAL48]: "T_PHREAL48",
  [T_PHREAL32PP]: "T_PHREAL32PP",
  [T_PHREAL16]: "T_PHREAL16",
  [T_PHCPLX32]: "T_PHCPLX32",
  [T_PHCPLX64]: "T_PHCPLX64",
  [T_PHCPLX80]: "T_PHCPLX80",
  [T_PHCPLX128]: "T_PHCPLX128",
  [T_PHINT1]: "T_PHINT1",
  [T_PHUINT1]: "T_PHUINT1",
  [T_PHRCHAR]: "T_PHRCHAR",
  [T_PHWCHAR]: "T_PHWCHAR",
  [T_PHINT2]: "T_PHINT2",
  [T_PHUINT2]: "T_PHUINT2",
  [T_PHINT4]: "T_PHINT4",
  [T_PHUINT4]: "T_PHUINT4",
  [T_PHINT8]: "T_PHINT8",
  [T_PHUINT8]: "T_PHUINT8",
  [T_PHINT16]: "T_PHINT16",
  [T_PHUINT16]: "T_PHUINT16",
  [T_PHCHAR16]: "T_PHCHAR16",
  [T_PHCHAR32]: "T_PHCHAR32",
  [T_HCVPTR]: "T_HCVPTR",
  [T_32PVOID]: "T_32PVOID",
  [T_32PHRESULT]: "T_32PHRESULT",
  [T_32PCHAR]: "T_32PCHAR",
  [T_32PSHORT]: "T_32PSHORT",
  [T_32PLONG]: "T_32PLONG",
  [T_32PQUAD]: "T_32PQUAD",
  [T_32POCT]: "T_32POCT",
  [T_32PUCHAR]: "T_32PUCHAR",
  [T_32PUSHORT]: "T_32PUSHORT",
  [T_32PULONG]: "T_32PULONG",
  [T_32PUQUAD]: "T_32PUQUAD",
  [T_32PUOCT]: "T_32PUOCT",
  [T_32PBOOL08]: "T_32PBOOL08",
  [T_32PBOOL16]: "T_32PBOOL16",
  [T_32PBOOL32]: "T_32PBOOL32",
  [T_32PBOOL64]: "T_32PBOOL64",
  [T_32PREAL32]: "T_32PREAL32",
  [T_32PREAL64]: "T_32PREAL64",
  [T_32PREAL80]: "T_32PREAL80",
  [T_32PREAL128]: "T_32PREAL128",
  [T_32PREAL48]: "T_32PREAL48",
  [T_32PREAL32PP]: "T_32PREAL32PP",
  [T_32PREAL16]: "T_32PREAL16",
  [T_32PCPLX32]: "T_32PCPLX32",
  [T_32PCPLX64]: "T_32PCPLX64",
  [T_32PCPLX80]: "T_32PCPLX80",
  [T_32PCPLX128]: "T_32PCPLX128",
  [T_32PINT1]: "T_32PINT1",
  [T_32PUINT1]: "T_32PUINT1",
  [T_32PRCHAR]: "T_32PRCHAR",
  [T_32PWCHAR]: "T_32PWCHAR",
  [T_32PINT2]: "T_32PINT2",
  [T_32PUINT2]: "T_32PUINT2",
  [T_32PINT4]: "T_32PINT4",
  [T_32PUINT4]: "T_32PUINT4",
  [T_32PINT8]: "T_32PINT8",
  [T_32PUINT8]: "T_32PUINT8",
  [T_32PINT16]: "T_32PINT16",
  [T_32PUINT16]: "T_32PUINT16",
  [T_32PCHAR16]: "T_32PCHAR16",
  [T_32PCHAR32]: "T_32PCHAR32",
  [T_32NCVPTR]: "T_32NCVPTR",
  [T_32PFVOID]: "T_32PFVOID",
  [T_32PFCHAR]: "T_32PFCHAR",
  [T_32PFSHORT]: "T_32PFSHORT",
  [T_32PFLONG]: "T_32PFLONG",
  [T_32PFQUAD]: "T_32PFQUAD",
  [T_32PFOCT]: "T_32PFOCT",
  [T_32PFUCHAR]: "T_32PFUCHAR",
  [T_32PFUSHORT]: "T_32PFUSHORT",
  [T_32PFULONG]: "T_32PFULONG",
  [T_32PFUQUAD]: "T_32PFUQUAD",
  [T_32PFUOCT]: "T_32PFUOCT",
  [T_32PFBOOL08]: "T_32PFBOOL08",
  [T_32PFBOOL16]: "T_32PFBOOL16",
  [T_32PFBOOL32]: "T_32PFBOOL32",
  [T_32PFBOOL64]: "T_32PFBOOL64",
  [T_32PFREAL32]: "T_32PFREAL32",
  [T_32PFREAL64]: "T_32PFREAL64",
  [T_32PFREAL80]: "T_32PFREAL80",
  [T_32PFREAL128]: "T_32PFREAL128",
  [T_32PFREAL48]: "T_32PFREAL48",
  [T_32PFREAL32PP]: "T_32PFREAL32PP",
  [T_32PFREAL16]: "T_32PFREAL16",
  [T_32PFCPLX32]: "T_32PFCPLX32",
  [T_32PFCPLX64]: "T_32PFCPLX64",
  [T_32PFCPLX80]: "T_32PFCPLX80",
  [T_32PFCPLX128]: "T_32PFCPLX128",
  [T_32PFINT1]: "T_32PFINT1",
  [T_32PFUINT1]: "T_32PFUINT1",
  [T_32PFRCHAR]: "T_32PFRCHAR",
  [T_32PFWCHAR]: "T_32PFWCHAR",
  [T_32PFINT2]: "T_32PFINT2",
  [T_32PFUINT2]: "T_32PFUINT2",
  [T_32PFINT4]: "T_32PFINT4",
  [T_32PFUINT4]: "T_32PFUINT4",
  [T_32PFINT8]: "T_32PFINT8",
  [T_32PFUINT8]: "T_32PFUINT8",
  [T_32PFINT16]: "T_32PFINT16",
  [T_32PFUINT16]: "T_32PFUINT16",
  [T_32PFCHAR16]: "T_32PFCHAR16",
  [T_32PFCHAR32]: "T_32PFCHAR32",
  [T_32FCVPTR]: "T_32FCVPTR",
  [T_64PVOID]: "T_64PVOID",
  [T_64PHRESULT]: "T_64PHRESULT",
  [T_64PCHAR]: "T_64PCHAR",
  [T_64PSHORT]: "T_64PSHORT",
  [T_64PLONG]: "T_64PLONG",
  [T_64PQUAD]: "T_64PQUAD",
  [T_64POCT]: "T_64POCT",
  [T_64PUCHAR]: "T_64PUCHAR",
  [T_64PUSHORT]: "T_64PUSHORT",
  [T_64PULONG]: "T_64PULONG",
  [T_64PUQUAD]: "T_64PUQUAD",
  [T_64PUOCT]: "T_64PUOCT",
  [T_64PBOOL08]: "T_64PBOOL08",
  [T_64PBOOL16]: "T_64PBOOL16",
  [T_64PBOOL32]: "T_64PBOOL32",
  [T_64PBOOL64]: "T_64PBOOL64",
  [T_64PREAL32]: "T_64PREAL32",
  [T_64PREAL64]: "T_64PREAL64",
  [T_64PREAL80]: "T_64PREAL80",
  [T_64PREAL128]: "T_64PREAL128",
  [T_64PREAL48]: "T_64PREAL48",
  [T_64PREAL32PP]: "T_64PREAL32PP",
  [T_64PREAL16]: "T_64PREAL16",
  [T_64PCPLX32]: "T_64PCPLX32",
  [T_64PCPLX64]: "T_64PCPLX64",
  [T_64PCPLX80]: "T_64PCPLX80",
  [T_64PCPLX128]: "T_64PCPLX128",
  [T_64PINT1]: "T_64PINT1",
  [T_64PUINT1]: "T_64PUINT1",
  [T_64PRCHAR]: "T_64PRCHAR",
  [T_64PWCHAR]: "T_64PWCHAR",
  [T_64PINT2]: "T_64PINT2",
  [T_64PUINT2]: "T_64PUINT2",
  [T_64PINT4]: "T_64PINT4",
  [T_64PUINT4]: "T_64PUINT4",
  [T_64PINT8]: "T_64PINT8",
  [T_64PUINT8]: "T_64PUINT8",
  [T_64PINT16]: "T_64PINT16",
  [T_64PUINT16]: "T_64PUINT16",
  [T_64PCHAR16]: "T_64PCHAR16",
  [T_64PCHAR32]: "T_64PCHAR32",
  [T_64NCVPTR]: "T_64NCVPTR",
};
