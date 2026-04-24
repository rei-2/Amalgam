#include "../World/World.h"

#include <numeric>

class CEntityEnumerator : public IPartitionEnumerator
{
public:
	IterationRetval_t EnumElement(IHandleEntity* pHandleEntity)
	{
		m_vEntities.push_back(pHandleEntity);
		return ITERATION_CONTINUE;
	}

	std::vector<IHandleEntity*>	m_vEntities = {};
};

void CWorld::SetVerticesValidCallback(VerticesValidCallback* pCallback)
{
	m_pVerticesValidCallback = pCallback ? pCallback : &s_fVerticesValid;
}

void CWorld::SetBoundsValidCallback(BoundsValidCallback* pCallback)
{
	m_pBoundsValidCallback = pCallback ? pCallback : &s_fBoundsValid;
}

void CWorld::SetNormalValidCallback(NormalValidCallback* pCallback)
{
	m_pNormalValidCallback = pCallback ? pCallback : &s_fNormalValid;
}

bool CWorld::FaceValid(const std::vector<Vec3>& vVertices, const Vec3* pNormal, const Vec3* pMins, const Vec3* pMaxs)
{
	if (!(*m_pVerticesValidCallback)(vVertices))
		return false;

	if (pMins && pMaxs && !(*m_pBoundsValidCallback)(vVertices, *pMins, *pMaxs))
		return false;

	if (pNormal && !(*m_pNormalValidCallback)(vVertices, *pNormal))
		return false;

	return true;
}



static CPhysConvex* s_aConvexes[1024];
static inline void GetFacesForConvex(CPhysConvex* pConvex, std::vector<Face_t>& vFaces, float flResize = 0.f, int8_t iType = FaceTypeEnum::None)
{
	CPolyhedron* pPolyhedron = I::PhysicsCollision->PolyhedronFromConvex(pConvex, true);
	if (!pPolyhedron)
		return;

	if (!flResize)
	{
		std::vector<std::pair<std::vector<int>, Vec3>> vTemp = {};
		for (int iPolygon = 0; iPolygon < pPolyhedron->iPolygonCount; iPolygon++)
		{
			Polyhedron_IndexedPolygon_t* pPolygon = &pPolyhedron->pPolygons[iPolygon];

			std::vector<int>* pVertices = nullptr;
			for (auto& [vVertices, vNormal] : vTemp)
			{
				if (pPolygon->polyNormal.Dot(vNormal) > 1 - CALC_EPSILON)
				{
					pVertices = &vVertices;
					break;
				}
			}
			if (!pVertices)
			{
				vTemp.emplace_back(std::vector<int>(), pPolygon->polyNormal);
				pVertices = &vTemp.back().first;
			}

			for (int iLineRef = 0; iLineRef < pPolygon->iIndexCount; iLineRef++)
			{
				Polyhedron_IndexedLineReference_t* pLineReference = &pPolyhedron->pIndices[pPolygon->iFirstIndex + iLineRef];
				Polyhedron_IndexedLine_t* pLine = &pPolyhedron->pLines[pLineReference->iLineIndex];
				int iVertex = pLine->iPointIndices[pLineReference->iEndPointIndex];
				if (std::find(pVertices->begin(), pVertices->end(), iVertex) == pVertices->end())
					pVertices->push_back(iVertex);
			}
		}

		for (auto& [vIndices, vNormal] : vTemp)
		{
			if (vIndices.size() < 3)
				continue;

			std::vector<Vec3> vVertices = {};
			for (auto& iVertex : vIndices)
				vVertices.push_back(pPolyhedron->pVertices[iVertex]);

			if (vVertices.size() > 3)
			{
				Vec3 vCenter = std::reduce(vVertices.begin(), vVertices.end()) / vVertices.size();
				Vec3 vDir1 = (vVertices.front() - vCenter).Normalized();
				Vec3 vDir2 = vDir1.Cross(vNormal);
				std::sort(vVertices.begin(), vVertices.end(), [&](const Vec3& a, const Vec3& b) -> bool
				{
					return atan2f(vDir2.Dot(a - vCenter), vDir1.Dot(a - vCenter)) < atan2f(vDir2.Dot(b - vCenter), vDir1.Dot(b - vCenter));
				});
			}

			vFaces.emplace_back(vVertices, vNormal, iType);
		}
	}
	else
	{	// rough approximation, not too sure why this is needed in the first place
		std::vector<Plane_t> vPlanes = {}; vPlanes.reserve(pPolyhedron->iPolygonCount);
		for (int iPolygon = 0; iPolygon < pPolyhedron->iPolygonCount; iPolygon++)
		{
			Polyhedron_IndexedPolygon_t* pPolygon = &pPolyhedron->pPolygons[iPolygon];

			if (pPolygon->iIndexCount)
			{
				Polyhedron_IndexedLineReference_t* pLineReference = &pPolyhedron->pIndices[pPolygon->iFirstIndex];
				Polyhedron_IndexedLine_t* pLine = &pPolyhedron->pLines[pLineReference->iLineIndex];
				Vec3& vVertex = pPolyhedron->pVertices[pLine->iPointIndices[pLineReference->iEndPointIndex]];
				vPlanes.emplace_back(pPolygon->polyNormal, pPolygon->polyNormal.Dot(vVertex + flResize * pPolygon->polyNormal));
			}
		}

		if (CPhysConvex* pConvex2 = I::PhysicsCollision->ConvexFromPlanes(reinterpret_cast<float*>(vPlanes.data()), int(vPlanes.size()), 0.f))
		{
			GetFacesForConvex(pConvex2, vFaces, 0.f, iType);
			I::PhysicsCollision->ConvexFree(pConvex2);
		}
	}

	pPolyhedron->Release();
}

static inline void GetFacesForCollision(CPhysCollide* pCollision, std::vector<Face_t>& vFaces, float flResize = 0.f, int8_t iType = FaceTypeEnum::None)
{
	int iConvexes = I::PhysicsCollision->GetConvexesUsedInCollideable(pCollision, s_aConvexes, sizeof(s_aConvexes) / sizeof(CPhysConvex*));
	for (int iConvex = 0; iConvex < iConvexes; iConvex++)
		GetFacesForConvex(s_aConvexes[iConvex], vFaces, flResize, iType);

	/*
	Vec3* pVertices; int iVertices = I::PhysicsCollision->CreateDebugMesh(pCollision, &pVertices); iVertices -= iVertices % 3;
	for (int iVertex = 0; iVertex < iVertices; iVertex += 3)
	{
		std::vector<Vec3> vVertices = { pVertices[iVertex], pVertices[iVertex + 1], pVertices[iVertex + 2] };
		Vec3 vNormal = (vVertices[0] - vVertices[2]).Cross(vVertices[0] - vVertices[1]).Normalized();

		vFaces.emplace_back(vVertices, vNormal, iType);
	}
	I::PhysicsCollision->DestroyDebugMesh(iVertices, pVertices);
	*/
}

void CWorld::Cache()
{
	if (!m_iNeedsCache)
		return;

	CacheBoxBrushes();
	CachePlaneBrushes();
	CacheProps();
	CacheEntities();
}

void CWorld::CacheBoxBrushes()
{
	if (!(m_iNeedsCache & FaceTypeEnum::BoxBrush))
		return;

	SDK::Output(__FUNCTION__, "Cache start", {}, OUTPUT_DEBUG);

	m_iNeedsCache &= ~FaceTypeEnum::BoxBrush;

	for (int iBrush = 0; iBrush < I::BSPData->numbrushes; iBrush++)
	{
		cbrush_t* pBrush = &I::BSPData->map_brushes[iBrush];
		if (!pBrush->IsBox())
			continue;

		cboxbrush_t* pBoxBrush = &I::BSPData->map_boxbrushes[pBrush->GetBox()];

		Vec3 aVertices[8];
		for (int i = 0; i != 8; i++)
		{
			aVertices[i].x = i & (1 << 0) ? pBoxBrush->maxs.x : pBoxBrush->mins.x;
			aVertices[i].y = i & (1 << 1) ? pBoxBrush->maxs.y : pBoxBrush->mins.y;
			aVertices[i].z = i & (1 << 2) ? pBoxBrush->maxs.z : pBoxBrush->mins.z;
		}
		Vec3 aNormals[6] = {
			{ 1, 0, 0 },
			{ -1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, -1, 0 },
			{ 0, 0, 1 },
			{ 0, 0, -1 }
		};

		for (int i = 0; i < 6; i++)
		{
			if (I::BSPData->map_surfaces[pBoxBrush->surfaceIndex[i]].flags & SURF_SKY)
				continue;

			std::vector<Vec3> vVertices;
			switch (i)
			{
			case 0: vVertices = { aVertices[1], aVertices[5], aVertices[7], aVertices[3] }; break;
			case 1: vVertices = { aVertices[2], aVertices[6], aVertices[4], aVertices[0] }; break;
			case 2: vVertices = { aVertices[3], aVertices[7], aVertices[6], aVertices[2] }; break;
			case 3: vVertices = { aVertices[0], aVertices[4], aVertices[5], aVertices[1] }; break;
			case 4: vVertices = { aVertices[6], aVertices[7], aVertices[5], aVertices[4] }; break;
			case 5: vVertices = { aVertices[0], aVertices[1], aVertices[3], aVertices[2] }; break;
			}
			Vec3& vNormal = aNormals[i];

			m_mFaceCache[pBrush].emplace_back(vVertices, vNormal, FaceTypeEnum::BoxBrush);
		}
	}

	SDK::Output(__FUNCTION__, "Cache finish", {}, OUTPUT_DEBUG);
}

void CWorld::CachePlaneBrushes()
{
	if (!(m_iNeedsCache & FaceTypeEnum::PlaneBrush))
		return;

	SDK::Output(__FUNCTION__, "Cache start", {}, OUTPUT_DEBUG);

	m_iNeedsCache &= ~FaceTypeEnum::PlaneBrush;

	for (int iBrush = 0; iBrush < I::BSPData->numbrushes; iBrush++)
	{
		cbrush_t* pBrush = &I::BSPData->map_brushes[iBrush];
		if (pBrush->IsBox())
			continue;

		std::vector<Plane_t> vPlanes = {}; vPlanes.reserve(pBrush->numsides);
		for (int iSide = 0; iSide != pBrush->numsides; iSide++)
		{
			cbrushside_t* pBrushSide = &I::BSPData->map_brushsides[pBrush->firstbrushside + iSide];
			if (I::BSPData->map_surfaces[pBrushSide->surfaceIndex].flags & SURF_SKY)
				continue;

			cplane_t* pPlane = pBrushSide->plane;
			vPlanes.emplace_back(pPlane->normal, pPlane->dist);
		}
		CPhysConvex* pConvex = I::PhysicsCollision->ConvexFromPlanes(reinterpret_cast<float*>(vPlanes.data()), int(vPlanes.size()), 0.f);
		if (!pConvex)
			continue;

		GetFacesForConvex(pConvex, m_mFaceCache[pBrush], 0.f, FaceTypeEnum::PlaneBrush);
		I::PhysicsCollision->ConvexFree(pConvex);
	}

	SDK::Output(__FUNCTION__, "Cache finish", {}, OUTPUT_DEBUG);
}

void CWorld::CacheProps()
{
	if (!(m_iNeedsCache & FaceTypeEnum::Prop))
		return;

	SDK::Output(__FUNCTION__, "Cache start", {}, OUTPUT_DEBUG);

	m_iNeedsCache &= ~FaceTypeEnum::Prop;

	for (int i = I::MDLCache->m_MDLDict.First(); i != I::MDLCache->m_MDLDict.InvalidIndex(); i = I::MDLCache->m_MDLDict.Next(i))
	{
		studiodata_t* pStudioData = I::MDLCache->m_MDLDict[i];
		if (!pStudioData)
			continue;

		vcollide_t* pCollide = &pStudioData->m_VCollisionData;
		if (m_mFaceCache.contains(pCollide))
			continue;

		for (int iSolid = 0; iSolid < pCollide->solidCount; iSolid++)
		{
			CPhysCollide* pCollision = pCollide->solids[iSolid];
			if (!pCollision)
				continue;

			GetFacesForCollision(pCollision, m_mFaceCache[pCollide], 0.f, FaceTypeEnum::Prop);
		}
	}

	SDK::Output(__FUNCTION__, "Cache finish", {}, OUTPUT_DEBUG);
}

void CWorld::CacheEntities()
{
	if (!(m_iNeedsCache & FaceTypeEnum::Entity))
		return;

	SDK::Output(__FUNCTION__, "Cache start", {}, OUTPUT_DEBUG);

	m_iNeedsCache &= ~FaceTypeEnum::Entity;

	for (int iCModel = 0; iCModel < I::BSPData->numcmodels; iCModel++)
	{
		cmodel_t* pCModel = &I::BSPData->map_cmodels[iCModel];
		vcollide_t* pCollide = &pCModel->vcollisionData;

		for (int iSolid = 0; iSolid < pCollide->solidCount; iSolid++)
		{
			CPhysCollide* pCollision = pCollide->solids[iSolid];
			if (!pCollision)
				continue;

			GetFacesForCollision(pCollision, m_mFaceCache[pCollide], 0.5f, FaceTypeEnum::Entity);
		}
	}

	SDK::Output(__FUNCTION__, "Cache finish", {}, OUTPUT_DEBUG);
}



void CWorld::Uncache()
{
	if (m_iNeedsCache == FaceTypeEnum::Cache)
		return;

	m_mFaceCache.clear();

	m_iNeedsCache = FaceTypeEnum::Cache;
}

void CWorld::UncacheBoxBrushes()
{
	if (m_iNeedsCache & FaceTypeEnum::BoxBrush)
		return;

	for (auto& [pKey, vFaces] : m_mFaceCache)
	{
		if (vFaces.front().m_iType == FaceTypeEnum::BoxBrush)
			m_mFaceCache.erase(pKey);
	}

	m_iNeedsCache |= FaceTypeEnum::BoxBrush;
}

void CWorld::UncachePlaneBrushes()
{
	if (m_iNeedsCache & FaceTypeEnum::PlaneBrush)
		return;

	for (auto& [pKey, vFaces] : m_mFaceCache)
	{
		if (vFaces.front().m_iType == FaceTypeEnum::PlaneBrush)
			m_mFaceCache.erase(pKey);
	}

	m_iNeedsCache |= FaceTypeEnum::PlaneBrush;
}

void CWorld::UncacheProps()
{
	if (m_iNeedsCache & FaceTypeEnum::Prop)
		return;

	for (auto& [pKey, vFaces] : m_mFaceCache)
	{
		if (vFaces.front().m_iType == FaceTypeEnum::Prop)
			m_mFaceCache.erase(pKey);
	}

	m_iNeedsCache |= FaceTypeEnum::Prop;
}

void CWorld::UncacheEntities()
{
	if (m_iNeedsCache & FaceTypeEnum::Entity)
		return;

	for (auto& [pKey, vFaces] : m_mFaceCache)
	{
		if (vFaces.front().m_iType == FaceTypeEnum::Entity)
			m_mFaceCache.erase(pKey);
	}

	m_iNeedsCache |= FaceTypeEnum::Entity;
}



std::vector<Face_t> CWorld::GetFacesInAABB(const Vec3& vMins, const Vec3& vMaxs, int iMask, ITraceFilter* pFilter, int iFlags)
{
	Cache();

	std::vector<Face_t> vFaces;

	if (iFlags & (FaceTypeEnum::BoxBrush | FaceTypeEnum::PlaneBrush))
	{
		CUtlVector<int> vBrushes; I::EngineTrace->GetBrushesInAABB(vMins, vMaxs, &vBrushes, iMask);
		for (int iBrush = 0; iBrush < vBrushes.Count(); iBrush++)
		{
			cbrush_t* pBrush = &I::BSPData->map_brushes[vBrushes[iBrush]];
			if (!m_mFaceCache.contains(pBrush))
				continue;

			bool bBox = pBrush->IsBox();
			if (!(iFlags & (bBox ? FaceTypeEnum::BoxBrush : FaceTypeEnum::PlaneBrush)))
				continue;

			auto& vCache = m_mFaceCache[pBrush];
			if (bBox)
			{
				for (auto& tFace : vCache)
				{
					if (FaceValid(tFace.m_vVertices, &tFace.m_vNormal, &vMins, &vMaxs))
					{
						vFaces.push_back(tFace);
						for (auto& vVertex : vFaces.back().m_vVertices)
							vVertex = vVertex.Clamp(vMins, vMaxs);
					}
				}
			}
			else
			{
				for (auto& tFace : vCache)
				{
					if (FaceValid(tFace.m_vVertices, &tFace.m_vNormal, &vMins, &vMaxs))
						vFaces.push_back(tFace);
				}
			}
		}
	}

	if (iFlags & FaceTypeEnum::Displacement && *I::DispCollTrees)
	{
		for (int iDisplacement = 0; iDisplacement < *I::DispCollTreeCount; iDisplacement++)
		{
			CDispCollTree* pDispTree = &(*I::DispCollTrees)[iDisplacement];
			if (!Math::IsBoxIntersectingBox(vMins, vMaxs, pDispTree->m_mins, pDispTree->m_maxs))
				continue;

			int iFaces = pDispTree->GetFaceCount();
			for (int iFace = 0; iFace < iFaces; iFace++)
			{
				CDispCollTri* pTri = &pDispTree->m_aTris[iFace];
						
				std::vector<Vec3> vVertices = { pDispTree->m_aVerts[pTri->GetVert(0)], pDispTree->m_aVerts[pTri->GetVert(1)], pDispTree->m_aVerts[pTri->GetVert(2)] };

				if (FaceValid(vVertices, &pTri->m_vecNormal, &vMins, &vMaxs))
					vFaces.emplace_back(vVertices, pTri->m_vecNormal, FaceTypeEnum::Displacement);
			}
		}
	}

	if (iFlags & (FaceTypeEnum::Entity | FaceTypeEnum::Prop))
	{
		CEntityEnumerator tEnumerator; I::SpatialPartition->EnumerateElementsInBox(I::EngineTrace->SpatialPartitionMask(), vMins, vMaxs, false, &tEnumerator);
		for (auto pHandleEntity : tEnumerator.m_vEntities)
		{
			if (!pFilter->ShouldHitEntity(pHandleEntity, iMask))
				continue;

			bool bProp = I::StaticPropMgr->IsStaticProp(pHandleEntity);
			if (!(iFlags & (bProp ? FaceTypeEnum::Prop : FaceTypeEnum::Entity)))
				continue;

			ICollideable* pCollideable = nullptr;
			if (!I::StaticPropMgr->IsStaticProp(pHandleEntity))
			{
				CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pHandleEntity);
				if (!pEntity->GetAbsVelocity().IsZero())
					continue;

				pCollideable = pEntity->GetCollideable();
			}
			else
				pCollideable = I::StaticPropMgr->GetStaticProp(pHandleEntity);
			if (!pCollideable || !pCollideable->IsSolid())
				continue;

			vcollide_t* pCollide = I::ModelInfoClient->GetVCollide(pCollideable->GetCollisionModel());
			if (!m_mFaceCache.contains(pCollide))
				continue; // cache as well?

			auto& vCache = m_mFaceCache[pCollide];
			for (auto& tFace : vCache)
			{
				std::vector<Vec3> vVertices = tFace.m_vVertices;
				Vec3 vNormal = tFace.m_vNormal;
				if (!pCollideable->GetCollisionAngles())
				{
					for (auto& vVertex : vVertices)
						vVertex += pCollideable->GetCollisionOrigin();
				}
				else
				{
					for (auto& vVertex : vVertices)
						vVertex = Math::VectorTransform(vVertex, pCollideable->CollisionToWorldTransform());
					vNormal = (vVertices[0] - vVertices[2]).Cross(vVertices[0] - vVertices[1]).Normalized();
				}

				if (FaceValid(vVertices, &vNormal, &vMins, &vMaxs))
					vFaces.emplace_back(vVertices, vNormal, tFace.m_iType);
			}
		}
	}

	return vFaces;
}

#ifdef WORLD_DEBUG
void CWorld::DrawFace(const Face_t& tFace, int iFlags, std::optional<Color_t> tColor)
{
	if (!tColor)
	{
		switch (tFace.m_iType)
		{
		case FaceTypeEnum::BoxBrush: tColor = Vars::World::BoxBrush.Value; break;
		case FaceTypeEnum::PlaneBrush: tColor = Vars::World::PlaneBrush.Value; break;
		case FaceTypeEnum::Displacement: tColor = Vars::World::Displacement.Value; break;
		case FaceTypeEnum::Prop: tColor = Vars::World::Prop.Value; break;
		case FaceTypeEnum::Entity: tColor = Vars::World::Entity.Value; break;
		default: tColor = { 255, 255, 255 }; break;
		}
	}

	if (iFlags & (DrawTypeEnum::Points | DrawTypeEnum::Edges))
	{
		for (int i = 0; i < tFace.m_vVertices.size(); i++)
		{
			const Vec3& vVertex1 = tFace.m_vVertices[i], &vVertex2 = tFace.m_vVertices[(i + 1) % tFace.m_vVertices.size()];

			if (iFlags & DrawTypeEnum::Points)
				G::BoxStorage.emplace_back(vVertex1, Vec3::Get(-1), Vec3::Get(1), Vec3(0, 0, 0), I::GlobalVars->curtime + 60.f, *tColor, Color_t(0, 0, 0, 0), true);
			if (iFlags & DrawTypeEnum::Edges)
				G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(vVertex1, vVertex2), I::GlobalVars->curtime + 60.f, *tColor, true);
		}
	}

	if (iFlags & DrawTypeEnum::Faces)
	{
		for (int i = 0; ++i < tFace.m_vVertices.size() - 1;)
		{
			const Vec3& vVertex1 = tFace.m_vVertices[0], &vVertex2 = tFace.m_vVertices[i], &vVertex3 = tFace.m_vVertices[i + 1];
			G::TriangleStorage.emplace_back(std::array<Vec3, 3>({ vVertex1, vVertex2, vVertex3 }), I::GlobalVars->curtime + 60.f, tColor->Alpha(50), true);
		}
	}

	if (iFlags & DrawTypeEnum::Normals)
	{
		G::LineStorage.emplace_back(std::pair<Vec3, Vec3>(tFace.m_vVertices[0], tFace.m_vVertices[0] + tFace.m_vNormal * 10), I::GlobalVars->curtime + 60.f, tColor->Inverse(), true);
		G::BoxStorage.emplace_back(tFace.m_vVertices[0] + tFace.m_vNormal * 10, Vec3::Get(-1), Vec3::Get(1), Vec3(), I::GlobalVars->curtime + 60.f, tColor->Inverse(), Color_t(0, 0, 0, 0), true);
	}
}
#endif