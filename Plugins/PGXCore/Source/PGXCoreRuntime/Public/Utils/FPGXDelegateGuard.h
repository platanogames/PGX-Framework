// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

/**
 * EN: RAII helper that holds a delegate-remove callback and invokes it on
 *     destruction. Replaces the 18+ duplicated 'AddUObject + RemoveAll
 *     mirror' patterns across the PGX subsystems.
 *
 *     Typical usage:
 *
 *     void UMySubsystem::Initialize(FSubsystemCollectionBase& Collection)
 *     {
 *         Super::Initialize(Collection);
 *         SomeDelegate.AddUObject(this, &UMySubsystem::Handler);
 *         DelegateGuard = MakeShared<FPGXDelegateGuard>([this]()
 *         {
 *             SomeDelegate.RemoveAll(this);
 *         });
 *     }
 *
 *     void UMySubsystem::Deinitialize()
 *     {
 *         DelegateGuard.Reset();  // fires the lambda, removes from delegate
 *         Super::Deinitialize();
 *     }
 *
 *     Why a lambda + TFunction instead of a templated AddX/RemoveY helper:
 *     UE multicast delegates have many variants (no-arg, args, world+args,
 *     FName, gameplay tag payload, etc.) and a single template signature
 *     would either be massively constrained or require macro magic. The
 *     lambda captures the exact Add* + Remove* pair the caller already uses
 *     — zero abstraction loss, full type safety.
 *
 * ES: Helper RAII que guarda un callback de remove de delegado y lo invoca
 *     en destruccion. Reemplaza los 18+ patrones duplicados de
 *     'AddUObject + RemoveAll mirror'.
 */
class PGXCORERUNTIME_API FPGXDelegateGuard
{
public:
	FPGXDelegateGuard() = default;

	/**
	 * EN: Construct from any callable that returns void and takes no args.
	 *     The callable is moved into the guard and invoked on destruction
	 *     or Reset(). Typical lambda captures the delegate + handle.
	 *
	 * ES: Construye desde cualquier callable que retorna void y no toma
	 *     argumentos. El callable se mueve al guard y se invoca en
	 *     destruccion o Reset().
	 */
	template<typename TRemoveFunc>
	explicit FPGXDelegateGuard(TRemoveFunc&& InRemoveFunc)
		: RemoveFunc(Forward<TRemoveFunc>(InRemoveFunc))
	{
	}

	~FPGXDelegateGuard()
	{
		Reset();
	}

	// EN: Non-copyable (the remove callback is a captured reference; copying
	//     would double-fire).
	// ES: No copiable (el callback es una referencia capturada; copiar
	//     dispararia dos veces).
	FPGXDelegateGuard(const FPGXDelegateGuard&) = delete;
	FPGXDelegateGuard& operator=(const FPGXDelegateGuard&) = delete;

	FPGXDelegateGuard(FPGXDelegateGuard&& Other) noexcept
		: RemoveFunc(MoveTemp(Other.RemoveFunc))
	{
		Other.RemoveFunc = nullptr;
	}

	FPGXDelegateGuard& operator=(FPGXDelegateGuard&& Other) noexcept
	{
		if (this != &Other)
		{
			Reset();
			RemoveFunc = MoveTemp(Other.RemoveFunc);
			Other.RemoveFunc = nullptr;
		}
		return *this;
	}

	/**
	 * EN: Fire the remove callback and disarm. Safe to call multiple times
	 *     (idempotent — only the first call invokes the callback).
	 *
	 * ES: Dispara el callback de remove y desarma. Safe de llamar varias
	 *     veces (idempotente — solo la primera llamada invoca el callback).
	 */
	void Reset()
	{
		if (RemoveFunc)
		{
			TFunction<void()> Local = MoveTemp(RemoveFunc);
			Local();
		}
	}

	/**
	 * EN: True iff a remove callback is currently armed. After Reset() or
	 *     move-construct, this returns false.
	 *
	 * ES: True si un callback de remove esta armado. Despues de Reset() o
	 *     move-construct, retorna false.
	 */
	bool IsArmed() const { return RemoveFunc != nullptr; }

private:
	TFunction<void()> RemoveFunc;
};
