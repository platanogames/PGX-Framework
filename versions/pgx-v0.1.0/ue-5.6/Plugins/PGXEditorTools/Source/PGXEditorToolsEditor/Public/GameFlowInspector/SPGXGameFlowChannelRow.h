// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "PGXGameFlowTypes.h"

class UPGXGameFlowSubsystem;

/**
 * EN: Summary data for a single channel row in the GameFlow Inspector.
 *     Built/updated by SPGXGameFlowInspectorTab from subsystem state.
 * ES: Datos de resumen para una fila de canal en el GameFlow Inspector.
 *     Construido/actualizado por SPGXGameFlowInspectorTab desde el estado del subsistema.
 */
struct FPGXFlowChannelSummary
{
	EPGXFlowChannel Channel = EPGXFlowChannel::Global;
	FString ChannelName;
	FGameplayTag CurrentTag;
	FGameplayTag LastTag;
	int32 HistoryCount = 0;
	bool bCanRevert = false;
};

/**
 * EN: Expandable row widget for a single GameFlow channel.
 *     Collapsed: color indicator + channel name + current/last tag + history count + revert icon.
 *     Expanded: active rule details + allowed/disallowed lists + recent history entries.
 *     Follows the same NoBorder SButton + lazy-build pattern as SPGXLogViewerRow.
 * ES: Widget de fila expandible para un canal GameFlow individual.
 *     Colapsado: indicador de color + nombre de canal + tag actual/anterior + conteo historial + icono revert.
 *     Expandido: detalles de regla activa + listas permitidas/prohibidas + entradas recientes del historial.
 *     Sigue el mismo patron NoBorder SButton + lazy-build de SPGXLogViewerRow.
 */
class PGXEDITORTOOLSEDITOR_API SPGXGameFlowChannelRow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPGXGameFlowChannelRow)
		: _Summary(nullptr)
		, _ChannelColor(FLinearColor::White)
	{}
		SLATE_ARGUMENT(TSharedPtr<FPGXFlowChannelSummary>, Summary)
		SLATE_ARGUMENT(TWeakObjectPtr<UPGXGameFlowSubsystem>, Subsystem)
		SLATE_ARGUMENT(FLinearColor, ChannelColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** EN: Update displayed data without rebuilding / ES: Actualizar datos mostrados sin reconstruir */
	void UpdateSummary(const FPGXFlowChannelSummary& NewSummary);

private:
	TSharedPtr<FPGXFlowChannelSummary> Summary;
	TWeakObjectPtr<UPGXGameFlowSubsystem> Subsystem;
	FLinearColor ChannelColor;

	bool bExpanded = false;
	TSharedPtr<SWidget> ExpandedContent;
	TSharedPtr<SVerticalBox> RootBox;

	// EN: Displayed text blocks for live updates / ES: Bloques de texto mostrados para actualizaciones en vivo
	TSharedPtr<STextBlock> CurrentTagText;
	TSharedPtr<STextBlock> LastTagText;
	TSharedPtr<STextBlock> HistoryCountText;
	TSharedPtr<STextBlock> RevertIndicator;

	// EN: Build expanded detail panel (lazy, first click) / ES: Construir panel expandido de detalle (lazy, primer click)
	TSharedRef<SWidget> BuildExpandedPanel() const;

	// EN: Get channel color by channel enum / ES: Obtener color de canal por enum de canal
	static FLinearColor GetChannelColor(EPGXFlowChannel Channel);
};
