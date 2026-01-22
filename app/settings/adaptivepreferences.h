#pragma once

#include <QObject>
#include <QSettings>

#ifdef ADAPTIVE_CLIENT_ENABLED
#define ADAPTIVE_FEATURE_AVAILABLE true
#else
#define ADAPTIVE_FEATURE_AVAILABLE false
#endif

class AdaptivePreferences : public QObject
{
    Q_OBJECT

public:
    static AdaptivePreferences* get();
    static bool isFeatureAvailable() { return ADAPTIVE_FEATURE_AVAILABLE; }

    Q_INVOKABLE void save();
    void reload();

    // Frame Drop Policy
    enum FrameDropPolicy {
        FDP_DROP_OLDEST,     // Default - drop oldest frame in queue
        FDP_DROP_NEWEST,     // Drop newest frame (preserve continuity)
        FDP_REPEAT_LAST      // Repeat last frame on stall (smoothness priority)
    };
    Q_ENUM(FrameDropPolicy)

    // Network Preset
    enum NetworkPreset {
        NP_LAN,              // Minimal buffering, lowest latency
        NP_WIFI,             // Moderate buffering (default)
        NP_POOR_INTERNET     // Maximum buffering, smoothness priority
    };
    Q_ENUM(NetworkPreset)

    // Qt Properties for QML binding
    Q_PROPERTY(bool adaptiveModeEnabled MEMBER adaptiveModeEnabled NOTIFY adaptiveModeEnabledChanged)
    Q_PROPERTY(NetworkPreset networkPreset MEMBER networkPreset NOTIFY networkPresetChanged)
    Q_PROPERTY(FrameDropPolicy frameDropPolicy MEMBER frameDropPolicy NOTIFY frameDropPolicyChanged)
    Q_PROPERTY(int frameQueueDepth MEMBER frameQueueDepth NOTIFY frameQueueDepthChanged)
    Q_PROPERTY(bool enableFrameRepetition MEMBER enableFrameRepetition NOTIFY enableFrameRepetitionChanged)
    Q_PROPERTY(bool enableInputCoalescing MEMBER enableInputCoalescing NOTIFY enableInputCoalescingChanged)
    Q_PROPERTY(int inputBufferMs MEMBER inputBufferMs NOTIFY inputBufferMsChanged)
    Q_PROPERTY(bool showFrameQueueDepth MEMBER showFrameQueueDepth NOTIFY showFrameQueueDepthChanged)
    Q_PROPERTY(bool showAdaptiveStats MEMBER showAdaptiveStats NOTIFY showAdaptiveStatsChanged)

    // Directly accessible members
    bool adaptiveModeEnabled;
    NetworkPreset networkPreset;
    FrameDropPolicy frameDropPolicy;
    int frameQueueDepth;             // 1-5, default: 3
    bool enableFrameRepetition;
    bool enableInputCoalescing;
    int inputBufferMs;               // 0-50ms, default: 0
    bool showFrameQueueDepth;
    bool showAdaptiveStats;

    // Helper to apply network preset
    Q_INVOKABLE void applyNetworkPreset(NetworkPreset preset);

signals:
    void adaptiveModeEnabledChanged();
    void networkPresetChanged();
    void frameDropPolicyChanged();
    void frameQueueDepthChanged();
    void enableFrameRepetitionChanged();
    void enableInputCoalescingChanged();
    void inputBufferMsChanged();
    void showFrameQueueDepthChanged();
    void showAdaptiveStatsChanged();

private:
    explicit AdaptivePreferences();
    static AdaptivePreferences* s_Instance;
};
