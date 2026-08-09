// Copyright PGX Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SPGXPass1InspectorBase.h"

class SDockTab;
class UClass;

/**
 * EN: read-only Vehicles Inspector. Inherits canonical layout from
 *     SPGXPass1InspectorBase (shared inspector layout).
 * ES: Inspector Vehicles de solo lectura; hereda el layout compartido.
 */
class SPGXVehiclesInspector : public SPGXPass1InspectorBase
{
public:
	SLATE_BEGIN_ARGS(SPGXVehiclesInspector) {}
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
