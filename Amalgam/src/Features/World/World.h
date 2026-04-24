#pragma once
#include "../../SDK/SDK.h"
#include <functional>

//#define WORLD_DEBUG

Enum(FaceType, None = 0, BoxBrush = 1 << 0, PlaneBrush = 1 << 1, Displacement = 1 << 2, Prop = 1 << 3, Entity = 1 << 4, All = BoxBrush | PlaneBrush | Displacement | Prop | Entity, Cache = BoxBrush | PlaneBrush | Prop | Entity)
#ifdef WORLD_DEBUG
Enum(DrawType, Points = 1 << 0, Edges = 1 << 1, Faces = 1 << 2, Normals = 1 << 3)

namespace Vars {
	NAMESPACE_BEGIN(World)
		CVarValues(Faces, "Faces", 0b11111, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
			"BoxBrush", "PlaneBrush", "Displacement", "Prop", "Entity");
		CVarValues(Draw, "Draw", 0b0110, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
			"Points", "Edges", "Faces", "Normals");

		CVar(Offset, "Offset", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, DIST_EPSILON);
		CVar(Resize, "Resize", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, DIST_EPSILON);

		CVar(BoxBrush, "BoxBrush", Color_t(255, 0, 0, 255), NOSAVE | DEBUGVAR);
		CVar(PlaneBrush, "PlaneBrush", Color_t(0, 255, 0, 255), NOSAVE | DEBUGVAR);
		CVar(Displacement, "Displacement", Color_t(0, 0, 255, 255), NOSAVE | DEBUGVAR);
		CVar(Prop, "Prop", Color_t(255, 0, 255, 255), NOSAVE | DEBUGVAR);
		CVar(Entity, "Entity", Color_t(255, 255, 0, 255), NOSAVE | DEBUGVAR);
	NAMESPACE_END(World)
}
#endif

using VerticesValidCallback = const std::function<bool(const std::vector<Vec3>&)>;
using BoundsValidCallback = const std::function<bool(const std::vector<Vec3>&, const Vec3&, const Vec3&)>;
using NormalValidCallback = const std::function<bool(const std::vector<Vec3>&, const Vec3&)>;

struct Plane_t
{
	Vec3 m_vNormal;
	float m_flDist;
};

struct Face_t
{
	std::vector<Vec3> m_vVertices;
	Vec3 m_vNormal;
	int8_t m_iType;
};

static VerticesValidCallback s_fVerticesValid = [](const std::vector<Vec3>& vVertices)
{
	return vVertices.size() >= 3;
};

static BoundsValidCallback s_fBoundsValid = [](const std::vector<Vec3>& vVertices, const Vec3& vMins, const Vec3& vMaxs)
{
	Vec3 vVerticeMins = Vec3::GetMax(), vVerticeMaxs = Vec3::GetMin();
	for (auto& vVertex : vVertices)
		vVerticeMins = vVerticeMins.Min(vVertex), vVerticeMaxs = vVerticeMaxs.Max(vVertex);
	return Math::IsBoxIntersectingBox(vVerticeMins, vVerticeMaxs, vMins, vMaxs);
};

static NormalValidCallback s_fNormalValid = [](const std::vector<Vec3>& vVertices, const Vec3& vNormal)
{
#ifdef WORLD_DEBUG
	Vec3 vAngles = I::EngineClient->GetViewAngles();
	Vec3 vForward; Math::AngleVectors(vAngles, &vForward);
	return vForward.Dot(vNormal) < 0;
#else
	return true;
#endif
};

class CWorld
{
private:
	bool FaceValid(const std::vector<Vec3>& vVertices, const Vec3* pNormal, const Vec3* pMins = nullptr, const Vec3* pMaxs = nullptr);

	VerticesValidCallback* m_pVerticesValidCallback = &s_fVerticesValid;
	BoundsValidCallback* m_pBoundsValidCallback = &s_fBoundsValid;
	NormalValidCallback* m_pNormalValidCallback = &s_fNormalValid;

	int m_iNeedsCache = FaceTypeEnum::Cache;
	std::unordered_map<void*, std::vector<Face_t>> m_mFaceCache = {};

public:
	void Cache();
	void CacheBoxBrushes();
	void CachePlaneBrushes();
	void CacheProps();
	void CacheEntities();
	void Uncache();
	void UncacheBoxBrushes();
	void UncachePlaneBrushes();
	void UncacheProps();
	void UncacheEntities();

	std::vector<Face_t> GetFacesInAABB(const Vec3& vMins, const Vec3& vMaxs, int iMask = MASK_SOLID, ITraceFilter* pFilter = nullptr, int iFlags = FaceTypeEnum::All);

	void SetVerticesValidCallback(VerticesValidCallback* pCallback = nullptr);
	void SetBoundsValidCallback(BoundsValidCallback* pCallback = nullptr);
	void SetNormalValidCallback(NormalValidCallback* pCallback = nullptr);

#ifdef WORLD_DEBUG
	void DrawFace(const Face_t& tFace, int iFlags = DrawTypeEnum::Points | DrawTypeEnum::Edges | DrawTypeEnum::Faces | DrawTypeEnum::Normals, std::optional<Color_t> tColor = std::nullopt);
#endif
};

ADD_FEATURE(CWorld, World);