#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "internalfilters.h"
#include "libvmaf/picture.h"
#include "libvmaf/cambi.h"

#include "VapourSynth4.h"
#include "VSHelper4.h"

typedef struct {
    VSNode *node;
    VSVideoInfo vi;
    CambiState s;
    int bpc;
    int scores;
    float scaling;
} CambiData;

static const VSFrame *VS_CC cambiGetFrame(int n, int activationReason, void *instanceData, void **frameData, VSFrameContext *frameCtx, VSCore *core, const VSAPI *vsapi) {
    CambiData *d = (CambiData *) instanceData;

    if (activationReason == arInitial) {
        vsapi->requestFrameFilter(n, d->node, frameCtx);
    } else if (activationReason == arAllFramesReady) {
        const VSFrame *src = vsapi->getFrameFilter(n, d->node, frameCtx);
        const unsigned int width = vsapi->getFrameWidth(src, 0);
        const unsigned int height = vsapi->getFrameHeight(src, 0);
        VSFrame *dst = vsapi->copyFrame(src, core);

        VmafPicture pic; // shares memory with src
        pic.pix_fmt = VMAF_PIX_FMT_YUV400P; // GRAY
        pic.bpc = d->bpc;
        pic.w[0] = width;
        pic.h[0] = height;
        pic.stride[0] = vsapi->getStride(src, 0);
        pic.data[0] = (uint8_t *)vsapi->getReadPtr(src, 0);
        pic.ref = NULL;

        double score;
        CambiState s = d->s; // cambiGetFrame might be called concurrently
        int err = cambi_init(&s, pic.w[0], pic.h[0]);
        assert(err == 0);

        float *c_values[NUM_SCALES];
        if (d->scores) {
            unsigned int w = width, h = height;
            for (int i = 0; i < NUM_SCALES; i++) {
                c_values[i] = calloc(w * h, sizeof *c_values[i]);
                scale_dimension(&w, 1);
                scale_dimension(&h, 1);
            }
        }
        err = cambi_extract(&s, &pic, &score, d->scores ? c_values : NULL);
        cambi_close(&s);

        VSMap *prop = vsapi->getFramePropertiesRW(dst);
        if (d->scores) {
            VSVideoFormat grays;
            vsapi->getVideoFormatByID(&grays, pfGrayS, core);
            unsigned int w = width, h = height;
            for (int i = 0; i < NUM_SCALES; i++) {
                VSFrame *f = vsapi->newVideoFrame(&grays, w, h, src, core);
                float *dst = (float *)vsapi->getWritePtr(f, 0);
                float *src = c_values[i];
                uintptr_t stride = vsapi->getStride(f, 0) / sizeof *dst;
                for (int y = 0; y < h; y++) {
                        for (int x = 0; x < w; x++)
                                dst[x] = src[x] * d->scaling;
                        src += w;
                        dst += stride;
                }
                free(c_values[i]);
                scale_dimension(&w, 1);
                scale_dimension(&h, 1);
                char name[16];
                sprintf(name, "CAMBI_SCALE%d", i);
                vsapi->mapSetFrame(prop, name, f, maReplace);
                vsapi->freeFrame(f);
            }
        }
        vsapi->freeFrame(src);
        assert(err == 0);

        err = vsapi->mapSetFloat(prop, "CAMBI", score, maReplace);
        assert(err == 0);

        return dst;
    }

    return NULL;
}

static void VS_CC cambiFree(void *instanceData, VSCore *core, const VSAPI *vsapi) {
    CambiData *d = (CambiData *)instanceData;
    vsapi->freeNode(d->node);
    cambi_close(&d->s);
    free(d);
}

// This function is responsible for validating arguments and creating a new filter
static void VS_CC cambiCreate(const VSMap *in, VSMap *out, void *userData, VSCore *core, const VSAPI *vsapi) {
    CambiData d;

    d.node = vsapi->mapGetNode(in, "clip", 0, 0);
    d.vi = *vsapi->getVideoInfo(d.node);

    if (!vsh_isConstantVideoFormat(&d.vi) || d.vi.format.sampleType != stInteger ||
        (d.vi.format.colorFamily != cfGray && d.vi.format.colorFamily != cfYUV) ||
        (d.vi.format.bitsPerSample != 8 && d.vi.format.bitsPerSample != 10)) {
        vsapi->mapSetError(out, "Cambi: only constant Gray/YUV format with 8/10bit integer samples supported");
        vsapi->freeNode(d.node);
        return;
    }
    d.bpc = d.vi.format.bitsPerSample;

    cambi_config(&d.s);
#define GETARG(type, var, name, api, min, max) \
    do { \
        int err; \
        type x = vsapi->api(in, #name, 0, &err); \
        if (err != 0) break; \
        if (x < min || x > max) { \
            char errmsg[256]; \
            snprintf(errmsg, sizeof errmsg, "Cambi: argument %s=%f is out of range [%f,%f] (default=%f)", #name, (double)x, (double)min, (double)max, (double)var.name); \
            vsapi->mapSetError(out, errmsg); \
            vsapi->freeNode(d.node); \
            return; \
        } \
        var.name = x; \
    } while (0)
    GETARG(int, d.s, window_size, mapGetInt, 15, 127);
    GETARG(double, d.s, topk, mapGetFloat, 0.0001, 1);
    GETARG(double, d.s, tvi_threshold, mapGetFloat, 0.0001, 1);
    d.scores = 0;
    GETARG(int, d, scores, mapGetInt, 0, 1);
    d.scaling = 1.0f / d.s.window_size;
    GETARG(int, d, scaling, mapGetFloat, 0, 1);
#undef GETARG

    int err = cambi_init(&d.s, d.vi.width, d.vi.height);
    if (err != 0) {
        vsapi->mapSetError(out, "cambi_init failure");
        vsapi->freeNode(d.node);
        return;
    }

    CambiData *data = malloc(sizeof(d));
    *data = d;

    VSFilterDependency deps[] = {{d.node, rpStrictSpatial}};

    vsapi->createVideoFilter(out, "Cambi", &d.vi, cambiGetFrame, cambiFree, fmParallel, deps, 1, data, core);
}

void bandingInitialize(VSPlugin *plugin, const VSPLUGINAPI *vsapi) {
    vsapi->registerFunction(
        "Cambi",
        "clip:vnode;window_size:int:opt;topk:float:opt;tvi_threshold:float:opt;scores:int:opt;scaling:float:opt;",
        "clip:vnode",
        cambiCreate,
        0,
        plugin
    );
}
