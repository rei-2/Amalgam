#include "BaselineCapture.h"
#include <fstream>
#include <iomanip>

// Minimal local includes for accessing player data; avoid pulling whole SDK where possible.
#include "../../src/SDK/Helpers/Entities/Entities.h" // corrected relative include path

bool BaselineCapture::Capture(int entIndex, int ticks, std::function<bool(int)> tickCallback, const CaptureOptions& opts)
{
    m_samples.clear();
    if (ticks <= 0) return false;
    m_samples.reserve(ticks);

    for (int i = 0; i < ticks; ++i)
    {
        if (!tickCallback(i))
            return false;

        auto pEntity = I::ClientEntityList->GetClientEntity(entIndex);
        if (!pEntity) return false;
        auto pPlayer = pEntity->As<CTFPlayer>();
        if (!pPlayer) return false;
        if (opts.requireAlive && !pPlayer->IsAlive()) return false;

        MovementSample s{};
        s.tick = i;
        s.simTime = pPlayer->m_flSimulationTime();
        if (opts.includeOrigin)
        {
            auto o = pPlayer->m_vecOrigin();
            s.origin[0] = o.x; s.origin[1] = o.y; s.origin[2] = o.z;
        }
        if (opts.includeVelocity)
        {
            auto v = pPlayer->m_vecVelocity();
            s.velocity[0] = v.x; s.velocity[1] = v.y; s.velocity[2] = v.z;
        }
        m_samples.push_back(s);
    }
    return true;
}

bool BaselineCapture::WriteJSON(const std::string& path) const
{
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs.is_open()) return false;
    ofs << "{\n  \"version\":1,\n  \"samples\":[\n";
    for (size_t i = 0; i < m_samples.size(); ++i)
    {
        const auto& s = m_samples[i];
        ofs << "    {\"tick\":" << s.tick
            << ",\"simTime\":" << std::fixed << std::setprecision(6) << s.simTime
            << ",\"origin\": [" << s.origin[0] << ',' << s.origin[1] << ',' << s.origin[2] << "]"
            << ",\"velocity\": [" << s.velocity[0] << ',' << s.velocity[1] << ',' << s.velocity[2] << "]}";
        if (i + 1 < m_samples.size()) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n}\n";
    return true;
}
