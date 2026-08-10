// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SPGXPass1InspectorBase.h"

class SDockTab;
class UClass;

/** Read-only Development Preview inspector for Trade configuration and schema status. Live offers, records and reputation views are not included. */
class SPGXTradeInspector : public SPGXPass1InspectorBase
{
public:
	SLATE_BEGIN_ARGS(SPGXTradeInspector) {}
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
