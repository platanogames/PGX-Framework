// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PGXEditorSettings.generated.h"

/**
 * EN: Per-user editor settings for PGX Framework.
 *     Configurable via Project Settings > PGX > Editor.
 *     Stored in EditorPerProjectUserSettings (not committed to source control).
 *
 * ES: Configuracion de editor por usuario para PGX Framework.
 *     Configurable via Project Settings > PGX > Editor.
 *     Almacenado en EditorPerProjectUserSettings (no se commitea).
 */
UCLASS(config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "PGX Editor"))
class PGXCOREEDITOR_API UPGXEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPGXEditorSettings();

	// EN: UDeveloperSettings interface / ES: Interfaz UDeveloperSettings
	FName GetCategoryName() const override { return FName(TEXT("PGX")); }
	FName GetSectionName() const override { return FName(TEXT("Editor")); }
	FText GetSectionText() const override;
	FText GetSectionDescription() const override;

	// ─── Quick Access Toolbar Pins ───

	// ── Core ──

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Core",
		meta = (DisplayName = "Pin: PGX Hub"))
	bool bPinHub = true;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Core",
		meta = (DisplayName = "Pin: Restart Editor"))
	bool bPinRestart = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Core",
		meta = (DisplayName = "Pin: Documentation"))
	bool bPinDocs = false;

	// ── System Inspectors ──

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Log Viewer"))
	bool bPinLogViewer = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Save Inspector"))
	bool bPinSaveInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: GameFlow Inspector"))
	bool bPinGameFlowInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Audio Inspector"))
	bool bPinAudioInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: MGOS Inspector"))
	bool bPinMGOSInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Message Inspector"))
	bool bPinMessageInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: PSO Inspector"))
	bool bPinPSOInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: LevelFlow Inspector"))
	bool bPinLevelFlowInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Loading Inspector"))
	bool bPinLoadingInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Profile Inspector"))
	bool bPinProfileInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|System Inspectors",
		meta = (DisplayName = "Pin: Event Debugger"))
	bool bPinEventDebugger = false;

	// ── Pipeline and Health ──

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Pipeline and Health",
		meta = (DisplayName = "Pin: Platform Health Dashboard"))
	bool bPinPlatformHealth = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Pipeline and Health",
		meta = (DisplayName = "Pin: Config Dashboard"))
	bool bPinConfigDashboard = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Pipeline and Health",
		meta = (DisplayName = "Pin: Data Registry Browser"))
	bool bPinDataRegistryBrowser = false;

	// ── Development Tools ──

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Development Tools",
		meta = (DisplayName = "Pin: System Observer"))
	bool bPinSystemObserver = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Development Tools",
		meta = (DisplayName = "Pin: Test Dashboard"))
	bool bPinTestDashboard = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Development Tools",
		meta = (DisplayName = "Pin: Version Control Inspector"))
	bool bPinVersionControlInspector = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Development Tools",
		meta = (DisplayName = "Pin: PGX Scaffold"))
	bool bPinScaffold = false;

	UPROPERTY(config, EditAnywhere, Category = "Quick Access Toolbar|Development Tools",
		meta = (DisplayName = "Pin: Tutorial Hub"))
	bool bPinTutorialHub = false;

	// ─── Visual Theme — System Colors ───
	// EN: Override system identity colors. Panels using GetColorByName() will reflect changes on next open.
	// ES: Override de colores de identidad de sistema. Paneles usando GetColorByName() reflejaran cambios al reabrir.

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Save"))
	FLinearColor ColorSave = FLinearColor(0.298f, 0.686f, 0.314f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "GameFlow"))
	FLinearColor ColorGameFlow = FLinearColor(1.0f, 0.596f, 0.0f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "PSO"))
	FLinearColor ColorPSO = FLinearColor(0.0f, 0.737f, 0.831f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "LevelFlow"))
	FLinearColor ColorLevelFlow = FLinearColor(0.129f, 0.588f, 0.953f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Loading"))
	FLinearColor ColorLoading = FLinearColor(0.914f, 0.118f, 0.388f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Profile"))
	FLinearColor ColorProfile = FLinearColor(1.0f, 0.757f, 0.027f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "MGOS"))
	FLinearColor ColorMGOS = FLinearColor(0.498f, 0.0f, 1.0f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Documentation"))
	FLinearColor ColorDocumentation = FLinearColor(0.0f, 0.545f, 0.545f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Audio"))
	FLinearColor ColorAudio = FLinearColor(1.0f, 0.596f, 0.0f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Construction"))
	FLinearColor ColorConstruction = FLinearColor(0.0f, 0.502f, 0.502f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Data Registry"))
	FLinearColor ColorDataRegistry = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Config"))
	FLinearColor ColorConfig = FLinearColor(0.4f, 0.902f, 0.302f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Message"))
	FLinearColor ColorMessage = FLinearColor(0.612f, 0.153f, 0.690f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Event Handler"))
	FLinearColor ColorEventHandler = FLinearColor(0.957f, 0.263f, 0.212f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Log"))
	FLinearColor ColorLog = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

	UPROPERTY(config, EditAnywhere, Category = "Visual Theme|System Colors",
		meta = (DisplayName = "Scaffold"))
	FLinearColor ColorScaffold = FLinearColor(1.0f, 0.749f, 0.0f, 1.0f);

	// ─── Asset Creation ───

	UPROPERTY(config, EditAnywhere, Category = "Asset Creation",
		meta = (DisplayName = "Default Blueprint Path", ContentDir))
	FDirectoryPath DefaultBlueprintPath;
};
