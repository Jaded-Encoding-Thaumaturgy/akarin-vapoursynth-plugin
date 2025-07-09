#include <vector>

#include "VapourSynth4.h"

#include "plugin.h"
#include "version.h"

#include "expr/internalfilters.h"
#include "ngx/internalfilters.h"
#include "vfx/internalfilters.h"
#include "banding/internalfilters.h"
#include "text/internalfilters.h"

static std::vector<VSPublicFunction> versionFuncs;

void registerVersionFunc(VSPublicFunction f) {
    versionFuncs.push_back(f);
}

void VS_CC versionCreate(const VSMap *in, VSMap *out, void *user_data, VSCore *core, const VSAPI *vsapi)
{
    for (auto f: versionFuncs)
        f(in, out, user_data, core, vsapi);
    vsapi->mapSetData(out, "version", VERSION, -1, dtUtf8, maAppend);
}

VS_EXTERNAL_API(void) VapourSynthPluginInit2(VSPlugin *plugin, const VSPLUGINAPI *vsapi) {
    vsapi->configPlugin(
        "info.akarin.vsplugin",
        "akarin",
        "Akarin's Experimental Filters",
        VS_MAKE_VERSION(1, 0),
        VAPOURSYNTH_API_VERSION,
        0,
        plugin
    );
    vsapi->registerFunction("Version", "", "clip:vnode;", versionCreate, nullptr, plugin);
    exprInitialize(plugin, vsapi);
#ifdef HAVE_NGX
    ngxInitialize(plugin, vsapi);
#endif
#ifdef HAVE_VFX
    vfxInitialize(plugin, vsapi);
#endif
    bandingInitialize(plugin, vsapi);
    textInitialize(plugin, vsapi);
    tmplInitialize(plugin, vsapi);
}
