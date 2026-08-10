// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Nodes/K2Node_AsyncAction_ListenForPGXMessages.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "EdGraph/EdGraph.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Messages/PGXAsyncListenForMessage.h"

#define LOCTEXT_NAMESPACE "K2Node_AsyncAction_ListenForPGXMessages"

namespace K2Node_AsyncAction_ListenForPGXMessagesHelper
{
	static const FName ChannelPinName(TEXT("Channel"));
	static const FName SenderPinName(TEXT("Sender"));
	static const FName PayloadPinName(TEXT("Payload"));
	static const FName PayloadTypePinName(TEXT("PayloadType"));
	static const FName DelegateProxyPinName(TEXT("ProxyObject"));
}

void UK2Node_AsyncAction_ListenForPGXMessages::PostReconstructNode()
{
	Super::PostReconstructNode();
	RefreshOutputPayloadType();
}

void UK2Node_AsyncAction_ListenForPGXMessages::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin == GetPayloadTypePin() && ChangedPin->LinkedTo.Num() == 0)
	{
		RefreshOutputPayloadType();
	}
}

void UK2Node_AsyncAction_ListenForPGXMessages::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{
	Super::GetPinHoverText(Pin, HoverTextOut);
	if (Pin.PinName == K2Node_AsyncAction_ListenForPGXMessagesHelper::PayloadPinName)
	{
		HoverTextOut += LOCTEXT("PayloadOutTooltip", "\n\nThe message payload received on this channel.").ToString();
	}
}

void UK2Node_AsyncAction_ListenForPGXMessages::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct FGetMenuActionsUtils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_AsyncAction_ListenForPGXMessages* AsyncTaskNode = CastChecked<UK2Node_AsyncAction_ListenForPGXMessages>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());

				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UPGXAsyncListenForMessage>(
		FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda(
			[NodeClass](const UFunction* FactoryFunc) -> UBlueprintNodeSpawner*
			{
				UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
				check(NodeSpawner != nullptr);
				NodeSpawner->NodeClass = NodeClass;

				const TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
				NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(
					FGetMenuActionsUtils::SetNodeFunc, FunctionPtr);

				return NodeSpawner;
			}));
}

void UK2Node_AsyncAction_ListenForPGXMessages::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// EN: Internal-only proxy object; hide it in the editor node.
	// ES: Objeto proxy interno; ocultarlo en el nodo del editor.
	UEdGraphPin* DelegateProxyPin = FindPin(K2Node_AsyncAction_ListenForPGXMessagesHelper::DelegateProxyPinName);
	if (ensure(DelegateProxyPin))
	{
		DelegateProxyPin->bHidden = true;
	}

	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, K2Node_AsyncAction_ListenForPGXMessagesHelper::PayloadPinName);
}

bool UK2Node_AsyncAction_ListenForPGXMessages::HandleDelegates(
	const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
	UEdGraphPin* ProxyObjectPin,
	UEdGraphPin*& InOutLastThenPin,
	UEdGraph* SourceGraph,
	FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;

	// EN: Expected layout: 0=ProxyObject, 1=Channel, 2=Sender, 3=Payload (custom pin).
	// ES: Layout esperado: 0=ProxyObject, 1=Channel, 2=Sender, 3=Payload (pin custom).
	if (VariableOutputs.Num() < 4)
	{
		ensureMsgf(false,
			TEXT("UK2Node_AsyncAction_ListenForPGXMessages::HandleDelegates - Unexpected output layout. Need ProxyObject/Channel/Sender/Payload."));
		return false;
	}

	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		UEdGraphPin* LastActivatedThenPin = nullptr;
		bIsErrorFree &= FBaseAsyncTaskHelper::HandleDelegateImplementation(
			*PropertyIt,
			VariableOutputs,
			ProxyObjectPin,
			InOutLastThenPin,
			LastActivatedThenPin,
			this,
			SourceGraph,
			CompilerContext);

		bIsErrorFree &= HandlePayloadImplementation(
			*PropertyIt,
			VariableOutputs[0],
			VariableOutputs[3],
			VariableOutputs[1],
			&VariableOutputs[2],
			LastActivatedThenPin,
			SourceGraph,
			CompilerContext);
	}

	return bIsErrorFree;
}

bool UK2Node_AsyncAction_ListenForPGXMessages::HandlePayloadImplementation(
	FMulticastDelegateProperty* CurrentProperty,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ChannelVar,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable* SenderVar,
	UEdGraphPin*& InOutLastActivatedThenPin,
	UEdGraph* SourceGraph,
	FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;

	const UEdGraphPin* PayloadPin = GetPayloadPin();
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	check(CurrentProperty && SourceGraph && Schema);

	const FEdGraphPinType& PayloadPinType = PayloadPin->PinType;
	if (PayloadPinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		// EN: If payload is still wildcard and unused, ignore silently.
		// ES: Si el payload sigue siendo wildcard y no se usa, ignorar silenciosamente.
		return PayloadPin->LinkedTo.Num() == 0;
	}

	UK2Node_TemporaryVariable* TempPayloadVar = CompilerContext.SpawnInternalVariable(
		this,
		PayloadPinType.PinCategory,
		PayloadPinType.PinSubCategory,
		PayloadPinType.PinSubCategoryObject.Get(),
		PayloadPinType.ContainerType,
		PayloadPinType.PinValueType);

	UK2Node_CallFunction* CallGetPayloadNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallGetPayloadNode->FunctionReference.SetExternalMember(TEXT("GetPayload"), CurrentProperty->GetOwnerClass());
	CallGetPayloadNode->AllocateDefaultPins();

	UEdGraphPin* GetPayloadSelfPin = Schema->FindSelfPin(*CallGetPayloadNode, EGPD_Input);
	if (GetPayloadSelfPin)
	{
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadSelfPin, ProxyObjectVar.TempVar->GetVariablePin());

		UEdGraphPin* GetPayloadExecPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* GetPayloadThenPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* GetPayloadOutPin = CallGetPayloadNode->FindPinChecked(TEXT("OutPayload"));

		bIsErrorFree &= Schema->TryCreateConnection(TempPayloadVar->GetVariablePin(), GetPayloadOutPin);

		UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(this, SourceGraph);
		AssignNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadThenPin, AssignNode->GetExecPin());
		bIsErrorFree &= Schema->TryCreateConnection(PayloadVar.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());
		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), TempPayloadVar->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());

		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*InOutLastActivatedThenPin, *AssignNode->GetThenPin()).CanSafeConnect();
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastActivatedThenPin, GetPayloadExecPin);

		UEdGraphPin* OutChannelPin = GetOutputChannelPin();
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutChannelPin, *ChannelVar.TempVar->GetVariablePin()).CanSafeConnect();

		if (SenderVar != nullptr)
		{
			UEdGraphPin* OutSenderPin = GetOutputSenderPin();
			if (OutSenderPin != nullptr)
			{
				bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutSenderPin, *SenderVar->TempVar->GetVariablePin()).CanSafeConnect();
			}
		}
	}

	return bIsErrorFree;
}

void UK2Node_AsyncAction_ListenForPGXMessages::RefreshOutputPayloadType()
{
	UEdGraphPin* PayloadPin = GetPayloadPin();
	UEdGraphPin* PayloadTypePin = GetPayloadTypePin();

	if (PayloadTypePin->DefaultObject != PayloadPin->PinType.PinSubCategoryObject)
	{
		if (PayloadPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(PayloadPin);
		}

		PayloadPin->PinType.PinSubCategoryObject = PayloadTypePin->DefaultObject;
		PayloadPin->PinType.PinCategory =
			(PayloadTypePin->DefaultObject == nullptr) ? UEdGraphSchema_K2::PC_Wildcard : UEdGraphSchema_K2::PC_Struct;
	}
}

UEdGraphPin* UK2Node_AsyncAction_ListenForPGXMessages::GetPayloadPin() const
{
	UEdGraphPin* Pin = FindPinChecked(K2Node_AsyncAction_ListenForPGXMessagesHelper::PayloadPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForPGXMessages::GetPayloadTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(K2Node_AsyncAction_ListenForPGXMessagesHelper::PayloadTypePinName);
	check(Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForPGXMessages::GetOutputChannelPin() const
{
	UEdGraphPin* Pin = FindPinChecked(K2Node_AsyncAction_ListenForPGXMessagesHelper::ChannelPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForPGXMessages::GetOutputSenderPin() const
{
	UEdGraphPin* Pin = FindPin(K2Node_AsyncAction_ListenForPGXMessagesHelper::SenderPinName);
	if (Pin != nullptr)
	{
		check(Pin->Direction == EGPD_Output);
	}
	return Pin;
}

#undef LOCTEXT_NAMESPACE
