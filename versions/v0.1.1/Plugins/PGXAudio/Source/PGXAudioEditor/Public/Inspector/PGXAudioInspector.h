// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UPGXAudioSubsystem;
class UPGXAudioMixSubsystem;
struct FPGXAudioChannelSnapshot;
struct FPGXActiveSoundInfo;
struct FPGXAudioEventRecord;

/**
 * EN: PGX Audio Inspector — NomadTab with 12 sections for full audio system observability.
 *     Universal shell (2px orange bar, KPI chips, footer).
 *     Channel Mixer, Active Sounds, Event History as SListView tables.
 *     Toolbar: Stop All + Mute All. Tick-based 0.5s refresh.
 *
 * ES: Inspector de Audio PGX — NomadTab con 12 secciones para observabilidad completa del sistema de audio.
 *     Shell universal (barra naranja 2px, KPI chips, footer).
 *     Channel Mixer, Active Sounds, Event History como tablas SListView.
 *     Toolbar: Stop All + Mute All. Refresco por Tick 0.5s.
 */
class SPGXAudioInspector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXAudioInspector) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SPGXAudioInspector() override;

	/** EN: Static: Register the NomadTab spawner / ES: Estatico: Registrar el spawner del NomadTab */
	static void RegisterTabSpawner();

	/** EN: Static: Unregister the NomadTab spawner / ES: Estatico: Desregistrar el spawner del NomadTab */
	static void UnregisterTabSpawner();

	/** EN: Tab ID / ES: ID del Tab */
	static const FName TabId;

private:
	/** EN: Spawn the tab / ES: Crear el tab */
	static TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);

	/** EN: Periodic refresh / ES: Refresco periodico */
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// ── Section Builders ─────────────────────────────────────────────────
	TSharedRef<SWidget> BuildSystemStatusSection();
	TSharedRef<SWidget> BuildChannelMixerSection();
	TSharedRef<SWidget> BuildActiveSoundsSection();
	TSharedRef<SWidget> BuildMusicSection();
	TSharedRef<SWidget> BuildDialogueSection();
	TSharedRef<SWidget> BuildMixLayersSection();
	TSharedRef<SWidget> BuildDuckingSection();
	TSharedRef<SWidget> BuildPoolSection();
	TSharedRef<SWidget> BuildMemorySection();
	TSharedRef<SWidget> BuildEventHistorySection();
	TSharedRef<SWidget> BuildProfileSection();
	TSharedRef<SWidget> BuildBackendSection();

	// ── Row Generators ───────────────────────────────────────────────────
	TSharedRef<ITableRow> OnGenerateChannelRow(TSharedPtr<FPGXAudioChannelSnapshot> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateActiveSoundRow(TSharedPtr<FPGXActiveSoundInfo> Item, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> OnGenerateEventRow(TSharedPtr<FPGXAudioEventRecord> Item, const TSharedRef<STableViewBase>& OwnerTable);

	// ── Actions ──────────────────────────────────────────────────────────
	FReply OnStopAllClicked();
	void OnMuteAllToggled(ECheckBoxState NewState);

	// ── Helpers ──────────────────────────────────────────────────────────
	UPGXAudioSubsystem* GetAudioSubsystem() const;
	UPGXAudioMixSubsystem* GetMixSubsystem() const;

	// ── Refresh ──────────────────────────────────────────────────────────
	float RefreshAccumulator = 0.0f;
	static constexpr float RefreshInterval = 0.5f;

	// ── SListView Data ───────────────────────────────────────────────────
	TArray<TSharedPtr<FPGXAudioChannelSnapshot>> ChannelData;
	TSharedPtr<SListView<TSharedPtr<FPGXAudioChannelSnapshot>>> ChannelListView;

	TArray<TSharedPtr<FPGXActiveSoundInfo>> SoundData;
	TSharedPtr<SListView<TSharedPtr<FPGXActiveSoundInfo>>> SoundListView;

	TArray<TSharedPtr<FPGXAudioEventRecord>> EventData;
	TSharedPtr<SListView<TSharedPtr<FPGXAudioEventRecord>>> EventListView;

	// ── Cached Text Blocks (sections still using STextBlock) ─────────────
	TSharedPtr<STextBlock> SystemStatusText;
	TSharedPtr<STextBlock> BackendText;
	TSharedPtr<STextBlock> MusicText;
	TSharedPtr<STextBlock> DialogueText;
	TSharedPtr<STextBlock> MixLayersText;
	TSharedPtr<STextBlock> DuckingText;
	TSharedPtr<STextBlock> PoolText;
	TSharedPtr<STextBlock> MemoryText;
	TSharedPtr<STextBlock> ProfileText;

	// ── KPI + Status ─────────────────────────────────────────────────────
	TSharedPtr<STextBlock> ActiveSoundsKPI;
	TSharedPtr<STextBlock> PoolUsageKPI;
	TSharedPtr<STextBlock> MemoryKPI;
	TSharedPtr<STextBlock> StatusFooterText;
};
