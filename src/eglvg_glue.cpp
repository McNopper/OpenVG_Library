/*
 * OpenVG_Library - the OpenVG 1.1 Reference Implementation on McNopper/EGL.
 *
 * This translation unit is the seam between the reference implementation and
 * EGL: it provides the "EGL&OS functions for use in an OpenVG implementation"
 * the core calls (eglvgGetCurrentVGContext, eglvgIsInUse and the OS mutex) and
 * the EGLVGProvider callbacks EGL calls (see eglvg.h). The provider registers
 * itself at startup, so linking this library is all a consumer has to do.
 *
 * The MIT License (MIT)
 * Copyright (c) since 2013 Norbert Nopper
 */

#include <mutex>
#include <new>

// The reference implementation's riDefs.h defines `for` as a macro, which
// breaks standard headers included after it - all standard includes come first.
#include "eglvg.h"

#include "riContext.h"
#include "riImage.h"

namespace
{

    // The reference implementation serializes its API with the OS mutex;
    // recursive, so provider callbacks can run while it is already held.
    std::recursive_mutex g_mutex;

    // The contexts created through the provider and their current drawables.
    // The list answers eglvgIsInUse for all of them, mirroring the "any of the
    // displays" semantics of the original EGL glue.
    struct LiveContext
    {
        OpenVGRI::VGContext* context;
        OpenVGRI::Drawable*  drawable;
        void*                framebuffer;
        EGLint               width;
        EGLint               height;
        LiveContext*         next;
    };

    LiveContext* s_liveContexts = 0;

    thread_local void* t_currentContext = 0;

    LiveContext* findContext(void* context)
    {
        for (LiveContext* walker = s_liveContexts; walker; walker = walker->next)
        {
            if (walker->context == context)
            {
                return walker;
            }
        }

        return 0;
    }

}

namespace OpenVGRI
{

    // "EGL&OS functions for use in an OpenVG implementation" (declared in
    // riApi.cpp, implemented by the EGL side - here).

    void OSAcquireMutex(void)
    {
        g_mutex.lock();
    }

    void OSReleaseMutex(void)
    {
        g_mutex.unlock();
    }

    void* eglvgGetCurrentVGContext(void)
    {
        return t_currentContext;
    }

    bool eglvgIsInUse(void* image)
    {
        // Called with the mutex already held by an API function.
        for (LiveContext* walker = s_liveContexts; walker; walker = walker->next)
        {
            if (walker->drawable && walker->drawable->isInUse((Image*)image))
            {
                return true;
            }
        }

        return false;
    }

}

namespace
{

    EGLBoolean EGLAPIENTRY vgCreateContext(void** outContext)
    {
        if (!outContext)
        {
            return EGL_FALSE;
        }

        OpenVGRI::VGContext* context = 0;

        try
        {
            std::lock_guard<std::recursive_mutex> _{g_mutex};

            context = new OpenVGRI::VGContext(0);

            LiveContext* entry = new LiveContext;

            entry->context     = context;
            entry->drawable    = 0;
            entry->framebuffer = 0;
            entry->width       = 0;
            entry->height      = 0;
            entry->next        = s_liveContexts;
            s_liveContexts     = entry;
        }
        catch (...)
        {
            context = 0;
        }

        if (!context)
        {
            return EGL_FALSE;
        }

        *outContext = context;

        return EGL_TRUE;
    }

    EGLBoolean EGLAPIENTRY vgDestroyContext(void* context)
    {
        std::lock_guard<std::recursive_mutex> _{g_mutex};

        LiveContext** link = &s_liveContexts;

        while (*link)
        {
            LiveContext* entry = *link;

            if (entry->context == context)
            {
                *link = entry->next;

                if (t_currentContext == context)
                {
                    t_currentContext = 0;
                }

                delete entry->drawable;
                delete entry->context;
                delete entry;

                return EGL_TRUE;
            }

            link = &entry->next;
        }

        return EGL_FALSE;
    }

    EGLBoolean EGLAPIENTRY vgMakeCurrent(void* context, void* framebuffer, EGLint format, EGLint width, EGLint height, EGLint strideBytes)
    {
        std::lock_guard<std::recursive_mutex> _{g_mutex};

        if (!context)
        {
            t_currentContext = 0;

            return EGL_TRUE;
        }

        if (format != EGL_VG_BUFFER_FORMAT_SARGB_8888 || !framebuffer || width <= 0 || height <= 0 || strideBytes < width * 4)
        {
            return EGL_FALSE;
        }

        LiveContext* entry = findContext(context);

        if (!entry)
        {
            return EGL_FALSE;
        }

        if (entry->framebuffer != framebuffer || entry->width != width || entry->height != height)
        {
            delete entry->drawable;

            entry->drawable    = 0;
            entry->framebuffer = framebuffer;
            entry->width       = width;
            entry->height      = height;

            try
            {
                // VG_sARGB_8888 is the natural window layout of the EGL
                // backends (see eglvg.h); the buffer stays owned by EGL. A
                // full 8-bit mask buffer keeps vgMask available.
                entry->drawable = new OpenVGRI::Drawable(OpenVGRI::Color::formatToDescriptor(VG_sARGB_8888), width, height, strideBytes, (OpenVGRI::RIuint8*)framebuffer, 8);
            }
            catch (...)
            {
                entry->drawable = 0;
            }

            if (!entry->drawable)
            {
                return EGL_FALSE;
            }
        }

        entry->context->setDefaultDrawable(entry->drawable);

        t_currentContext = context;

        return EGL_TRUE;
    }

    void EGLAPIENTRY vgNotifySwap(void* /*context*/)
    {
        // The reference implementation renders directly into the EGL buffer.
    }

    const EGLVGProvider g_provider = {vgCreateContext, vgDestroyContext, vgMakeCurrent, vgNotifySwap};

    // Runs at startup. This translation unit is linked whenever any vg* entry
    // point is used - which is exactly when a provider is needed.
    struct ProviderRegistration
    {
        ProviderRegistration()
        {
            eglSetVGProvider(&g_provider);
        }
    };

    ProviderRegistration g_providerRegistration;

}
