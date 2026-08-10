// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SPGXPass1InspectorBase.h"

class SDockTab;
class UClass;

/**
 * EN: read-only MGOS Inspector. Inherits canonical layout from
 *     the shared editor-inspector base.
 * ES: Inspector MGOS read-only — hereda layout canonical.
 */
class SPGXMGOSInspector : public SPGXPass1InspectorBase
{
public:
	SLATE_BEGIN_ARGS(SPGXMGOSInspector) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static void RegisterTabSpawner();
	static void UnregisterTabSpawner();
	static const FName TabId;

protected:
	virtual FText GetInspectorTitle() const override;
	virtual TArray<TPair<FText, UClass*>> GetObservableClasses() const override;
	virtual TArray<TPair<FText, FText>> GetDeferredCards() const override;

private:
	static TSharedRef<SDockTab> SpawnTab(const class FSpawnTabArgs& Args);
};
