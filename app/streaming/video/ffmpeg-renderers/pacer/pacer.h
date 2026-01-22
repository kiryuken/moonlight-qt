#pragma once

#include "../../decoder.h"
#include "../renderer.h"
#include "../../../../settings/adaptivepreferences.h"

#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

// Default maximum number of frames pacer will ever hold is:
// - 3 frames in the pacing queue
// - 1 frame removed from the render queue in the process of rendering
// - 1 frame for deferred free
#define PACER_DEFAULT_QUEUE_DEPTH 3

#ifdef ADAPTIVE_CLIENT_ENABLED
// Adaptive mode: configurable queue depth range
#define ADAPTIVE_MIN_QUEUE_DEPTH 1
#define ADAPTIVE_MAX_QUEUE_DEPTH 5
#define PACER_ABSOLUTE_MAX_QUEUE_DEPTH ADAPTIVE_MAX_QUEUE_DEPTH
#else
#define PACER_ABSOLUTE_MAX_QUEUE_DEPTH PACER_DEFAULT_QUEUE_DEPTH
#endif

// Calculate frames for a specific queue depth
#define PACER_OUTSTANDING_FRAMES_FOR_DEPTH(depth) ((depth) + 1 + 1)

// Absolute maximum for allocation (used by ffmpeg.cpp)
#define PACER_MAX_OUTSTANDING_FRAMES PACER_OUTSTANDING_FRAMES_FOR_DEPTH(PACER_ABSOLUTE_MAX_QUEUE_DEPTH)

class IVsyncSource {
public:
    virtual ~IVsyncSource() {}
    virtual bool initialize(SDL_Window* window, int displayFps) = 0;

    // Asynchronous sources produce callbacks on their own, while synchronous
    // sources require calls to waitForVsync().
    virtual bool isAsync() = 0;

    virtual void waitForVsync() {
        // Synchronous sources must implement waitForVsync()!
        SDL_assert(false);
    }
};

class Pacer
{
public:
    Pacer(IFFmpegRenderer* renderer, PVIDEO_STATS videoStats);

    ~Pacer();

    void submitFrame(AVFrame* frame);

    bool initialize(SDL_Window* window, int maxVideoFps, bool enablePacing);

    void signalVsync();

    void renderOnMainThread();

#ifdef ADAPTIVE_CLIENT_ENABLED
    // Adaptive mode configuration
    void setAdaptivePreferences(AdaptivePreferences* prefs);
    void updateAdaptiveSettings();
    int getEffectiveQueueDepth() const;
#endif

private:
    static int vsyncThread(void* context);

    static int renderThread(void* context);

    void handleVsync(int timeUntilNextVsyncMillis);

    void enqueueFrameForRenderingAndUnlock(AVFrame* frame);

    void renderFrame(AVFrame* frame);

    void dropFrameForEnqueue(QQueue<AVFrame*>& queue);

#ifdef ADAPTIVE_CLIENT_ENABLED
    AVFrame* dropFrameWithPolicy(QQueue<AVFrame*>& queue);
#endif

    QQueue<AVFrame*> m_RenderQueue;
    QQueue<AVFrame*> m_PacingQueue;
    QQueue<int> m_PacingQueueHistory;
    QQueue<int> m_RenderQueueHistory;
    QMutex m_FrameQueueLock;
    QWaitCondition m_RenderQueueNotEmpty;
    QWaitCondition m_PacingQueueNotEmpty;
    QWaitCondition m_VsyncSignalled;
    SDL_Thread* m_RenderThread;
    SDL_Thread* m_VsyncThread;
    AVFrame* m_DeferredFreeFrame;
    bool m_Stopping;

    IVsyncSource* m_VsyncSource;
    IFFmpegRenderer* m_VsyncRenderer;
    int m_MaxVideoFps;
    int m_DisplayFps;
    PVIDEO_STATS m_VideoStats;
    int m_RendererAttributes;

#ifdef ADAPTIVE_CLIENT_ENABLED
    // Adaptive client state
    AdaptivePreferences* m_AdaptivePrefs;
    int m_ConfiguredQueueDepth;
    AdaptivePreferences::FrameDropPolicy m_DropPolicy;
    AVFrame* m_LastRenderedFrame;   // For frame repetition
    uint64_t m_LastFrameArrivalTime; // Stall detection
    bool m_StallDetected;
    uint32_t m_FramesRepeated;       // Stats counter
#endif
};
