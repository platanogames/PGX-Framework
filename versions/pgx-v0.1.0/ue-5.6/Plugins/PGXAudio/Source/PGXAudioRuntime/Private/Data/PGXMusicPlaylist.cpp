// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Data/PGXMusicPlaylist.h"
#include "Observability/PGXCoreObservability.h"

FPGXJsonValue UPGXMusicPlaylist::ToJson() const
{
	return PGXCoreObservability::MakeJsonEnvelope(this, GetSchemaVersion());
}

FPGXValidationResult UPGXMusicPlaylist::FromJson(const FPGXJsonValue& Json)
{
	return PGXCoreObservability::ValidateJsonEnvelope(Json);
}

FName UPGXMusicPlaylist::GetSchemaVersion() const
{
	return TEXT("1.0");
}

FPGXSchemaDescriptor UPGXMusicPlaylist::GetSchemaDescriptor() const
{
	return PGXCoreObservability::MakeSchemaDescriptor(this, GetSchemaVersion());
}
