// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
// EN: SPGXLiveInspectorBase — canonical live-PIE inspector base class (G7).
//     Extracted from SPGXLoadingInspectorTab (the most complete live inspector)
//     per the inspector_panel doctrine. Owns the shared machinery: PIE lifecycle
//     (PostPIEStarted/EndPIE), the standard toolbar (Refresh + KPI slot), the
//     status bar with PIE state + POST-PIE DATA RETENTION ("PIE Ended (data
//     retained)"), and the RefreshAll orchestration. Subclasses supply their own
//     data structs, row generators, RebuildPanels() queries and BuildBody() layout
//     via virtual hooks.
//
//     Bind modes (EPGXLiveBindMode): Delegate (subscribe to subsystem multicast
//     delegates, refresh reactively — Loading/Save) or Poll (no delegates, refresh
//     on Refresh-click + optional Tick @ PollIntervalSeconds while PIE — Trade).
//     Default Poll (works for query-only subsystems).
//
//     Slate inheritance pattern (same as SPGXPass1InspectorBase): the base does NOT
//     declare its own SLATE_BEGIN_ARGS / Construct. Each leaf subclass declares its
//     own SLATE_BEGIN_ARGS(SPGX<X>InspectorTab) + Construct(FArguments) and calls
//     SPGXLiveInspectorBase::BuildLiveLayout() once from inside it. This avoids the
//     FArguments type-mismatch when SLATE_BEGIN_ARGS is on both base + leaf.
//
// ES: SPGXLiveInspectorBase — base canonical para inspectores live-PIE (G7).
//     Extraida de SPGXLoadingInspectorTab. Posee: ciclo PIE, toolbar (Refresh+KPI),
//     status bar con estado PIE + retencion post-PIE, y la orquestacion RefreshAll.
//     Las subclases aportan structs, row generators, RebuildPanels() y BuildBody().
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SNullWidget.h"
#include "Input/Reply.h"

class UWorld;
class UGameInstance;

/**
 * EN: How the live inspector stays in sync with its subsystem.
 * ES: Como el inspector live se mantiene sincronizado con su subsystem.
 */
enum class EPGXLiveBindMode : uint8
{
	/** EN: Subscribe to subsystem multicast delegates; refresh reactively. */
	Delegate,
	/** EN: No delegates; refresh on Refresh-click + optional Tick poll while PIE. */
	Poll
};

class PGXCOREEDITOR_API SPGXLiveInspectorBase : public SCompoundWidget
{
public:
	~SPGXLiveInspectorBase() override;

	/**
	 * EN: Tick drives poll-mode refresh while PIE is active (no-op for Delegate mode
	 *     and outside PIE). Cadence = PollIntervalSeconds.
	 * ES: Tick mueve el refresh en poll-mode mientras PIE esta activo.
	 */
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	/**
	 * EN: Builds the standard live layout into ChildSlot (system-color shell + toolbar
	 *     [Refresh + KPI slot] + body via BuildBody() + status bar). Subclasses MUST call
	 *     this exactly once from their own Construct(), then BindPIEDelegates().
	 * ES: Construye el layout live estandar en ChildSlot. Llamar una sola vez desde el
	 *     Construct de la subclass, luego BindPIEDelegates().
	 */
	void BuildLiveLayout();

	/** EN: Binds editor PIE delegates (PostPIEStarted/EndPIE). Call after BuildLiveLayout(). */
	void BindPIEDelegates();

	/** EN: Re-pulls all data and refreshes the UI + KPI + status bar. Safe to call anytime. */
	void RefreshAll();

	// ── Virtual hooks (subclass supplies plugin-specific behavior) ───────────

	/** EN: System token color for the shell + accents (e.g. PGX::System::Trade). */
	virtual FLinearColor GetSystemColor() const = 0;

	/** EN: Inspector display title (e.g. "PGXTrade Live Inspector"). */
	virtual FText GetInspectorTitle() const = 0;

	/** EN: Bind mode. Default Poll. Override to Delegate for subsystems with delegates. */
	virtual EPGXLiveBindMode GetBindMode() const { return EPGXLiveBindMode::Poll; }

	/** EN: Poll cadence in seconds (poll-mode only). Default 1.0s (SystemObserver precedent). */
	virtual float GetPollIntervalSeconds() const { return 1.0f; }

	/**
	 * EN: Builds the body widget (the plugin's STATUS / CATALOG / HISTORY / DEBUG panels)
	 *     placed between toolbar and status bar. Must follow the standard layout order.
	 * ES: Construye el cuerpo (paneles STATUS/CATALOG/HISTORY/DEBUG del plugin).
	 */
	virtual TSharedRef<SWidget> BuildBody() = 0;

	/**
	 * EN: Re-pulls the plugin's data from the live subsystem into its lists/widgets and
	 *     requests list refresh. Called by RefreshAll() (and the Tick poll). The base
	 *     guarantees this runs only when a live subsystem is resolvable; post-PIE the
	 *     last data is retained (RefreshAll is not called on PIE-end).
	 * ES: Re-pulla los datos del plugin desde el subsystem live. Llamado por RefreshAll().
	 */
	virtual void RebuildPanels() = 0;

	/**
	 * EN: Optional: build the KPI chips shown in the toolbar (counts/state). Default none.
	 * ES: Opcional: chips KPI del toolbar. Default ninguno.
	 */
	virtual TSharedRef<SWidget> BuildKPIChips() { return SNullWidget::NullWidget; }

	/**
	 * EN: Delegate-mode only: subscribe to the subsystem's multicast delegates so refresh
	 *     is reactive. Called on PIE start when GetBindMode()==Delegate. Default no-op.
	 * ES: Solo Delegate-mode: suscribe a los delegates del subsystem. Default no-op.
	 */
	virtual void BindSubsystemDelegates() {}

	/** EN: Delegate-mode only: unsubscribe. Called on PIE end / destruction. Default no-op. */
	virtual void UnbindSubsystemDelegates() {}

	// ── Helpers for subclasses ───────────────────────────────────────────────

	/** EN: Returns the active PIE GameInstance, or nullptr outside PIE. */
	UGameInstance* GetPIEGameInstance() const;

	/** EN: True while a PIE session is active. */
	bool IsPIEActive() const { return bIsPIEActive; }

private:
	void OnPIEStarted(bool bIsSimulating);
	void OnPIEEnded(bool bIsSimulating);
	FReply OnRefreshClicked();
	void SetStatusBarLive();
	void SetStatusBarIdle();
	void SetStatusBarRetained();

	// ── State ────────────────────────────────────────────────────────────────

	bool bIsPIEActive = false;
	// EN: Tracks whether subsystem delegates are currently bound (Delegate mode). Read in
	//     the destructor instead of the virtual GetBindMode() — the vtable is no longer the
	//     derived class's during destruction.
	// ES: Rastrea si los delegates estan bound. Leido en el destructor en vez del virtual
	//     GetBindMode() (el vtable ya no es del derivado en destruccion).
	bool bDelegatesBound = false;
	double LastPollTime = 0.0;

	FDelegateHandle PIEStartedHandle;
	FDelegateHandle PIEEndedHandle;

	TSharedPtr<STextBlock> StatusBarWidget;
};
