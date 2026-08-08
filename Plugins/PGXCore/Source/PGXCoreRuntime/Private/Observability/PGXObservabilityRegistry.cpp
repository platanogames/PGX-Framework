// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Observability/PGXObservabilityRegistry.h"
#include "Observability/PGXObservable.h"
#include "Observability/PGXObservableBase.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Class.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXObservabilityRegistry, Log, All);

// ============================================================================
// EN: FPGXObservabilityRegistry — static registry of PGX observable UClass entries.
//     Auto-discovery via TObjectIterator at module startup + manual fallback.
//     The registry remains valid when no observable subclasses are present.
// ES: Registro estatico de UClass observables PGX. Auto-discovery por
//     TObjectIterator + fallback manual.
// ============================================================================

namespace
{
	/** EN: Internal storage for registered observable UClass entries. */
	TArray<TWeakObjectPtr<UClass>>& GetMutableRegisteredClasses()
	{
		static TArray<TWeakObjectPtr<UClass>> RegisteredClasses;
		return RegisteredClasses;
	}

	/** EN: True when `Class` is a candidate observable (UPGXObservableBase subclass OR implements IPGXObservable). */
	bool IsObservableClass(const UClass* Class)
	{
		if (!Class)
		{
			return false;
		}

		// EN: Skip the abstract base itself + the UInterface itself.
		if (Class == UPGXObservableBase::StaticClass() || Class == UPGXObservable::StaticClass())
		{
			return false;
		}

		// EN: Skip abstract classes (cannot instantiate observables from abstract roots).
		if (Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
		{
			return false;
		}

		// EN: Either inherits UPGXObservableBase OR implements IPGXObservable directly.
		const bool bDerivesFromBase = Class->IsChildOf(UPGXObservableBase::StaticClass());
		const bool bImplementsInterface = Class->ImplementsInterface(UPGXObservable::StaticClass());

		return bDerivesFromBase || bImplementsInterface;
	}
}

bool FPGXObservabilityRegistry::Register(UClass* ObservableClass)
{
	if (!ObservableClass)
	{
		UE_LOG(LogPGXObservabilityRegistry, Warning, TEXT("Register called with null UClass — ignored"));
		return false;
	}

	if (!IsObservableClass(ObservableClass))
	{
		UE_LOG(LogPGXObservabilityRegistry, Warning,
			TEXT("Register called with non-observable UClass %s — ignored (must inherit UPGXObservableBase OR implement IPGXObservable)"),
			*ObservableClass->GetName());
		return false;
	}

	TArray<TWeakObjectPtr<UClass>>& Registered = GetMutableRegisteredClasses();
	const TWeakObjectPtr<UClass> WeakClass(ObservableClass);

	// EN: Idempotent — skip if already registered.
	for (const TWeakObjectPtr<UClass>& Existing : Registered)
	{
		if (Existing == WeakClass)
		{
			return false;
		}
	}

	Registered.Add(WeakClass);
	UE_LOG(LogPGXObservabilityRegistry, Verbose,
		TEXT("Registered observable class: %s (count: %d)"),
		*ObservableClass->GetName(), Registered.Num());
	return true;
}

void FPGXObservabilityRegistry::BootstrapDiscovery()
{
	int32 RegisteredThisPass = 0;

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (IsObservableClass(Class))
		{
			if (Register(Class))
			{
				++RegisteredThisPass;
			}
		}
	}

	UE_LOG(LogPGXObservabilityRegistry, Log,
		TEXT("BootstrapDiscovery completed — %d observable classes registered this pass (total: %d)"),
		RegisteredThisPass, GetMutableRegisteredClasses().Num());
}

TArray<TWeakObjectPtr<UClass>> FPGXObservabilityRegistry::GetRegisteredClasses()
{
	return GetMutableRegisteredClasses();
}

int32 FPGXObservabilityRegistry::GetRegisteredClassCount()
{
	return GetMutableRegisteredClasses().Num();
}

UClass* FPGXObservabilityRegistry::FindClassByTypeName(FName TypeName)
{
	if (TypeName.IsNone())
	{
		return nullptr;
	}

	const TArray<TWeakObjectPtr<UClass>>& Registered = GetMutableRegisteredClasses();
	for (const TWeakObjectPtr<UClass>& Entry : Registered)
	{
		if (UClass* Class = Entry.Get())
		{
			if (Class->GetFName() == TypeName)
			{
				return Class;
			}
		}
	}

	return nullptr;
}

void FPGXObservabilityRegistry::Reset()
{
	const int32 ClearedCount = GetMutableRegisteredClasses().Num();
	GetMutableRegisteredClasses().Reset();
	UE_LOG(LogPGXObservabilityRegistry, Log,
		TEXT("Reset — cleared %d observable class entries"), ClearedCount);
}
