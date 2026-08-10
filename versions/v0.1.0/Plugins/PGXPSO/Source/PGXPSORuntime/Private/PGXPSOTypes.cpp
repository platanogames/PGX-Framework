// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXPSOTypes.h"

// ============================================================================
// FPGXPSOEntry
// ============================================================================

FPGXPSOKey FPGXPSOEntry::MakeKey() const
{
	FPGXPSOKey Key;
	// EN: Use full path to deduplicate materials with same name in different folders
	// ES: Usar ruta completa para deduplicar materiales con mismo nombre en carpetas distintas
	Key.MaterialPath = Material.IsNull() ? NAME_None : FName(*Material.ToSoftObjectPath().ToString());
	Key.VertexFactoryName = GetResolvedVertexFactoryName();
	Key.PassHint = PassHint;

	// EN: Pack boolean render parameters into a uint64 for fast comparison
	// ES: Empaquetar parametros booleanos de render en un uint64 para comparacion rapida
	uint64 Packed = 0;
	Packed |= (bRenderInMainPass   ? 1ULL : 0ULL) << 0;
	Packed |= (bRenderInDepthPass  ? 1ULL : 0ULL) << 1;
	Packed |= (bCastShadow         ? 1ULL : 0ULL) << 2;
	Packed |= (bSkinnedMesh        ? 1ULL : 0ULL) << 3;
	Packed |= (bRenderCustomDepth  ? 1ULL : 0ULL) << 4;
	Packed |= (bNaniteMesh         ? 1ULL : 0ULL) << 5;
	Packed |= (static_cast<uint64>(Mobility.GetValue()) & 0x3) << 6;
	Key.PackedParams = Packed;

	return Key;
}

FName FPGXPSOEntry::GetResolvedVertexFactoryName() const
{
	return PGXPSOUtils::ResolveVertexFactoryType(VertexFactory, CustomVertexFactoryName);
}

// ============================================================================
// PGXPSOUtils
// ============================================================================

FName PGXPSOUtils::ResolveVertexFactoryType(EPGXVertexFactoryType Type, FName CustomName)
{
	switch (Type)
	{
	case EPGXVertexFactoryType::StaticMesh:
		return FName(TEXT("FLocalVertexFactory"));
	case EPGXVertexFactoryType::SkeletalMesh:
		return FName(TEXT("FGPUSkinVertexFactory"));
	case EPGXVertexFactoryType::InstancedMesh:
		return FName(TEXT("FInstancedStaticMeshVertexFactory"));
	case EPGXVertexFactoryType::SplineMesh:
		return FName(TEXT("FSplineMeshVertexFactory"));
	case EPGXVertexFactoryType::Landscape:
		return FName(TEXT("FLandscapeVertexFactory"));
	case EPGXVertexFactoryType::NiagaraSprite:
		return FName(TEXT("FNiagaraSpriteVertexFactory"));
	case EPGXVertexFactoryType::NiagaraMesh:
		return FName(TEXT("FNiagaraMeshVertexFactory"));
	case EPGXVertexFactoryType::Custom:
		return CustomName.IsNone() ? FName(TEXT("FLocalVertexFactory")) : CustomName;
	default:
		return FName(TEXT("FLocalVertexFactory"));
	}
}
