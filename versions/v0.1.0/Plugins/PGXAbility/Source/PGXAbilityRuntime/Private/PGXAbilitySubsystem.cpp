// Copyright PGX Framework. All Rights Reserved.

#include "PGXAbilitySubsystem.h"
#include "PGXAbilityRuntime.h"
#include "PGXAbilityComponent.h"
#include "PGXAbilityConfig.h"
#include "PGXAbilitySettings.h"
#include "Utils/PGXConfigResolution.h"
#include "Subsystems/PGXLogSubsystem.h"
#include "HAL/IConsoleManager.h"

void UPGXAbilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ComponentRegistry.Reset();
	DiscoverConfig();
	RegisterConsoleCommands();

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilitySubsystem: Initialized (config: %s)"),
		ActiveConfig ? *ActiveConfig->GetName() : TEXT("<defaults>"));
}

void UPGXAbilitySubsystem::Deinitialize()
{
	const int32 Leaked = ComponentRegistry.Num();
	if (Leaked > 0)
	{
		PGX_LOG_WARNING(LogPGXAbility,
			TEXT("UPGXAbilitySubsystem: Deinitialize with %d component(s) still registered. Owners should unregister during EndPlay."),
			Leaked);
	}

	UnregisterConsoleCommands();
	ComponentRegistry.Reset();
	ActiveConfig = nullptr;
	OnAbilityActivatedNative.Clear();
	OnComponentRegisteredNative.Clear();

	PGX_LOG_INFO(LogPGXAbility, TEXT("UPGXAbilitySubsystem: Deinitialized"));

	Super::Deinitialize();
}

void UPGXAbilitySubsystem::RegisterConsoleCommands()
{
	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.ability.status"),
		TEXT("Print PGXAbilitySubsystem readiness, config source, and aggregate counts."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			UGameInstance* GameInstance = World->GetGameInstance();
			UPGXAbilitySubsystem* Sub = GameInstance ? GameInstance->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;
			if (!Sub)
			{
				PGX_LOG_WARNING(LogPGXAbility, TEXT("pgx.ability.status — subsystem unavailable."));
				return;
			}
			PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbility status — config=%s, components=%d, active abilities=%d"),
				Sub->GetActiveConfig() ? *Sub->GetActiveConfig()->GetName() : TEXT("<defaults>"),
				Sub->GetRegisteredComponentCount(),
				Sub->GetActiveAbilityCount());
		}),
		ECVF_Default));

	RegisteredCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("pgx.ability.components"),
		TEXT("List PGXAbilityComponent instances currently registered with the subsystem."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!World) return;
			UGameInstance* GameInstance = World->GetGameInstance();
			UPGXAbilitySubsystem* Sub = GameInstance ? GameInstance->GetSubsystem<UPGXAbilitySubsystem>() : nullptr;
			if (!Sub)
			{
				PGX_LOG_WARNING(LogPGXAbility, TEXT("pgx.ability.components — subsystem unavailable."));
				return;
			}
			const TArray<TWeakObjectPtr<UPGXAbilityComponent>> Registry = Sub->GetComponentRegistry();
			PGX_LOG_INFO(LogPGXAbility, TEXT("PGXAbility components (%d):"), Registry.Num());
			for (const TWeakObjectPtr<UPGXAbilityComponent>& Entry : Registry)
			{
				if (UPGXAbilityComponent* Component = Entry.Get())
				{
					PGX_LOG_INFO(LogPGXAbility, TEXT("  - %s (owner=%s, active abilities=%d, ASC ready=%d)"),
						*Component->GetName(), *GetNameSafe(Component->GetOwner()),
						Component->GetActiveAbilityCount(), Component->IsAbilitySystemReady());
				}
			}
		}),
		ECVF_Default));
}

void UPGXAbilitySubsystem::UnregisterConsoleCommands()
{
	for (IConsoleObject* Command : RegisteredCommands)
	{
		if (Command)
		{
			IConsoleManager::Get().UnregisterConsoleObject(Command);
		}
	}
	RegisteredCommands.Reset();
}

void UPGXAbilitySubsystem::DiscoverConfig()
{
	// EN: Settings-first resolution, same pattern as PGXLoading/PGXAudio/PGXGameFlow.
	// ES: Resolucion Settings-first, mismo patron que PGXLoading/PGXAudio/PGXGameFlow.
	const UPGXAbilitySettings* Settings = GetDefault<UPGXAbilitySettings>();
	ActiveConfig = PGX::ResolveSingleConfig<UPGXAbilityConfig>(Settings->ActiveConfig, TEXT("Ability"));

	if (!ActiveConfig)
	{
		PGX_LOG_WARNING(LogPGXAbility, TEXT("No UPGXAbilityConfig found — using field defaults."));
	}
}

void UPGXAbilitySubsystem::RegisterComponent(UPGXAbilityComponent* Component)
{
	if (!IsValid(Component))
	{
		return;
	}

	// EN: Idempotency — never add the same component twice.
	// ES: Idempotencia — nunca agregar el mismo componente dos veces.
	for (const TWeakObjectPtr<UPGXAbilityComponent>& Existing : ComponentRegistry)
	{
		if (Existing.Get() == Component)
		{
			return;
		}
	}

	ComponentRegistry.Add(Component);
	OnComponentRegisteredNative.Broadcast(Component, /*bRegistered=*/true);

	PGX_LOG_VERBOSE(LogPGXAbility, TEXT("UPGXAbilitySubsystem::RegisterComponent — %s (owner=%s)"),
		*Component->GetName(), *GetNameSafe(Component->GetOwner()));
}

void UPGXAbilitySubsystem::UnregisterComponent(UPGXAbilityComponent* Component)
{
	if (!IsValid(Component))
	{
		return;
	}

	const int32 RemovedCount = ComponentRegistry.RemoveAll([Component](const TWeakObjectPtr<UPGXAbilityComponent>& Entry)
	{
		return Entry.Get() == Component;
	});

	if (RemovedCount > 0)
	{
		OnComponentRegisteredNative.Broadcast(Component, /*bRegistered=*/false);
		PGX_LOG_VERBOSE(LogPGXAbility, TEXT("UPGXAbilitySubsystem::UnregisterComponent — %s"), *Component->GetName());
	}
}

TArray<TWeakObjectPtr<UPGXAbilityComponent>> UPGXAbilitySubsystem::GetComponentRegistry() const
{
	TArray<TWeakObjectPtr<UPGXAbilityComponent>> Snapshot;
	Snapshot.Reserve(ComponentRegistry.Num());
	for (const TWeakObjectPtr<UPGXAbilityComponent>& Entry : ComponentRegistry)
	{
		if (Entry.IsValid())
		{
			Snapshot.Add(Entry);
		}
	}
	return Snapshot;
}

int32 UPGXAbilitySubsystem::GetActiveAbilityCount() const
{
	int32 Total = 0;
	for (const TWeakObjectPtr<UPGXAbilityComponent>& Entry : ComponentRegistry)
	{
		if (UPGXAbilityComponent* Component = Entry.Get())
		{
			Total += Component->GetActiveAbilityCount();
		}
	}
	return Total;
}
