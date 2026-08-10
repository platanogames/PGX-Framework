// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXInputSettings.generated.h"

class UPGXInputConfig;

/**
 * EN: Discovery strategy for the active Input Config DA.
 *     Runtime config selection is explicit through ActiveConfig; this legacy mode is retained for compatibility.
 * ES: Estrategia de descubrimiento del Config DA activo de Input.
 */
UENUM(BlueprintType)
enum class EPGXInputDiscoveryMode : uint8
{
	/** EN: Scan AssetRegistry for UPGXInputConfig DAs (reserved fallback). */
	AssetRegistryScan UMETA(DisplayName = "Asset Registry Scan"),

	/** EN: Manual mode retained for configuration compatibility. */
	Manual            UMETA(DisplayName = "Manual (Reserved)")
};

/**
 * EN: Project Settings for the PGX Input system. ActiveConfig is resolved before runtime objects
 *     are initialized. An unset or unloadable reference uses a transient safe default.
 * ES: Project Settings del sistema PGX Input. La superficie Settings declara la forma
 *     Settings-first; runtime resuelve ActiveConfig antes de crear sus objetos auxiliares.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX Input System"))
class PGXINPUTRUNTIME_API UPGXInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** EN: Canonical Input config asset resolved by UPGXInputSubsystem during Initialize. */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "Active Config", ToolTip = "Input config loaded during subsystem initialization; unset or invalid references use safe defaults."))
	TSoftObjectPtr<UPGXInputConfig> ActiveConfig;

	/** EN: Legacy discovery setting retained for config compatibility; runtime never scans AssetRegistry. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ToolTip = "Legacy compatibility setting. Runtime config resolution is explicit through Active Config."))
	EPGXInputDiscoveryMode DiscoveryMode = EPGXInputDiscoveryMode::AssetRegistryScan;

	/** EN: Enable verbose logging for configuration resolution diagnostics. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
