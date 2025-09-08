#pragma once

// BaselineCapture
// Collects ground truth movement samples for baseline vs improved comparison.
// NOTE: This is a lightweight harness; integrates minimally with production code.

#include <vector>
#include <string>
#include <functional>

// forward dec of engine interfaces (avoid heavy includes)
struct Vec3; // real definition in SDK
class CTFPlayer; // forward
namespace I { struct IGlobalVarsStub; }

struct CaptureOptions
{
    bool includeVelocity = true;    // capture velocity array
    bool includeOrigin   = true;    // capture origin array
    bool requireAlive    = true;    // abort if player dies mid cap
};

struct MovementSample
{
    int   tick{};
    float simTime{};
    float origin[3]{};
    float velocity[3]{};
};

struct BaselineMetrics
{
    std::vector<float> posError; // placeholder (empty for baseline raw capture)
    std::vector<float> velError; // placeholder
    float maxPosError{};         // computed during comparison
};

class BaselineCapture
{
public:
    // capture N ticks from a player entity (identified by entindex) invoking per-tick callback to advance sim
    // callback must advance the underlying simulation by exactly one tick.
    bool Capture(int entIndex, int ticks, std::function<bool(int)> tickCallback, const CaptureOptions& opts = {});

    const std::vector<MovementSample>& Samples() const { return m_samples; }

    // serialize raw samples to a very small JSON-like file
    bool WriteJSON(const std::string& path) const;

private:
    std::vector<MovementSample> m_samples;
};
