// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXInputSettings.generated.h"

class UPGXInputConfig;

/**
 * EN: Discovery strategy for the active Input Config DA.
 *     AssetRegistryScan is a deprecated fallback shape; Manual is reserved.
 * ES: Estrategia de descubrimiento del Config DA activo de Input.
 */
UENUM(BlueprintType)
enum class EPGXInputDiscoveryMode : uint8
{
	/** EN: Scan AssetRegistry for UPGXInputConfig DAs (reserved fallback). */
	AssetRegistryScan UMETA(DisplayName = "Asset Registry Scan"),

	/** EN: [Reserved] Manual mode. Not consumed at runtime. */
	Manual            UMETA(DisplayName = "Manual (Reserved)")
};

/**
 * EN: Project Settings for the PGX Input system. The Settings surface declares the Settings-first
 *     shape only; runtime does not consume ActiveConfig.
 * ES: Project Settings del sistema PGX Input. La superficie Settings declara la forma
 *     Settings-first; runtime no consume ActiveConfig.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX Input System"))
class PGXINPUTRUNTIME_API UPGXInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** EN: Canonical Input config DA setting; declared but not consumed by runtime yet. */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "Active Config", ToolTip = "Declares the Settings-first shape; runtime does not consume this setting."))
	TSoftObjectPtr<UPGXInputConfig> ActiveConfig;

	/** EN: Reserved fallback discovery mode; runtime does not consume it. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ToolTip = "Declares the fallback shape; runtime does not perform this config resolution."))
	EPGXInputDiscoveryMode DiscoveryMode = EPGXInputDiscoveryMode::AssetRegistryScan;

	/** EN: Enable verbose logging for configuration resolution diagnostics. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
