#include "adaptivepreferences.h"

#include <QCoreApplication>

AdaptivePreferences* AdaptivePreferences::s_Instance = nullptr;

AdaptivePreferences::AdaptivePreferences()
    : QObject(nullptr)
{
    reload();
}

AdaptivePreferences* AdaptivePreferences::get()
{
    if (s_Instance == nullptr) {
        s_Instance = new AdaptivePreferences();
    }
    return s_Instance;
}

void AdaptivePreferences::reload()
{
    QSettings settings;

    // Load settings with defaults
    adaptiveModeEnabled = settings.value("adaptive/enabled", false).toBool();
    networkPreset = static_cast<NetworkPreset>(
        settings.value("adaptive/networkPreset", NP_WIFI).toInt());
    frameDropPolicy = static_cast<FrameDropPolicy>(
        settings.value("adaptive/frameDropPolicy", FDP_DROP_OLDEST).toInt());
    frameQueueDepth = settings.value("adaptive/frameQueueDepth", 3).toInt();
    enableFrameRepetition = settings.value("adaptive/enableFrameRepetition", false).toBool();
    enableInputCoalescing = settings.value("adaptive/enableInputCoalescing", false).toBool();
    inputBufferMs = settings.value("adaptive/inputBufferMs", 0).toInt();
    showFrameQueueDepth = settings.value("adaptive/showFrameQueueDepth", false).toBool();
    showAdaptiveStats = settings.value("adaptive/showAdaptiveStats", false).toBool();

    // Clamp values to valid ranges
    frameQueueDepth = qBound(1, frameQueueDepth, 5);
    inputBufferMs = qBound(0, inputBufferMs, 50);
}

void AdaptivePreferences::save()
{
    QSettings settings;

    settings.setValue("adaptive/enabled", adaptiveModeEnabled);
    settings.setValue("adaptive/networkPreset", static_cast<int>(networkPreset));
    settings.setValue("adaptive/frameDropPolicy", static_cast<int>(frameDropPolicy));
    settings.setValue("adaptive/frameQueueDepth", frameQueueDepth);
    settings.setValue("adaptive/enableFrameRepetition", enableFrameRepetition);
    settings.setValue("adaptive/enableInputCoalescing", enableInputCoalescing);
    settings.setValue("adaptive/inputBufferMs", inputBufferMs);
    settings.setValue("adaptive/showFrameQueueDepth", showFrameQueueDepth);
    settings.setValue("adaptive/showAdaptiveStats", showAdaptiveStats);
}

void AdaptivePreferences::applyNetworkPreset(NetworkPreset preset)
{
    networkPreset = preset;

    switch (preset) {
    case NP_LAN:
        // Minimal buffering, lowest latency
        frameQueueDepth = 1;
        frameDropPolicy = FDP_DROP_OLDEST;
        enableFrameRepetition = false;
        enableInputCoalescing = false;
        inputBufferMs = 0;
        break;

    case NP_WIFI:
        // Moderate buffering (default)
        frameQueueDepth = 3;
        frameDropPolicy = FDP_DROP_OLDEST;
        enableFrameRepetition = false;
        enableInputCoalescing = false;
        inputBufferMs = 0;
        break;

    case NP_POOR_INTERNET:
        // Maximum buffering, smoothness priority
        frameQueueDepth = 5;
        frameDropPolicy = FDP_REPEAT_LAST;
        enableFrameRepetition = true;
        enableInputCoalescing = true;
        inputBufferMs = 16;  // ~1 frame at 60fps
        break;
    }

    emit networkPresetChanged();
    emit frameQueueDepthChanged();
    emit frameDropPolicyChanged();
    emit enableFrameRepetitionChanged();
    emit enableInputCoalescingChanged();
    emit inputBufferMsChanged();
}
