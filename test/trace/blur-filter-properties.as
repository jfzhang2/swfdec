// makeswf -v 7 -r 1 -o blur-filter-properties-7.swf blur-filter-properties.as

// enable flash structure for version < 8 too for this test
ASSetPropFlags (_global, "flash", 0, 4096);

#include "trace_properties.as"

var a = new flash.filters.BlurFilter ();

trace_properties (_global.flash.filters.BlurFilter, "_global.flash.filters",
    "BlurFilter");
trace_properties (a, "local", "a");

loadMovie ("FSCommand:quit", "");
