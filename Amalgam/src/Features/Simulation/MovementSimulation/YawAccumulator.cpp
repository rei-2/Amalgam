// YawAccumulator.cpp

#include "YawAccumulator.h"
#include "MovementSimulation.h" // for MoveData, Math helpers, TIME_TO_TICKS, etc.

void YawAccumulator::Begin(float straightFuzzyValue, int maxChanges, int maxChangeTimeTicks, float maxSpeedClamp)
{
    m_straightFuzzy = straightFuzzyValue;
    m_maxChanges = maxChanges;
    m_maxChangeTime = maxChangeTimeTicks;
    m_maxSpeed = maxSpeedClamp;
    m_started = false;
    m_changes = 0;
    m_startTick = 0;
    m_ticks = 0;
    m_yawAccum = 0.f;
    m_lastSign = 0;
    m_lastZero = false;
}

bool YawAccumulator::Step(MoveData& newer, MoveData& older)
{
    const float yawNew = Math::VectorAngles(newer.m_vDirection).y;
    const float yawOld = Math::VectorAngles(older.m_vDirection).y;
    const int   ticks  = std::max(TIME_TO_TICKS(newer.m_flSimTime - older.m_flSimTime), 1);

    float yawDelta = Math::NormalizeAngle(yawNew - yawOld);
    if (m_maxSpeed && newer.m_iMode != 1)
        yawDelta *= std::clamp(newer.m_vVelocity.Length2D() / m_maxSpeed, 0.f, 1.f);

    // replicate 45° sanity bound
    if (fabsf(yawDelta) > 45.f)
        return false;

    const int signNow = yawDelta ? (yawDelta > 0.f ? 1 : -1) : m_lastSign;
    const bool isZero = yawDelta == 0.f;

    const bool changed = (signNow != m_lastSign) || (isZero && m_lastZero);
    const bool straight = fabsf(yawDelta) * newer.m_vVelocity.Length2D() * ticks < m_straightFuzzy;

    if (!m_started)
    {
        m_started = true;
        m_startTick = TIME_TO_TICKS(newer.m_flSimTime);
        if (straight && ++m_changes > m_maxChanges)
            return false;
    }
    else
    {
        if ((changed || straight) && ++m_changes > m_maxChanges)
            return false;
        if (m_changes && (m_startTick - TIME_TO_TICKS(older.m_flSimTime)) > m_maxChangeTime)
            return false;
    }

    m_yawAccum += yawDelta;
    m_ticks += ticks;
    m_lastSign = signNow;
    m_lastZero = isZero;
    return true;
}

float YawAccumulator::Finalize(int minTicks, int dynamicMinTicks) const
{
    if (m_ticks < std::max(minTicks, dynamicMinTicks) || m_ticks <= 0)
        return 0.f;
    float avg = m_yawAccum / static_cast<float>(m_ticks);
    if (fabsf(avg) < 0.36f) // retain legacy cutoff
        return 0.f;
    return avg;
}
