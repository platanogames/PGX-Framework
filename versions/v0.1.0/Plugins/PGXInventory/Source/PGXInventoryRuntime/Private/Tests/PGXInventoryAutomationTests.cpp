// Copyright PGX Framework. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXInventoryComponent.h"
#include "PGXInventoryTypes.h"
#include "PGXItemDefinition.h"
#include "Misc/AutomationTest.h"

namespace PGXInventoryAutomation
{
#define PGX_INVENTORY_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	UPGXInventoryComponent* MakeInventory(const TCHAR* Name, int32 MaxSlots = 4, float MaxWeight = 100.0f)
	{
		UPGXInventoryComponent* Inventory = NewObject<UPGXInventoryComponent>(GetTransientPackage(), UPGXInventoryComponent::StaticClass(), FName(Name), RF_Transient);
		Inventory->MaxSlots = MaxSlots;
		Inventory->MaxWeight = MaxWeight;
		return Inventory;
	}

	UPGXItemDefinition* MakeDefinition(const TCHAR* Name, int32 MaxStackSize = 8, float UnitWeight = 1.0f)
	{
		UPGXItemDefinition* Definition = NewObject<UPGXItemDefinition>(GetTransientPackage(), UPGXItemDefinition::StaticClass(), FName(Name), RF_Transient);
		Definition->ItemName = FName(Name);
		Definition->MaxStackSize = MaxStackSize;
		Definition->Weight = UnitWeight;
		return Definition;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_AddSuccessAutomationTest,
	"PGX.Inventory.preview.AddSuccess", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_AddSuccessAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_AddSuccess"));
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_AddSuccess"), 8, 1.0f);

	const FPGXInventoryResult Result = Inventory->AddItem(Definition, 3);
	TestTrue(TEXT("AddSuccess result"), Result.bSuccess && Result.Code == EPGXInventoryResultCode::Success);
	TestEqual(TEXT("AddSuccess quantity"), Inventory->GetItemQuantity(Definition), 3);
	TestEqual(TEXT("AddSuccess slots"), Inventory->GetUsedSlotCount(), 1);
	TestTrue(TEXT("AddSuccess weight"), FMath::IsNearlyEqual(Inventory->GetCurrentWeight(), 3.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_StackMergeAutomationTest,
	"PGX.Inventory.preview.StackMerge", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_StackMergeAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_StackMerge"));
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_StackMerge"), 5, 0.5f);

	Inventory->AddItem(Definition, 3);
	const FPGXInventoryResult Result = Inventory->AddItem(Definition, 2);

	TestTrue(TEXT("StackMerge result"), Result.bSuccess);
	TestEqual(TEXT("StackMerge quantity"), Inventory->GetItemQuantity(Definition), 5);
	TestEqual(TEXT("StackMerge stays in one slot"), Inventory->GetUsedSlotCount(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_SlotRejectionAutomationTest,
	"PGX.Inventory.preview.SlotRejection", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_SlotRejectionAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_SlotRejection"), 1, 100.0f);
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_SlotRejection"), 2, 0.0f);

	Inventory->AddItem(Definition, 2);
	const FPGXInventoryResult Result = Inventory->AddItem(Definition, 1);

	TestFalse(TEXT("SlotRejection result"), Result.bSuccess);
	TestTrue(TEXT("SlotRejection code"), Result.Code == EPGXInventoryResultCode::SlotCapacityExceeded);
	TestEqual(TEXT("SlotRejection no mutation"), Inventory->GetItemQuantity(Definition), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_WeightRejectionAutomationTest,
	"PGX.Inventory.preview.WeightRejection", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_WeightRejectionAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_WeightRejection"), 4, 2.0f);
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_WeightRejection"), 8, 1.5f);

	const FPGXInventoryResult Rejected = Inventory->AddItem(Definition, 2);
	const FPGXInventoryResult Accepted = Inventory->AddItem(Definition, 1);

	TestFalse(TEXT("WeightRejection result"), Rejected.bSuccess);
	TestTrue(TEXT("WeightRejection code"), Rejected.Code == EPGXInventoryResultCode::WeightCapacityExceeded);
	TestTrue(TEXT("WeightRejection accepts fitting quantity"), Accepted.bSuccess);
	TestEqual(TEXT("WeightRejection final quantity"), Inventory->GetItemQuantity(Definition), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_RemoveAutomationTest,
	"PGX.Inventory.preview.Remove", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_RemoveAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_Remove"));
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_Remove"), 8, 1.0f);
	Inventory->AddItem(Definition, 5);

	const FPGXInventoryResult Removed = Inventory->RemoveItem(Definition, 2);
	const FPGXInventoryResult Rejected = Inventory->RemoveItem(Definition, 4);

	TestTrue(TEXT("Remove result"), Removed.bSuccess);
	TestEqual(TEXT("Remove remaining quantity"), Inventory->GetItemQuantity(Definition), 3);
	TestFalse(TEXT("Remove rejects insufficient quantity"), Rejected.bSuccess);
	TestTrue(TEXT("Remove insufficient code"), Rejected.Code == EPGXInventoryResultCode::InsufficientQuantity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_TransferAutomationTest,
	"PGX.Inventory.preview.Transfer", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_TransferAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Source = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_Transfer_Source"));
	UPGXInventoryComponent* Destination = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_Transfer_Destination"));
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_Transfer"), 8, 1.0f);
	Source->AddItem(Definition, 4);

	const FPGXInventoryResult Result = Source->TransferItemTo(Destination, Definition, 3);

	TestTrue(TEXT("Transfer result"), Result.bSuccess);
	TestEqual(TEXT("Transfer source quantity"), Source->GetItemQuantity(Definition), 1);
	TestEqual(TEXT("Transfer destination quantity"), Destination->GetItemQuantity(Definition), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_InvalidTypedFailuresAutomationTest,
	"PGX.Inventory.preview.InvalidTypedFailures", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_InvalidTypedFailuresAutomationTest::RunTest(const FString& Parameters)
{
	UPGXInventoryComponent* Inventory = PGXInventoryAutomation::MakeInventory(TEXT("PGXInventory_InvalidTypedFailures"));
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_Item_InvalidTypedFailures"), 8, 1.0f);

	const FPGXInventoryResult InvalidDefinition = Inventory->AddItem(nullptr, 1);
	const FPGXInventoryResult InvalidQuantity = Inventory->AddItem(Definition, 0);
	const FPGXInventoryResult MissingDestination = Inventory->TransferItemTo(nullptr, Definition, 1);

	TestFalse(TEXT("Invalid definition failure"), InvalidDefinition.bSuccess);
	TestTrue(TEXT("Invalid definition code"), InvalidDefinition.Code == EPGXInventoryResultCode::InvalidDefinition);
	TestFalse(TEXT("Invalid quantity failure"), InvalidQuantity.bSuccess);
	TestTrue(TEXT("Invalid quantity code"), InvalidQuantity.Code == EPGXInventoryResultCode::InvalidQuantity);
	TestFalse(TEXT("Missing destination failure"), MissingDestination.bSuccess);
	TestTrue(TEXT("Missing destination code"), MissingDestination.Code == EPGXInventoryResultCode::DestinationMissing);
	TestFalse(TEXT("Failure messages visible"), InvalidDefinition.Message.IsEmpty() || InvalidQuantity.Message.IsEmpty() || MissingDestination.Message.IsEmpty());
	return true;
}


// ============================================================================
// EN: Observability 8.3.C economy cluster
// ES: Observability 8.3.C cluster economia
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXInventory_ObservableItemDefinitionAutomationTest,
	"PGX.Inventory.preview.ObservableItemDefinition", PGX_INVENTORY_AUTOMATION_FLAGS)
bool FPGXInventory_ObservableItemDefinitionAutomationTest::RunTest(const FString& Parameters)
{
	UPGXItemDefinition* Definition = PGXInventoryAutomation::MakeDefinition(TEXT("PGXInventory_ObservableItem"), 16, 2.5f);
	if (!TestNotNull(TEXT("Item definition asset"), Definition))
	{
		return false;
	}
	Definition->DisplayName = FText::FromString(TEXT("Observable Item"));
	Definition->Description = FText::FromString(TEXT("Observable item description"));

	TestTrue(TEXT("Item definition implements IPGXObservable"), UPGXItemDefinition::StaticClass()->ImplementsInterface(UPGXObservable::StaticClass()));
	const FPGXJsonValue Json = Definition->ToJson();
	TestFalse(TEXT("Item observable ToJson non-empty"), Json.IsEmpty());
	TestTrue(TEXT("Item JSON contains type"), Json.JsonString.Contains(UPGXItemDefinition::StaticClass()->GetName()));
	TestTrue(TEXT("Item JSON contains concrete data"), Json.JsonString.Contains(TEXT("MaxStackSize")));
	TestEqual(TEXT("Item schema version"), Definition->GetSchemaVersion(), UPGXItemDefinition::SchemaVersion);

	const FPGXSchemaDescriptor Descriptor = Definition->GetSchemaDescriptor();
	TestEqual(TEXT("Item descriptor type"), Descriptor.TypeName, UPGXItemDefinition::StaticClass()->GetFName());
	TestTrue(TEXT("Item descriptor fields"), Descriptor.Fields.Num() > 0);

	const FPGXValidationResult EmptyValidation = Definition->FromJson(FPGXJsonValue());
	TestFalse(TEXT("Item FromJson empty payload fails"), EmptyValidation.bValid);
	TestTrue(TEXT("Item FromJson reports errors"), EmptyValidation.Errors.Num() > 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
