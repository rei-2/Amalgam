#include "INetMessage.h"

#include <format>
#include <algorithm>

#define NETMSG_TYPE_BITS 6
#define NUM_NEW_COMMAND_BITS 4
#define NUM_BACKUP_COMMAND_BITS 3

const char* CLC_Move::ToString(void) const { return "CLC_Move"; }

bool CLC_Move::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();

    buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
    buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

    buffer.WriteWord(m_nLength);

    return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool CLC_Move::ReadFromBuffer(bf_read& buffer)
{
    m_nNewCommands = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
    m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
    m_nLength = buffer.ReadWord();
    m_DataIn = buffer;
    return buffer.SeekRelative(m_nLength);
}

const char* NET_SetConVar::ToString() const { return "(NULL)"; }

bool NET_SetConVar::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), 6);
    const int numVars = 1;
    buffer.WriteByte(numVars);
    buffer.WriteString(ConVar.Name);
    buffer.WriteString(ConVar.Value);

    return !buffer.IsOverflowed();
}

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer)
{
    const int numVars = buffer.ReadByte();

    for (int i = 0; i < numVars; i++)
    {
        CVar_t cvar;
        buffer.ReadString(cvar.Name, sizeof(cvar.Name));
        buffer.ReadString(cvar.Value, sizeof(cvar.Value));
    }
    return !buffer.IsOverflowed();
}

bool NET_SignonState::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), 6);
    buffer.WriteByte(m_nSignonState);
    buffer.WriteLong(m_nSpawnCount);

    return !buffer.IsOverflowed();
}

bool NET_SignonState::ReadFromBuffer(bf_read& buffer)
{
    /*
    m_nSignonState = buffer.ReadByte();
    m_nSpawnCount = buffer.ReadLong();
    */
    return true;
}

const char* NET_SignonState::ToString(void) const
{
    return std::format("net_SignonState: state {}, count {}", m_nSignonState, m_nSpawnCount).c_str();
}

#define NET_TICK_SCALEUP	100000.0f

bool NET_Tick::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    buffer.WriteLong(m_nTick);
    buffer.WriteUBitLong(std::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTime), 0, 65535), 16);
    buffer.WriteUBitLong(std::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTimeStdDeviation), 0, 65535), 16);
    return !buffer.IsOverflowed();
}

bool NET_Tick::ReadFromBuffer(bf_read& buffer)
{
    m_nTick = buffer.ReadLong();
    m_flHostFrameTime = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
    m_flHostFrameTimeStdDeviation = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
    return !buffer.IsOverflowed();
}


const char* NET_Tick::ToString(void) const
{
    return std::format("{}: tick {}", GetName(), m_nTick).c_str();
}

const char* CLC_VoiceData::ToString(void) const
{
    return std::format("{}: {} bytes", GetName(), static_cast<int>(m_nLength * 0.125f)).c_str();
}


bool CLC_VoiceData::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    m_nLength = m_DataOut.GetNumBitsWritten();
    buffer.WriteWord(m_nLength); // length in bits

    return buffer.WriteBits(m_DataOut.GetBasePointer(), m_nLength);
}

bool CLC_VoiceData::ReadFromBuffer(bf_read& buffer)
{
    m_nLength = buffer.ReadWord(); // length in bits
    m_DataIn = buffer;

    return buffer.SeekRelative(m_nLength);
}

bool CLC_BaselineAck::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
    buffer.WriteLong(m_nBaselineTick);
    buffer.WriteUBitLong(m_nBaselineNr, 1);
    return !buffer.IsOverflowed();
}

bool CLC_BaselineAck::ReadFromBuffer(bf_read& buffer)
{

    m_nBaselineTick = buffer.ReadLong();
    m_nBaselineNr = buffer.ReadUBitLong(1);
    return !buffer.IsOverflowed();
}

const char* CLC_BaselineAck::ToString(void) const
{
    return std::format("{}: tick {}", GetName(), m_nBaselineTick).c_str();
}