// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Nodes/K2Node_PGXGetSaveGameByDomain.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet/KismetSystemLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "PGXSaveConfig.h"
#include "PGXSaveGame.h"
#include "PGXSaveBlueprintLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node_PGXGetSaveGameByDomain"

namespace PGXGetSaveGamePins
{
	static const FName DomainTagPinName(TEXT("DomainTag"));
	static const FName SaveGamePinName(TEXT("SaveGame"));
	static const FName SuccessPinName(TEXT("bSuccess"));
}

// ============================================================================
// EN: Node visuals
// ============================================================================

FText UK2Node_PGXGetSaveGameByDomain::GetNodeTitle(ENodeTitleType::Type /*TitleType*/) const
{
	return LOCTEXT("NodeTitle", "Get Domain Save Game");
}

FText UK2Node_PGXGetSaveGameByDomain::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip",
		"Get the active SaveGame for a domain.\n"
		"The output type is auto-resolved from the DomainTag's SaveConfig DA.\n\n"
		"ES: Obtener el SaveGame activo para un dominio.\n"
		"El tipo de salida se auto-resuelve del DA SaveConfig del DomainTag.");
}

FLinearColor UK2Node_PGXGetSaveGameByDomain::GetNodeTitleColor() const
{
	// EN: Save system green / ES: Verde del sistema Save
	return FLinearColor(76.f / 255.f, 175.f / 255.f, 80.f / 255.f);
}

FSlateIcon UK2Node_PGXGetSaveGameByDomain::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = GetNodeTitleColor();
	return FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.SaveGame");
}

// ============================================================================
// EN: Pin allocation
// ============================================================================

void UK2Node_PGXGetSaveGameByDomain::AllocateDefaultPins()
{
	// EN: Exec pins / ES: Pines de ejecucion
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// EN: Input — DomainTag (FGameplayTag) / ES: Entrada — DomainTag (FGameplayTag)
	UEdGraphPin* DomainPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct,
		FGameplayTag::StaticStruct(), PGXGetSaveGamePins::DomainTagPinName);
	DomainPin->PinFriendlyName = LOCTEXT("DomainTagFriendly", "Domain Tag");

	// EN: Hidden self/world context pin / ES: Pin oculto de contexto
	UEdGraphPin* WorldPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object,
		UObject::StaticClass(), UEdGraphSchema_K2::PN_Self);
	WorldPin->bHidden = true;

	// EN: Output — SaveGame (dynamic type) / ES: Salida — SaveGame (tipo dinamico)
	UEdGraphPin* SaveGamePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object,
		UPGXSaveGame::StaticClass(), PGXGetSaveGamePins::SaveGamePinName);
	SaveGamePin->PinFriendlyName = LOCTEXT("SaveGameFriendly", "Save Game");

	// EN: Output — bSuccess / ES: Salida — bSuccess
	UEdGraphPin* SuccessPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean,
		PGXGetSaveGamePins::SuccessPinName);
	SuccessPin->PinFriendlyName = LOCTEXT("SuccessFriendly", "Success");

	Super::AllocateDefaultPins();
}

// ============================================================================
// EN: Dynamic type resolution
// ============================================================================

void UK2Node_PGXGetSaveGameByDomain::PostReconstructNode()
{
	Super::PostReconstructNode();
	RefreshOutputPinType();
}

void UK2Node_PGXGetSaveGameByDomain::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin && ChangedPin->PinName == PGXGetSaveGamePins::DomainTagPinName)
	{
		RefreshOutputPinType();
	}
}

UClass* UK2Node_PGXGetSaveGameByDomain::ResolveSaveGameClassForTag(const FGameplayTag& DomainTag) const
{
	if (!DomainTag.IsValid())
	{
		return nullptr;
	}

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> ConfigAssets;
	AssetRegistry.GetAssetsByClass(UPGXSaveConfig::StaticClass()->GetClassPathName(), ConfigAssets);

	for (const FAssetData& AssetData : ConfigAssets)
	{
		const UPGXSaveConfig* Config = Cast<UPGXSaveConfig>(AssetData.GetAsset());
		if (!Config)
		{
			continue;
		}

		for (const FPGXSaveDomainEntry& Entry : Config->SaveDomains)
		{
			if (Entry.DomainTag == DomainTag && Entry.SaveGameClass)
			{
				return Entry.SaveGameClass.Get();
			}
		}
	}

	return nullptr;
}

void UK2Node_PGXGetSaveGameByDomain::RefreshOutputPinType()
{
	UEdGraphPin* DomainPin = GetDomainTagPin();
	UEdGraphPin* SaveGamePin = GetSaveGamePin();
	if (!DomainPin || !SaveGamePin)
	{
		return;
	}

	// EN: Default to base class / ES: Valor por defecto la clase base
	UClass* ResolvedClass = UPGXSaveGame::StaticClass();

	// EN: Only resolve when the pin has a literal value (not connected by wire)
	// ES: Solo resolver cuando el pin tiene valor literal (no conectado por cable)
	if (DomainPin->LinkedTo.Num() == 0 && !DomainPin->DefaultValue.IsEmpty())
	{
		FGameplayTag ParsedTag;
		// EN: Parse the tag from the pin's default value string / ES: Parsear el tag del string del valor por defecto del pin
		const FString& TagStr = DomainPin->DefaultValue;

		// EN: FGameplayTag default value is serialized as the tag string directly
		// ES: El valor por defecto de FGameplayTag se serializa como el string del tag directamente
		FGameplayTag CandidateTag = FGameplayTag::RequestGameplayTag(FName(*TagStr), false);
		if (CandidateTag.IsValid())
		{
			UClass* FoundClass = ResolveSaveGameClassForTag(CandidateTag);
			if (FoundClass)
			{
				ResolvedClass = FoundClass;
			}
		}
	}

	// EN: Update pin type only if it actually changed / ES: Actualizar tipo de pin solo si cambio
	if (SaveGamePin->PinType.PinSubCategoryObject != ResolvedClass)
	{
		SaveGamePin->PinType.PinSubCategoryObject = ResolvedClass;

		// EN: Notify graph that pin type changed / ES: Notificar al grafo que el tipo de pin cambio
		GetGraph()->NotifyNodeChanged(this);
	}
}

// ============================================================================
// EN: Compilation
// ============================================================================

void UK2Node_PGXGetSaveGameByDomain::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
	Super::ValidateNodeDuringCompilation(MessageLog);

	const UEdGraphPin* DomainPin = GetDomainTagPin();
	if (!DomainPin)
	{
		return;
	}

	// EN: Skip validation if pin is connected by wire (runtime value, can't validate)
	// ES: Saltar validacion si el pin esta conectado por cable (valor runtime, no validable)
	if (DomainPin->LinkedTo.Num() > 0)
	{
		return;
	}

	if (DomainPin->DefaultValue.IsEmpty())
	{
		MessageLog.Error(*LOCTEXT("NoDomainTag", "@@: No DomainTag specified. Set a domain tag or connect one.").ToString(), this);
		return;
	}

	FGameplayTag CandidateTag = FGameplayTag::RequestGameplayTag(FName(*DomainPin->DefaultValue), false);
	if (!CandidateTag.IsValid())
	{
		MessageLog.Error(*FText::Format(
			LOCTEXT("InvalidTag", "@@: DomainTag '{0}' is not a valid GameplayTag."),
			FText::FromString(DomainPin->DefaultValue)).ToString(), this);
		return;
	}

	UClass* ResolvedClass = ResolveSaveGameClassForTag(CandidateTag);
	if (!ResolvedClass)
	{
		MessageLog.Error(*FText::Format(
			LOCTEXT("TagNotFound", "@@: DomainTag '{0}' not found in any UPGXSaveConfig DA. Create a SaveConfig with this domain."),
			FText::FromString(DomainPin->DefaultValue)).ToString(), this);
	}
}

void UK2Node_PGXGetSaveGameByDomain::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// EN: Get our pins / ES: Obtener nuestros pines
	UEdGraphPin* ExecPin = GetExecPin();
	UEdGraphPin* ThenPin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	UEdGraphPin* DomainPin = GetDomainTagPin();
	UEdGraphPin* SelfPin = FindPinChecked(UEdGraphSchema_K2::PN_Self);
	UEdGraphPin* SaveGamePin = GetSaveGamePin();
	UEdGraphPin* SuccessPin = GetSuccessPin();

	// EN: Determine the resolved class for the cast / ES: Determinar la clase resuelta para el cast
	UClass* ResolvedClass = Cast<UClass>(SaveGamePin->PinType.PinSubCategoryObject.Get());
	if (!ResolvedClass)
	{
		ResolvedClass = UPGXSaveGame::StaticClass();
	}

	// EN: Step 1: Create a CallFunction node for UPGXSaveBlueprintLibrary::GetSaveGame
	// ES: Paso 1: Crear nodo CallFunction para UPGXSaveBlueprintLibrary::GetSaveGame
	UK2Node_CallFunction* GetSaveGameNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetSaveGameNode->FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UPGXSaveBlueprintLibrary, GetSaveGame),
		UPGXSaveBlueprintLibrary::StaticClass());
	GetSaveGameNode->AllocateDefaultPins();

	// EN: Wire exec / ES: Cablear ejecucion
	UEdGraphPin* CallExecPin = GetSaveGameNode->GetExecPin();
	UEdGraphPin* CallThenPin = GetSaveGameNode->GetThenPin();

	// EN: Wire inputs / ES: Cablear entradas
	UEdGraphPin* CallWorldContextPin = GetSaveGameNode->FindPinChecked(TEXT("WorldContextObject"));
	UEdGraphPin* CallDomainTagPin = GetSaveGameNode->FindPinChecked(TEXT("DomainTag"));
	UEdGraphPin* CallReturnPin = GetSaveGameNode->GetReturnValuePin();

	CompilerContext.MovePinLinksToIntermediate(*ExecPin, *CallExecPin);
	CompilerContext.MovePinLinksToIntermediate(*SelfPin, *CallWorldContextPin);
	CompilerContext.MovePinLinksToIntermediate(*DomainPin, *CallDomainTagPin);

	// EN: If the resolved class is the base class, skip the cast node
	// ES: Si la clase resuelta es la clase base, saltar el nodo de cast
	if (ResolvedClass == UPGXSaveGame::StaticClass())
	{
		// EN: Direct wiring — no cast needed / ES: Cableado directo — no necesita cast
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *CallThenPin);
		CompilerContext.MovePinLinksToIntermediate(*SaveGamePin, *CallReturnPin);

		// EN: Wire bSuccess — true if return is valid / ES: Cablear bSuccess — true si el retorno es valido
		if (SuccessPin->LinkedTo.Num() > 0)
		{
			UK2Node_CallFunction* IsValidNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
			IsValidNode->FunctionReference.SetExternalMember(
				GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid),
				UKismetSystemLibrary::StaticClass());
			IsValidNode->AllocateDefaultPins();

			UEdGraphPin* IsValidInputPin = IsValidNode->FindPinChecked(TEXT("Object"));
			UEdGraphPin* IsValidReturnPin = IsValidNode->GetReturnValuePin();

			CallReturnPin->MakeLinkTo(IsValidInputPin);
			CompilerContext.MovePinLinksToIntermediate(*SuccessPin, *IsValidReturnPin);
		}
	}
	else
	{
		// EN: Step 2: Create a DynamicCast node for the resolved type
		// ES: Paso 2: Crear nodo DynamicCast para el tipo resuelto
		UK2Node_DynamicCast* CastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, SourceGraph);
		CastNode->TargetType = ResolvedClass;
		CastNode->SetPurity(false);
		CastNode->AllocateDefaultPins();

		// EN: Wire from GetSaveGame return to cast input / ES: Cablear desde retorno de GetSaveGame a entrada del cast
		UEdGraphPin* CastInputPin = CastNode->GetCastSourcePin();
		UEdGraphPin* CastExecPin = CastNode->GetExecPin();
		UEdGraphPin* CastValidPin = CastNode->GetValidCastPin();
		UEdGraphPin* CastResultPin = CastNode->GetCastResultPin();
		UEdGraphPin* CastInvalidPin = CastNode->GetInvalidCastPin();

		CallThenPin->MakeLinkTo(CastExecPin);
		CallReturnPin->MakeLinkTo(CastInputPin);

		// EN: Wire outputs / ES: Cablear salidas
		CompilerContext.MovePinLinksToIntermediate(*ExecPin, *CallExecPin);
		CompilerContext.MovePinLinksToIntermediate(*ThenPin, *CastValidPin);
		CompilerContext.MovePinLinksToIntermediate(*SaveGamePin, *CastResultPin);

		// EN: bSuccess = cast succeeded / ES: bSuccess = cast exitoso
		// EN: Wire the invalid cast to Then as well (execution continues either way)
		// ES: Cablear el cast invalido a Then tambien (la ejecucion continua de todos modos)
		if (SuccessPin->LinkedTo.Num() > 0)
		{
			// EN: Create an IsValid check on the cast result
			// ES: Crear una verificacion IsValid en el resultado del cast
			UK2Node_CallFunction* IsValidNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
			IsValidNode->FunctionReference.SetExternalMember(
				GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid),
				UKismetSystemLibrary::StaticClass());
			IsValidNode->AllocateDefaultPins();

			UEdGraphPin* IsValidInputPin = IsValidNode->FindPinChecked(TEXT("Object"));
			UEdGraphPin* IsValidReturnPin = IsValidNode->GetReturnValuePin();

			CastResultPin->MakeLinkTo(IsValidInputPin);
			CompilerContext.MovePinLinksToIntermediate(*SuccessPin, *IsValidReturnPin);
		}
	}

	BreakAllNodeLinks();
}

// ============================================================================
// EN: Menu registration
// ============================================================================

void UK2Node_PGXGetSaveGameByDomain::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_PGXGetSaveGameByDomain::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "PGX|Save");
}

// ============================================================================
// EN: Pin accessors
// ============================================================================

UEdGraphPin* UK2Node_PGXGetSaveGameByDomain::GetDomainTagPin() const
{
	return FindPin(PGXGetSaveGamePins::DomainTagPinName);
}

UEdGraphPin* UK2Node_PGXGetSaveGameByDomain::GetSaveGamePin() const
{
	return FindPin(PGXGetSaveGamePins::SaveGamePinName);
}

UEdGraphPin* UK2Node_PGXGetSaveGameByDomain::GetSuccessPin() const
{
	return FindPin(PGXGetSaveGamePins::SuccessPinName);
}

#undef LOCTEXT_NAMESPACE
