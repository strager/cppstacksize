import { SubFileReader } from "../src/reader.mjs";

// CodeView signatures:
let CV_SIGNATURE_C13 = 4;

// Subsection types:
let DEBUG_S_SYMBOLS = 0xf1;

// Symbol types:
let S_REGREL32 = 0x1111;
let S_GPROC32_ID = 0x1147;

// Special types:
let T_NOTYPE = 0x0000;
let T_ABS = 0x0001;
let T_SEGMENT = 0x0002;
let T_VOID = 0x0003;
let T_CURRENCY = 0x0004;
let T_NBASICSTR = 0x0005;
let T_FBASICSTR = 0x0006;
let T_NOTTRANS = 0x0007;
let T_HRESULT = 0x0008;
let T_CHAR = 0x0010;
let T_SHORT = 0x0011;
let T_LONG = 0x0012;
let T_QUAD = 0x0013;
let T_OCT = 0x0014;
let T_UCHAR = 0x0020;
let T_USHORT = 0x0021;
let T_ULONG = 0x0022;
let T_UQUAD = 0x0023;
let T_UOCT = 0x0024;
let T_BOOL08 = 0x0030;
let T_BOOL16 = 0x0031;
let T_BOOL32 = 0x0032;
let T_BOOL64 = 0x0033;
let T_REAL32 = 0x0040;
let T_REAL64 = 0x0041;
let T_REAL80 = 0x0042;
let T_REAL128 = 0x0043;
let T_REAL48 = 0x0044;
let T_REAL32PP = 0x0045;
let T_REAL16 = 0x0046;
let T_CPLX32 = 0x0050;
let T_CPLX64 = 0x0051;
let T_CPLX80 = 0x0052;
let T_CPLX128 = 0x0053;
let T_BIT = 0x0060;
let T_PASCHAR = 0x0061;
let T_BOOL32FF = 0x0062;
let T_INT1 = 0x0068;
let T_UINT1 = 0x0069;
let T_RCHAR = 0x0070;
let T_WCHAR = 0x0071;
let T_INT2 = 0x0072;
let T_UINT2 = 0x0073;
let T_INT4 = 0x0074;
let T_UINT4 = 0x0075;
let T_INT8 = 0x0076;
let T_UINT8 = 0x0077;
let T_INT16 = 0x0078;
let T_UINT16 = 0x0079;
let T_CHAR16 = 0x007a;
let T_CHAR32 = 0x007b;
let T_PVOID = 0x0103;
let T_PCHAR = 0x0110;
let T_PSHORT = 0x0111;
let T_PLONG = 0x0112;
let T_PQUAD = 0x0113;
let T_POCT = 0x0114;
let T_PUCHAR = 0x0120;
let T_PUSHORT = 0x0121;
let T_PULONG = 0x0122;
let T_PUQUAD = 0x0123;
let T_PUOCT = 0x0124;
let T_PBOOL08 = 0x0130;
let T_PBOOL16 = 0x0131;
let T_PBOOL32 = 0x0132;
let T_PBOOL64 = 0x0133;
let T_PREAL32 = 0x0140;
let T_PREAL64 = 0x0141;
let T_PREAL80 = 0x0142;
let T_PREAL128 = 0x0143;
let T_PREAL48 = 0x0144;
let T_PREAL32PP = 0x0145;
let T_PREAL16 = 0x0146;
let T_PCPLX32 = 0x0150;
let T_PCPLX64 = 0x0151;
let T_PCPLX80 = 0x0152;
let T_PCPLX128 = 0x0153;
let T_PINT1 = 0x0168;
let T_PUINT1 = 0x0169;
let T_PRCHAR = 0x0170;
let T_PWCHAR = 0x0171;
let T_PINT2 = 0x0172;
let T_PUINT2 = 0x0173;
let T_PINT4 = 0x0174;
let T_PUINT4 = 0x0175;
let T_PINT8 = 0x0176;
let T_PUINT8 = 0x0177;
let T_PINT16 = 0x0178;
let T_PUINT16 = 0x0179;
let T_PCHAR16 = 0x017a;
let T_PCHAR32 = 0x017b;
let T_NCVPTR = 0x01f0;
let T_PFVOID = 0x0203;
let T_PFCHAR = 0x0210;
let T_PFSHORT = 0x0211;
let T_PFLONG = 0x0212;
let T_PFQUAD = 0x0213;
let T_PFOCT = 0x0214;
let T_PFUCHAR = 0x0220;
let T_PFUSHORT = 0x0221;
let T_PFULONG = 0x0222;
let T_PFUQUAD = 0x0223;
let T_PFUOCT = 0x0224;
let T_PFBOOL08 = 0x0230;
let T_PFBOOL16 = 0x0231;
let T_PFBOOL32 = 0x0232;
let T_PFBOOL64 = 0x0233;
let T_PFREAL32 = 0x0240;
let T_PFREAL64 = 0x0241;
let T_PFREAL80 = 0x0242;
let T_PFREAL128 = 0x0243;
let T_PFREAL48 = 0x0244;
let T_PFREAL32PP = 0x0245;
let T_PFREAL16 = 0x0246;
let T_PFCPLX32 = 0x0250;
let T_PFCPLX64 = 0x0251;
let T_PFCPLX80 = 0x0252;
let T_PFCPLX128 = 0x0253;
let T_PFINT1 = 0x0268;
let T_PFUINT1 = 0x0269;
let T_PFRCHAR = 0x0270;
let T_PFWCHAR = 0x0271;
let T_PFINT2 = 0x0272;
let T_PFUINT2 = 0x0273;
let T_PFINT4 = 0x0274;
let T_PFUINT4 = 0x0275;
let T_PFINT8 = 0x0276;
let T_PFUINT8 = 0x0277;
let T_PFINT16 = 0x0278;
let T_PFUINT16 = 0x0279;
let T_PFCHAR16 = 0x027a;
let T_PFCHAR32 = 0x027b;
let T_FCVPTR = 0x02f0;
let T_PHVOID = 0x0303;
let T_PHCHAR = 0x0310;
let T_PHSHORT = 0x0311;
let T_PHLONG = 0x0312;
let T_PHQUAD = 0x0313;
let T_PHOCT = 0x0314;
let T_PHUCHAR = 0x0320;
let T_PHUSHORT = 0x0321;
let T_PHULONG = 0x0322;
let T_PHUQUAD = 0x0323;
let T_PHUOCT = 0x0324;
let T_PHBOOL08 = 0x0330;
let T_PHBOOL16 = 0x0331;
let T_PHBOOL32 = 0x0332;
let T_PHBOOL64 = 0x0333;
let T_PHREAL32 = 0x0340;
let T_PHREAL64 = 0x0341;
let T_PHREAL80 = 0x0342;
let T_PHREAL128 = 0x0343;
let T_PHREAL48 = 0x0344;
let T_PHREAL32PP = 0x0345;
let T_PHREAL16 = 0x0346;
let T_PHCPLX32 = 0x0350;
let T_PHCPLX64 = 0x0351;
let T_PHCPLX80 = 0x0352;
let T_PHCPLX128 = 0x0353;
let T_PHINT1 = 0x0368;
let T_PHUINT1 = 0x0369;
let T_PHRCHAR = 0x0370;
let T_PHWCHAR = 0x0371;
let T_PHINT2 = 0x0372;
let T_PHUINT2 = 0x0373;
let T_PHINT4 = 0x0374;
let T_PHUINT4 = 0x0375;
let T_PHINT8 = 0x0376;
let T_PHUINT8 = 0x0377;
let T_PHINT16 = 0x0378;
let T_PHUINT16 = 0x0379;
let T_PHCHAR16 = 0x037a;
let T_PHCHAR32 = 0x037b;
let T_HCVPTR = 0x03f0;
let T_32PVOID = 0x0403;
let T_32PHRESULT = 0x0408;
let T_32PCHAR = 0x0410;
let T_32PSHORT = 0x0411;
let T_32PLONG = 0x0412;
let T_32PQUAD = 0x0413;
let T_32POCT = 0x0414;
let T_32PUCHAR = 0x0420;
let T_32PUSHORT = 0x0421;
let T_32PULONG = 0x0422;
let T_32PUQUAD = 0x0423;
let T_32PUOCT = 0x0424;
let T_32PBOOL08 = 0x0430;
let T_32PBOOL16 = 0x0431;
let T_32PBOOL32 = 0x0432;
let T_32PBOOL64 = 0x0433;
let T_32PREAL32 = 0x0440;
let T_32PREAL64 = 0x0441;
let T_32PREAL80 = 0x0442;
let T_32PREAL128 = 0x0443;
let T_32PREAL48 = 0x0444;
let T_32PREAL32PP = 0x0445;
let T_32PREAL16 = 0x0446;
let T_32PCPLX32 = 0x0450;
let T_32PCPLX64 = 0x0451;
let T_32PCPLX80 = 0x0452;
let T_32PCPLX128 = 0x0453;
let T_32PINT1 = 0x0468;
let T_32PUINT1 = 0x0469;
let T_32PRCHAR = 0x0470;
let T_32PWCHAR = 0x0471;
let T_32PINT2 = 0x0472;
let T_32PUINT2 = 0x0473;
let T_32PINT4 = 0x0474;
let T_32PUINT4 = 0x0475;
let T_32PINT8 = 0x0476;
let T_32PUINT8 = 0x0477;
let T_32PINT16 = 0x0478;
let T_32PUINT16 = 0x0479;
let T_32PCHAR16 = 0x047a;
let T_32PCHAR32 = 0x047b;
let T_32NCVPTR = 0x04f0;
let T_32PFVOID = 0x0503;
let T_32PFCHAR = 0x0510;
let T_32PFSHORT = 0x0511;
let T_32PFLONG = 0x0512;
let T_32PFQUAD = 0x0513;
let T_32PFOCT = 0x0514;
let T_32PFUCHAR = 0x0520;
let T_32PFUSHORT = 0x0521;
let T_32PFULONG = 0x0522;
let T_32PFUQUAD = 0x0523;
let T_32PFUOCT = 0x0524;
let T_32PFBOOL08 = 0x0530;
let T_32PFBOOL16 = 0x0531;
let T_32PFBOOL32 = 0x0532;
let T_32PFBOOL64 = 0x0533;
let T_32PFREAL32 = 0x0540;
let T_32PFREAL64 = 0x0541;
let T_32PFREAL80 = 0x0542;
let T_32PFREAL128 = 0x0543;
let T_32PFREAL48 = 0x0544;
let T_32PFREAL32PP = 0x0545;
let T_32PFREAL16 = 0x0546;
let T_32PFCPLX32 = 0x0550;
let T_32PFCPLX64 = 0x0551;
let T_32PFCPLX80 = 0x0552;
let T_32PFCPLX128 = 0x0553;
let T_32PFINT1 = 0x0568;
let T_32PFUINT1 = 0x0569;
let T_32PFRCHAR = 0x0570;
let T_32PFWCHAR = 0x0571;
let T_32PFINT2 = 0x0572;
let T_32PFUINT2 = 0x0573;
let T_32PFINT4 = 0x0574;
let T_32PFUINT4 = 0x0575;
let T_32PFINT8 = 0x0576;
let T_32PFUINT8 = 0x0577;
let T_32PFINT16 = 0x0578;
let T_32PFUINT16 = 0x0579;
let T_32PFCHAR16 = 0x057a;
let T_32PFCHAR32 = 0x057b;
let T_32FCVPTR = 0x05f0;
let T_64PVOID = 0x0603;
let T_64PHRESULT = 0x0608;
let T_64PCHAR = 0x0610;
let T_64PSHORT = 0x0611;
let T_64PLONG = 0x0612;
let T_64PQUAD = 0x0613;
let T_64POCT = 0x0614;
let T_64PUCHAR = 0x0620;
let T_64PUSHORT = 0x0621;
let T_64PULONG = 0x0622;
let T_64PUQUAD = 0x0623;
let T_64PUOCT = 0x0624;
let T_64PBOOL08 = 0x0630;
let T_64PBOOL16 = 0x0631;
let T_64PBOOL32 = 0x0632;
let T_64PBOOL64 = 0x0633;
let T_64PREAL32 = 0x0640;
let T_64PREAL64 = 0x0641;
let T_64PREAL80 = 0x0642;
let T_64PREAL128 = 0x0643;
let T_64PREAL48 = 0x0644;
let T_64PREAL32PP = 0x0645;
let T_64PREAL16 = 0x0646;
let T_64PCPLX32 = 0x0650;
let T_64PCPLX64 = 0x0651;
let T_64PCPLX80 = 0x0652;
let T_64PCPLX128 = 0x0653;
let T_64PINT1 = 0x0668;
let T_64PUINT1 = 0x0669;
let T_64PRCHAR = 0x0670;
let T_64PWCHAR = 0x0671;
let T_64PINT2 = 0x0672;
let T_64PUINT2 = 0x0673;
let T_64PINT4 = 0x0674;
let T_64PUINT4 = 0x0675;
let T_64PINT8 = 0x0676;
let T_64PUINT8 = 0x0677;
let T_64PINT16 = 0x0678;
let T_64PUINT16 = 0x0679;
let T_64PCHAR16 = 0x067a;
let T_64PCHAR32 = 0x067b;
let T_64NCVPTR = 0x06f0;

function alignUp(n, alignment) {
  let mask = alignment - 1;
  return (n + mask) & ~mask;
}

export function findAllCodeViewFunctions(reader) {
  let signature = reader.u32(0);
  if (signature !== CV_SIGNATURE_C13) {
    throw new UnsupportedCodeViewError(
      `unrecognized CodeView signature: 0x${signature.toString(16)}`
    );
  }

  let functions = [];
  let offset = 4;
  while (offset < reader.size) {
    offset = alignUp(offset, 4);
    let subsectionType = reader.u32(offset);
    offset += 4;
    let subsectionSize = reader.u32(offset);
    offset += 4;
    switch (subsectionType) {
      case DEBUG_S_SYMBOLS:
        findAllCodeViewFunctionsInSubsection(
          new SubFileReader(reader, offset, subsectionSize),
          functions
        );
        break;
      default:
        // Ignore.
        break;
    }
    offset += subsectionSize;
  }
  return functions;
}

function findAllCodeViewFunctionsInSubsection(reader, outFunctions) {
  let offset = 0;
  while (offset < reader.size) {
    let recordSize = reader.u16(offset + 0);
    let recordType = reader.u16(offset + 2);
    switch (recordType) {
      case S_GPROC32_ID: {
        outFunctions.push({
          name: reader.utf8CString(offset + 39),
          reader: reader,
          byteOffset: offset,
        });
        break;
      }
      default:
        break;
    }

    offset += recordSize + 2;
  }
}

// A local variable or parameter in a function.
export class CodeViewFunctionLocal {
  constructor(name) {
    this.name = name;
    this.spOffset = null;
    this.byteSize = -1;
  }
}

export function getCodeViewFunctionLocals(reader, offset) {
  let locals = [];
  while (offset < reader.size) {
    let recordSize = reader.u16(offset + 0);
    let recordType = reader.u16(offset + 2);
    switch (recordType) {
      case S_REGREL32: {
        let local = new CodeViewFunctionLocal(reader.utf8CString(offset + 14));
        // TODO(strager): Verify that the register is RSP.
        local.spOffset = reader.u32(offset + 4);
        let type = reader.u32(offset + 8);
        local.byteSize = specialTypeSize(type);
        locals.push(local);
        break;
      }
      // TODO(strager): Stop scanning when we hit S_PROC_ID_END.
      default:
        break;
    }

    offset += recordSize + 2;
  }
  return locals;
}

function specialTypeSize(type) {
  let maybeSize = specialTypeSizeMap[type];
  if (maybeSize === undefined) {
    console.warn(`unknown special type: 0x${type.toString(16)}`);
    return -1;
  }
  if (typeof maybeSize === "string") {
    console.warn(`unsupported special type: ${maybeSize}`);
    return -1;
  }
  return maybeSize;
}

let specialTypeSizeMap = {
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
