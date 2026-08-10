// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "PGXSaveProvider.h"
#include "PGXSaveFailingTestProvider.generated.h"

#if WITH_EDITORONLY_DATA

/**
 * EN: Test-only provider that fails every write, succeeds nothing else.
 *     Used by active-slot safety failing test to deterministically
 *     produce a save failure. UCLASS gated by WITH_EDITORONLY_DATA — UHT
 *     accepts this guard specifically (UCLASS may not sit inside arbitrary
 *     preprocessor blocks; WITH_EDITORONLY_DATA is the supported exception).
 *     Editor builds (Development/Debug) compile this in; Shipping compiles
 *     it out, removing the symbol from runtime production binaries.
 *
 * ES: Provider solo-test que falla toda escritura, no aporta exitos.
 *     Usado por el failing test active-slot safety para producir
 *     un fallo de save deterministico. UCLASS bajo WITH_EDITORONLY_DATA — UHT
 *     acepta este guard especificamente (UCLASS no puede estar en bloques
 *     preprocessor arbitrarios; WITH_EDITORONLY_DATA es la excepcion soportada).
 *     Builds Editor (Development/Debug) compilan esto adentro; Shipping lo
 *     compila fuera, removiendo el simbolo de los binarios production.
 */
UCLASS(NotPlaceable, Transient)
class UPGXSaveFailingTestProvider : public UPGXSaveProvider
{
	GENERATED_BODY()

public:
	// EN: Always returns false to simulate provider write failure.
	// ES: Siempre retorna false para simular fallo de escritura del provider.
	virtual bool SaveBytes(const FString& FilePath, const TArray<uint8>& Bytes) override;

	// EN: Always returns false; never used in failing-write tests but required override.
	// ES: Siempre retorna false; nunca usado en tests de failing-write pero override requerido.
	virtual bool LoadBytes(const FString& FilePath, TArray<uint8>& OutBytes) override;

	virtual bool DeleteFile(const FString& FilePath) override;
	virtual bool DoesFileExist(const FString& FilePath) const override;

	virtual TArray<FString> GetFilesInDirectory(
		const FString& DirectoryPath,
		const FString& Extension = TEXT("")) const override;

	virtual bool EnsureDirectoryExists(const FString& DirectoryPath) override;

	virtual int64 GetDiskSpaceAvailable() const override;

	virtual FString GetBaseSaveDirectory() const override;
};

#endif // WITH_EDITORONLY_DATA
