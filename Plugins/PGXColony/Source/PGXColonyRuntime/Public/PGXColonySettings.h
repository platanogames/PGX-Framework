// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXColonySettings.generated.h"

class UPGXColonyConfig;

/**
 * Reserved project settings; Colony runtime does not currently resolve configuration through them.
 * These values are editable but are not consumed by the runtime subsystem yet.
 */
UENUM(BlueprintType)
enum class EPGXColonyDiscoveryMode : uint8
{
	/** EN: Scan AssetRegistry for UPGXColonyConfig DAs (deprecated; only consulted when ActiveConfig is unset). */
	AssetRegistryScan UMETA(DisplayName = "Asset Registry Scan"),

	/** EN: [Reserved] Manual mode — explicit config paths. Not consumed at runtime to runtime. */
	Manual            UMETA(DisplayName = "Manual (Reserved)")
};

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "PGX Colony System"))
class PGXCOLONYRUNTIME_API UPGXColonySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EN: Config Resolution
	// ES: Resolucion de Config
	// ========================================================================

	/** Reserved configuration asset reference; currently has no runtime effect. */
	UPROPERTY(config, EditAnywhere, Category = "Config",
		meta = (DisplayName = "Active Config"))
	TSoftObjectPtr<UPGXColonyConfig> ActiveConfig;

	// ========================================================================
	// Reserved discovery settings
	// ========================================================================

	/** Reserved discovery mode; neither mode is consumed by runtime yet. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery",
		meta = (ToolTip = "Reserved configuration discovery modes; neither has runtime effect."))
	EPGXColonyDiscoveryMode DiscoveryMode = EPGXColonyDiscoveryMode::AssetRegistryScan;

	/** EN: Enable verbose logging for config resolution. */
	UPROPERTY(config, EditAnywhere, Category = "Discovery|Debug",
		meta = (DisplayName = "Verbose Config Resolution"))
	bool bVerboseConfigResolution = false;

	// ========================================================================
	// EN: UDeveloperSettings Interface
	// ES: Interfaz UDeveloperSettings
	// ========================================================================

	FName GetCategoryName() const override { return TEXT("PGX"); }
};
