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

const char* NET_SetConVar::ToString() const
{
    return std::format("{}: {} cvars, \"{}\"=\"{}\"", GetName(), m_ConVars.Count(), m_ConVars[0].Name, m_ConVars[0].Value).c_str();
}

bool NET_SetConVar::WriteToBuffer(bf_write& buffer)
{
    buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

    int numvars = m_ConVars.Count();

    buffer.WriteByte(numvars);

    for (int i = 0; i < numvars; i++)
    {
        CVar_t* var = &m_ConVars[i];
        buffer.WriteString(var->Name);
        buffer.WriteString(var->Value);
    }

    return !buffer.IsOverflowed();
}

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer)
{
    int numvars = buffer.ReadByte();

    m_ConVars.RemoveAll();

    for (int i = 0; i < numvars; i++)
    {
        CVar_t var;
        buffer.ReadString(var.Name, sizeof(var.Name));
        buffer.ReadString(var.Value, sizeof(var.Value));
        m_ConVars.AddToTail(var);
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