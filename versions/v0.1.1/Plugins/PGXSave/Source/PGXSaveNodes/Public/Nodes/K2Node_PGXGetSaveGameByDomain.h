// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "GameplayTagContainer.h"

#include "K2Node_PGXGetSaveGameByDomain.generated.h"

class FBlueprintActionDatabaseRegistrar;

/**
 * EN: K2Node that auto-resolves the SaveGame class from a DomainTag.
 *     When the user sets a DomainTag literal, the node scans UPGXSaveConfig DAs
 *     via AssetRegistry to find the matching FPGXSaveDomainEntry and updates
 *     the output pin type to the configured SaveGameClass.
 *     Scales to N domains with zero manual configuration per node.
 *
 * ES: K2Node que auto-resuelve la clase SaveGame desde un DomainTag.
 *     Cuando el usuario pone un DomainTag literal, el nodo escanea UPGXSaveConfig DAs
 *     via AssetRegistry para encontrar el FPGXSaveDomainEntry correspondiente y actualiza
 *     el tipo del pin de salida a la SaveGameClass configurada.
 *     Escala a N dominios sin configuracion manual por nodo.
 */
UCLASS()
class PGXSAVENODES_API UK2Node_PGXGetSaveGameByDomain : public UK2Node
{
	GENERATED_BODY()

public:
	//~ UEdGraphNode Interface
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
	FLinearColor GetNodeTitleColor() const override;
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	void PostReconstructNode() override;
	void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	//~ End UEdGraphNode Interface

	//~ UK2Node Interface
	void AllocateDefaultPins() override;
	void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	FText GetMenuCategory() const override;
	bool IsNodePure() const override { return false; }
	//~ End UK2Node Interface

private:
	/**
	 * EN: Scan AssetRegistry for UPGXSaveConfig DAs, find FPGXSaveDomainEntry matching the tag.
	 * ES: Escanear AssetRegistry por UPGXSaveConfig DAs, encontrar FPGXSaveDomainEntry con el tag.
	 * @return The SaveGameClass for the domain, or nullptr if not found.
	 */
	UClass* ResolveSaveGameClassForTag(const FGameplayTag& DomainTag) const;

	/** EN: Update the output SaveGame pin type based on current DomainTag / ES: Actualizar tipo del pin SaveGame segun DomainTag actual */
	void RefreshOutputPinType();

	UEdGraphPin* GetDomainTagPin() const;
	UEdGraphPin* GetSaveGamePin() const;
	UEdGraphPin* GetSuccessPin() const;
};
