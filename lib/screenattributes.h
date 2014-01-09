#ifndef Q5250_SCREENATTRIBUTES_H
#define Q5250_SCREENATTRIBUTES_H

namespace q5250 {

namespace ScreenAttribute {

enum ScreenAttributes {
    GREEN           = 0x20,     // 0010 0000
    GREEN_RI        = 0x21,     // 0010 0001
    WHITE           = 0x22,     // 0010 0010
    WHITE_RI        = 0x23,     // 0010 0011
    GREEN_UL        = 0x24,     // 0010 0100
    GREEN_UL_RI     = 0x25,     // 0010 0101
    WHITE_UL        = 0x26,     // 0010 0110
    NON_DISPLAY     = 0x27,     // 0010 0111
    RED             = 0x28,     // 0010 1000
    RED_RI          = 0x29,     // 0010 1001
    RED_BL          = 0x2a,     // 0010 1010
    RED_RI_BL       = 0x2b,     // 0010 1011
    RED_UL          = 0x2c,     // 0010 1100
    RED_UL_RI       = 0x2d,     // 0010 1101
    RED_UL_BL       = 0x2e,     // 0010 1110
    NON_DISPLAY2    = 0x2f,     // 0010 1111
    TURQUOISE_CS    = 0x30,     // 0011 0000
    TURQUOISE_CS_RI = 0x31,     // 0011 0001
    YELLOW_CS       = 0x32,     // 0011 0010
    YELLOW_CS_RI    = 0x33,     // 0011 0011
    TURQUOISE_UL    = 0x34,     // 0011 0100
    TURQUOISE_UL_RI = 0x35,     // 0011 0101
    YELLOW_UL       = 0x36,     // 0011 0110
    NON_DISPLAY3    = 0x37,     // 0011 0111
    PINK            = 0x38,     // 0011 1000
    PINK_RI         = 0x39,     // 0011 1001
    BLUE            = 0x3a,     // 0011 1010
    BLUE_RI         = 0x3b,     // 0011 1011
    PINK_UL         = 0x3c,     // 0011 1100
    PINK_UL_RI      = 0x3d,     // 0011 1101
    BLUE_UL         = 0x3e,     // 0011 1110
    NON_DISPLAY4    = 0x3f      // 0011 1111
};

bool IsNonDisplay(unsigned char attribute);
bool ShowUnderline(unsigned char attribute);

} // namespace ScreenAttribute

} // namespace q5250

#endif // Q5250_SCREENATTRIBUTES_H
