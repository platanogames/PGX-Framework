// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SPGXPass1InspectorBase.h"

class SDockTab;
class UClass;

/**
 * EN: Read-only Interaction Panel using the shared SPGXPanelHeader layout.
 *     Inherits the shared layout from SPGXPass1InspectorBase.
 * ES: Panel Interaction de solo lectura con el layout compartido SPGXPanelHeader.
 */
class SPGXInteractionPanel : public SPGXPass1InspectorBase
{
public:
	SLATE_BEGIN_ARGS(SPGXInteractionPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static void RegisterTabSpawner();
	static void UnregisterTabSpawner();
	static const FName TabId;

protected:
	virtual TArray<TPair<FText, UClass*>> GetObservableClasses() const override;
	virtual TArray<TPair<FText, FText>> GetDeferredCards() const override;
	virtual TSharedRef<SWidget> BuildHeader() const override;

private:
	static TSharedRef<SDockTab> SpawnTab(const class FSpawnTabArgs& Args);
};
