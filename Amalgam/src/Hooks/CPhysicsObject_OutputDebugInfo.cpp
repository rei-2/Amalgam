#include "../SDK/SDK.h"

MAKE_SIGNATURE(CPhysicsObject_OutputDebugInfo, "vphysics.dll", "48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68", 0x0);

// physics_debug_entity !picker
MAKE_HOOK(CPhysicsObject_OutputDebugInfo, S::CPhysicsObject_OutputDebugInfo(), void,
    void* rcx)
{
    //CALL_ORIGINAL(rcx);

	/*
	auto GetVelocity = reinterpret_cast<void(__fastcall*)(void*, Vec3*, Vec3*)>(*reinterpret_cast<uintptr_t*>(rcx) + 408); // crash...
	Vec3 speed, angSpeed;
	GetVelocity(rcx, &speed, &angSpeed);
	SDK::Output("Velocity", std::format("{}, {}, {} ({})", speed.x, speed.y, speed.z, speed.Length()).c_str());
	SDK::Output("Ang Velocity", std::format("{}, {}, {} ({})", angSpeed.x, angSpeed.y, angSpeed.z, angSpeed.Length()).c_str());
	*/

	SDK::Output("Linear drag", std::format("{}, {}, {} ({})", *reinterpret_cast<float*>(uintptr_t(rcx) + 10i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 11i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 12i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 22i64 * 4)).c_str());
	SDK::Output("Angular drag", std::format("{}, {}, {} ({})", *reinterpret_cast<float*>(uintptr_t(rcx) + 13i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 14i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 15i64 * 4), *reinterpret_cast<float*>(uintptr_t(rcx) + 23i64 * 4)).c_str());
}