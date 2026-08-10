// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "K2Node_AsyncAction.h"

#include "K2Node_AsyncAction_ListenForPGXMessages.generated.h"

class FBlueprintActionDatabaseRegistrar;
class FKismetCompilerContext;
class FMulticastDelegateProperty;
class FString;
class UEdGraph;
class UEdGraphPin;

/**
 * EN: Dedicated K2 node for UPGXAsyncListenForMessage.
 *     Keeps Payload output as wildcard/typed struct based on PayloadType input (Lyra pattern).
 *     Lives in UncookedOnly module so it can be used in runtime Blueprints.
 * ES: Nodo K2 dedicado para UPGXAsyncListenForMessage.
 *     Mantiene Payload como wildcard/struct tipado segun el input PayloadType (patron Lyra).
 *     Vive en modulo UncookedOnly para poder usarse en Blueprints runtime.
 */
UCLASS()
class UK2Node_AsyncAction_ListenForPGXMessages : public UK2Node_AsyncAction
{
	GENERATED_BODY()

public:
	//~ UEdGraphNode Interface
	void PostReconstructNode() override;
	void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;
	void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	//~ End UEdGraphNode Interface

	//~ UK2Node Interface
	void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	void AllocateDefaultPins() override;
	//~ End UK2Node Interface

protected:
	bool HandleDelegates(
		const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
		UEdGraphPin* ProxyObjectPin,
		UEdGraphPin*& InOutLastThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext) override;

private:

	// EN: Handles the delegate binding and execution logic for the ListenForPGXMessages node.
	// ES: Maneja la logica de enlace y ejecucion del delegado para el nodo ListenForPGXMessages.
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ChannelVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable* SenderVar,
		UEdGraphPin*& InOutLastActivatedThenPin,
		UEdGraph* SourceGraph,
		FKismetCompilerContext& CompilerContext);

	// EN: Updates the output pin type based on the current PayloadType input pin value.
	// ES: Actualiza el tipo del pin de salida basado en el valor actual del pin de entrada PayloadType.
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
	UEdGraphPin* GetOutputChannelPin() const;
	UEdGraphPin* GetOutputSenderPin() const;
};
