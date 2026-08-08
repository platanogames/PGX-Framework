// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Tests/PGXSaveFailingTestProvider.h"

#if WITH_EDITORONLY_DATA

#include "Misc/Paths.h"

bool UPGXSaveFailingTestProvider::SaveBytes(const FString& /*FilePath*/, const TArray<uint8>& /*Bytes*/)
{
	// EN: Deterministic failure for active-slot safety test.
	// ES: Fallo deterministico para test active-slot safety.
	return false;
}

bool UPGXSaveFailingTestProvider::LoadBytes(const FString& /*FilePath*/, TArray<uint8>& OutBytes)
{
	OutBytes.Reset();
	return false;
}

bool UPGXSaveFailingTestProvider::DeleteFile(const FString& /*FilePath*/)
{
	return false;
}

bool UPGXSaveFailingTestProvider::DoesFileExist(const FString& /*FilePath*/) const
{
	return false;
}

TArray<FString> UPGXSaveFailingTestProvider::GetFilesInDirectory(
	const FString& /*DirectoryPath*/,
	const FString& /*Extension*/) const
{
	return {};
}

bool UPGXSaveFailingTestProvider::EnsureDirectoryExists(const FString& /*DirectoryPath*/)
{
	// EN: Returning true here lets the subsystem proceed past the directory
	//     gate so the failure surfaces specifically on SaveBytes (the
	//     contractually relevant point for the active-slot safety contract).
	// ES: Retornar true aqui permite que el subsistema avance pasando el gate
	//     de directorio para que el fallo se superficie especificamente en
	//     SaveBytes (el punto contractualmente relevante para el contrato de active-slot safety).
	return true;
}

int64 UPGXSaveFailingTestProvider::GetDiskSpaceAvailable() const
{
	// EN: Pretend infinite space so absence-of-space is not the failure cause.
	// ES: Pretender espacio infinito para que la ausencia de espacio no sea
	//     la causa del fallo.
	return TNumericLimits<int64>::Max();
}

FString UPGXSaveFailingTestProvider::GetBaseSaveDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames");
}

#endif // WITH_EDITORONLY_DATA
