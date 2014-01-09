#include "screenattributes.h"

namespace q5250 {

namespace ScreenAttribute {

static const unsigned short UNDERLINE_MASK        = 0x04;
static const unsigned short NON_DISPLAY_MASK      = 0x07;

bool ShowUnderline(unsigned char attribute)
{
    return attribute & UNDERLINE_MASK;
}

bool IsNonDisplay(unsigned char attribute)
{
    return (attribute & NON_DISPLAY_MASK) == NON_DISPLAY_MASK;
}

} // namespace ScreenAttribute

} // namespace q5250
