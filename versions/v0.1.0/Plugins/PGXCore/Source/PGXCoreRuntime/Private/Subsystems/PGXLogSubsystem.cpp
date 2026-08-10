// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Subsystems/PGXLogSubsystem.h"
#include "Logging/PGXLogMacros.h"   // GPGXLogAddEntry definition + FPGXLogEntry
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogTagHelper.h"
#include "Logging/PGXLogSerializer.h"
#include "Logging/PGXLogSettings.h"
#include "Data/PGXLogConfig.h"
#include "Utils/PGXConfigResolution.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Tasks/Task.h"
#include "Async/Async.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Trace/PGXTraceHelper.h"
#include "Trace/PGXTraceTags.h"
#include "Profile/PGXProfileSubsystem.h"
#include "Profile/PGXPlatformConfig.h"
#include "Data/PGXLogDomainConfig.h"
#include "Logging/PGXLogDomainRendererBase.h"
#include "Logging/PGXLogDomainTableRow.h"
#include "Logging/PGXLogTags.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"

// EN: Centralized log management subsystem implementation
// ES: Implementacion del subsistema de gestion centralizada de logs

// ═══════════════════════════════════════════════════════════════
// Static Cache / Cache Estatico
// ═══════════════════════════════════════════════════════════════

TWeakObjectPtr<UPGXLogSubsystem> UPGXLogSubsystem::CachedInstance;

UPGXLogSubsystem* UPGXLogSubsystem::GetCached()
{
	return CachedInstance.Get();
}

// ═══════════════════════════════════════════════════════════════
// Log Submission Function Pointer (decouples macros from header)
// Puntero a Funcion de Envio de Log (desacopla macros del header)
// ═══════════════════════════════════════════════════════════════

// EN: Set by Initialize(), reset by Deinitialize(). No-op default means
//     logs go to UE_LOG but NOT to the PGX ring buffer until the subsystem
//     is ready. This avoids a circular include of PGXLogSubsystem.h in
//     PGXLogMacros.h (which dragged in PGXLogSubsystem.generated.h and its
//     forward declaration of FPGXLogEntry, breaking Editor module compilation).
// ES: Establecido por Initialize(), reiniciado por Deinitialize(). No-op por
//     defecto significa que los logs van a UE_LOG pero NO al ring buffer PGX
//     hasta que el subsistema este listo. Evita el include circular de
//     PGXLogSubsystem.h en PGXLogMacros.h.
FPGXLogAddEntryFunc GPGXLogAddEntry = [](FPGXLogEntry&&) { /* no-op until subsystem initializes */ };

// ═══════════════════════════════════════════════════════════════
// Initialize & Deinitialize / Inicializar y Desinicializar
// ═══════════════════════════════════════════════════════════════
// Lifecycle / Ciclo de Vida
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CachedInstance = this;
	// EN: Activate the log submission function pointer — macros start sending
	//     FPGXLogEntry to the ring buffer from this point forward.
	// ES: Activar el puntero a funcion de envio de log — desde este punto las
	//     macros comienzan a enviar FPGXLogEntry al ring buffer.
	GPGXLogAddEntry = [](FPGXLogEntry&& Entry) {
		if (UPGXLogSubsystem* Inst = GetCached())
		{
			Inst->AddEntry(MoveTemp(Entry));
		}
	};
	RegisterConsoleCommands();
	StartNewSession();
	DiscoverDomainConfigs();

	// EN: Register default trace config for the Log system
	// ES: Registrar config de traza por defecto para el sistema de Log
	FPGXTraceHelper::RegisterSystemTraceConfig(TAG_PGX_System_Log, FPGXTraceConfig());

	// EN: Apply project profile constraints if available
	// ES: Aplicar restricciones del profile de proyecto si esta disponible
	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* ProfileSS = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			if (ProfileSS->IsProfileResolved())
			{
				ApplyProfileConstraints(ProfileSS->GetResolvedProfile());
			}
			ProfileSS->OnProfileChangedNative.AddUObject(this, &UPGXLogSubsystem::HandleProfileChanged);
		}
	}

	UE_LOG(LogPGXLog, Log, TEXT("PGX Log Subsystem initialized | SessionId=%d | BufferCapacity=%d"),
		CurrentSession.SessionId, RingBuffer.Num());
}

void UPGXLogSubsystem::Deinitialize()
{
	// EN: Cleanup Profile delegate subscription / ES: Limpiar suscripcion a delegate de Profile
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPGXProfileSubsystem* Profile = GI->GetSubsystem<UPGXProfileSubsystem>())
		{
			Profile->OnProfileChangedNative.RemoveAll(this);
		}
	}

	UE_LOG(LogPGXLog, Log, TEXT("PGX Log Subsystem deinitializing | SessionId=%d | Entries=%d"),
		CurrentSession.SessionId, CurrentSession.EntryCount);

	EndCurrentSession();

	// EN: Auto-export if config says so
	// ES: Auto-exportar si la config lo indica
	if (ActiveConfig && ActiveConfig->bExportJsonOnSessionEnd)
	{
		ExportToJsonAsync();
	}

	// EN: Cancel auto-save timer
	// ES: Cancelar timer de auto-guardado
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
		}
	}

	UnregisterConsoleCommands();
	FPGXTraceHelper::UnregisterSystemTraceConfig(TAG_PGX_System_Log);
	GPGXLogAddEntry = [](FPGXLogEntry&&) { /* no-op after subsystem deinitialized */ };
	CachedInstance.Reset();

	Super::Deinitialize();
}

// ═══════════════════════════════════════════════════════════════
// Entry / Entrada
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::AddEntry(FPGXLogEntry&& Entry)
{
	// EN: Stamp session ID
	// ES: Estampar ID de sesion
	Entry.SessionId = CurrentSession.SessionId;

	// EN: Auto-populate domain tag if not set (backward compat with v2.2)
	// ES: Auto-popular tag de dominio si no esta establecido (backward compat con v2.2)
	if (!Entry.DomainTag.IsValid())
	{
		AutoPopulateDomainTag(Entry);
	}

	// EN: Auto-tag fallback: if entry has no tags (came from PGX_LOG_INTERNAL macro), populate them
	// ES: Fallback de auto-tag: si la entrada no tiene tags (vino de macro PGX_LOG_INTERNAL), popularlos
	if (Entry.Tags.IsEmpty())
	{
		FPGXLogTagHelper::AutoPopulateTags(Entry);
	}

	// EN: Filter check
	// ES: Verificar filtro
	if (!PassesFilter(Entry))
	{
		return;
	}

	// EN: Update session counters thread-safely before push (entry may be moved)
	// ES: Actualizar contadores de sesion thread-safe antes del push (la entry puede ser movida)
	{
		FScopeLock Lock(&SessionCounterLock);
		CurrentSession.EntryCount++;
		if (Entry.Verbosity == EPGXLogVerbosity::Warning)
		{
			CurrentSession.WarningCount++;
		}
		else if (Entry.Verbosity >= EPGXLogVerbosity::Error)
		{
			CurrentSession.ErrorCount++;
		}
	}

	// EN: On-screen display if enabled and meets verbosity threshold
	// ES: Display en pantalla si esta habilitado y cumple umbral de verbosity
	if (bScreenPrintEnabled && Entry.Verbosity >= ScreenMinVerbosity)
	{
		PrintToScreen(Entry);
	}

	// EN: Broadcast delegate if enabled and meets verbosity threshold
	// ES: Broadcast de delegado si esta habilitado y cumple umbral de verbosity
	const bool bShouldBroadcast = bBroadcastEnabled && Entry.Verbosity >= BroadcastMinVerbosity;
	const bool bHasListeners = OnLogEntryAdded.IsBound() || OnLogEntryAddedNative.IsBound();

	// EN: Keep a copy for broadcast BEFORE moving into ring buffer
	// ES: Mantener copia para broadcast ANTES de mover al ring buffer
	FPGXLogEntry BroadcastCopy;
	if (bShouldBroadcast && bHasListeners)
	{
		BroadcastCopy = Entry;
	}

	// EN: Push to ring buffer (thread-safe, short lock)
	// ES: Push al ring buffer (thread-safe, lock corto)
	RingBuffer.Push(MoveTemp(Entry));

	// EN: Broadcast AFTER push, OUTSIDE of any lock
	// ES: Broadcast DESPUES del push, FUERA de cualquier lock
	if (bShouldBroadcast && bHasListeners)
	{
		// EN: Ensure broadcast always happens on GameThread (editor widgets require it)
		// ES: Asegurar que el broadcast siempre ocurre en GameThread (widgets de editor lo requieren)
		if (!IsInGameThread())
		{
			AsyncTask(ENamedThreads::GameThread, [this, BroadcastCopy = MoveTemp(BroadcastCopy)]()
			{
				OnLogEntryAdded.Broadcast(BroadcastCopy);
				OnLogEntryAddedNative.Broadcast(BroadcastCopy);
			});
		}
		else
		{
			OnLogEntryAdded.Broadcast(BroadcastCopy);
			OnLogEntryAddedNative.Broadcast(BroadcastCopy);
		}
	}
}

bool UPGXLogSubsystem::PassesFilter(const FPGXLogEntry& Entry) const
{
	// EN: Check global minimum verbosity
	// ES: Verificar verbosity minima global
	if (Entry.Verbosity < GlobalMinVerbosity)
	{
		return false;
	}

	// EN: Check per-category override
	// ES: Verificar override por categoria
	if (const EPGXLogVerbosity* Override = CategoryVerbosityOverrides.Find(Entry.Category))
	{
		if (Entry.Verbosity < *Override)
		{
			return false;
		}
	}

	// EN: Check category whitelist (empty = all allowed)
	// ES: Verificar whitelist de categorias (vacio = todas permitidas)
	if (CategoryWhitelist.Num() > 0 && !CategoryWhitelist.Contains(Entry.Category))
	{
		return false;
	}

	return true;
}

// ═══════════════════════════════════════════════════════════════
// Query / Consulta
// ═══════════════════════════════════════════════════════════════

TArray<FPGXLogEntry> UPGXLogSubsystem::GetCurrentSessionEntries() const
{
	return RingBuffer.Snapshot();
}

FPGXLogSession UPGXLogSubsystem::GetCurrentSession() const
{
	return CurrentSession;
}

int32 UPGXLogSubsystem::GetEntryCount() const
{
	return RingBuffer.Num();
}

TArray<FPGXLogEntry> UPGXLogSubsystem::GetEntriesFiltered(EPGXLogVerbosity MinLevel, FName CategoryFilter) const
{
	return RingBuffer.SnapshotFiltered(MinLevel, CategoryFilter);
}

// ═══════════════════════════════════════════════════════════════
// IO Async / IO Asincrono
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::ExportToJsonAsync(const FString& Path)
{
	// EN: Step 1: Snapshot (short lock) → Step 2: Release → Step 3: Serialize in background
	// ES: Paso 1: Snapshot (lock corto) → Paso 2: Release → Paso 3: Serializar en background
	TArray<FPGXLogEntry> Entries = RingBuffer.Snapshot();
	FPGXLogSession SessionCopy = CurrentSession;

	FString ExportPath = Path;
	if (ExportPath.IsEmpty())
	{
		ExportPath = GetDefaultJsonExportPath();
	}

	// EN: Launch on background thread — never serialize under lock
	// ES: Lanzar en hilo background — nunca serializar bajo lock
	UE::Tasks::Launch(TEXT("PGXLogExportJson"),
		[Entries = MoveTemp(Entries), SessionCopy, ExportPath]()
		{
			// EN: Serialize using FPGXLogSerializer (proper JSON with typed params)
			// ES: Serializar usando FPGXLogSerializer (JSON correcto con params tipados)
			const FString JsonString = FPGXLogSerializer::ToJson(Entries, SessionCopy);

			// EN: Ensure directory exists and write
			// ES: Asegurar que el directorio existe y escribir
			const FString Dir = FPaths::GetPath(ExportPath);
			IFileManager::Get().MakeDirectory(*Dir, true);
			if (!FFileHelper::SaveStringToFile(JsonString, *ExportPath, FFileHelper::EEncodingOptions::ForceUTF8))
			{
				UE_LOG(LogPGXLog, Error, TEXT("Failed to export PGX logs to: %s"), *ExportPath);
				return;
			}
			UE_LOG(LogPGXLog, Log, TEXT("PGX Logs exported to: %s (%d entries)"), *ExportPath, Entries.Num());
		});
}

void UPGXLogSubsystem::SaveToSlotAsync()
{
	// EN: Slot persistence is unavailable in this preview; use JSON export.
	// ES: La persistencia por slots no esta disponible en esta version; usa la exportacion JSON.
	UE_LOG(LogPGXLog, Warning, TEXT("SaveToSlotAsync: unavailable in this preview; use ExportToJsonAsync()"));
}

bool UPGXLogSubsystem::LoadFromSlot(int32 /*SessionId*/)
{
	// EN: .sav loading is unavailable while slot persistence remains unsupported.
	// ES: La carga .sav no esta disponible mientras no haya persistencia por slots.
	UE_LOG(LogPGXLog, Log, TEXT("LoadFromSlot: Not yet implemented"));
	return false;
}

// ═══════════════════════════════════════════════════════════════
// Runtime Filtering / Filtrado en Runtime
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::SetGlobalMinVerbosity(EPGXLogVerbosity Level)
{
	GlobalMinVerbosity = Level;
	UE_LOG(LogPGXLog, Log, TEXT("Global min verbosity set to %d"), static_cast<int32>(Level));
}

void UPGXLogSubsystem::SetCategoryVerbosity(FName Category, EPGXLogVerbosity Level)
{
	CategoryVerbosityOverrides.Add(Category, Level);
	UE_LOG(LogPGXLog, Log, TEXT("Category '%s' verbosity set to %d"), *Category.ToString(), static_cast<int32>(Level));
}

void UPGXLogSubsystem::SetScreenPrintEnabled(bool bEnabled)
{
	bScreenPrintEnabled = bEnabled;
	UE_LOG(LogPGXLog, Log, TEXT("Screen print %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

// ═══════════════════════════════════════════════════════════════
// Config / Configuracion
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::ApplyConfig(const UPGXLogConfig* Config)
{
	if (!Config)
	{
		return;
	}

	ActiveConfig = Config;

	// EN: Apply buffer settings
	// ES: Aplicar configuracion de buffer
	RingBuffer.Resize(Config->RingBufferCapacity);

	// EN: Apply filter settings
	// ES: Aplicar configuracion de filtros
	GlobalMinVerbosity = Config->GlobalMinVerbosity;
	CategoryVerbosityOverrides = Config->CategoryOverrides;
	CategoryWhitelist = Config->CategoryWhitelist;

	// EN: Apply screen settings
	// ES: Aplicar configuracion de pantalla
	bScreenPrintEnabled = Config->bShowOnScreen;
	ScreenMinVerbosity = Config->ScreenMinVerbosity;
	ScreenDisplayDuration = Config->ScreenDisplayDuration;

	// EN: Apply delegate settings
	// ES: Aplicar configuracion de delegados
	bBroadcastEnabled = Config->bBroadcastOnEveryEntry;
	BroadcastMinVerbosity = Config->BroadcastMinVerbosity;

	// EN: Apply persistence settings
	// ES: Aplicar configuracion de persistencia
	if (Config->bAutoSaveToSlot && Config->SaveIntervalSeconds > 0.0f)
	{
		ScheduleAutoSave(Config->SaveIntervalSeconds);
	}

	UE_LOG(LogPGXLog, Log, TEXT("Config applied: Buffer=%d, MinVerbosity=%d, Screen=%s"),
		Config->RingBufferCapacity,
		static_cast<int32>(Config->GlobalMinVerbosity),
		Config->bShowOnScreen ? TEXT("on") : TEXT("off"));
}

// ═══════════════════════════════════════════════════════════════
// Screen Display / Display en Pantalla
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::PrintToScreen(const FPGXLogEntry& Entry)
{
#if !UE_BUILD_SHIPPING
	if (!GEngine)
	{
		return;
	}

	// EN: Default color map by verbosity / ES: Mapa de colores default por verbosity
	FColor DisplayColor = FColor::White;
	FString Prefix;
	switch (Entry.Verbosity)
	{
	case EPGXLogVerbosity::Verbose:
		DisplayColor = FColor(128, 128, 128); // Gray
		Prefix = TEXT("[VRB]");
		break;
	case EPGXLogVerbosity::Debug:
		DisplayColor = FColor(0, 200, 200);   // Cyan
		Prefix = TEXT("[DBG]");
		break;
	case EPGXLogVerbosity::Info:
		DisplayColor = FColor::White;
		Prefix = TEXT("[INF]");
		break;
	case EPGXLogVerbosity::Warning:
		DisplayColor = FColor::Yellow;
		Prefix = TEXT("[WRN]");
		break;
	case EPGXLogVerbosity::Error:
		DisplayColor = FColor(255, 80, 80);   // Red
		Prefix = TEXT("[ERR]");
		break;
	case EPGXLogVerbosity::Fatal:
		DisplayColor = FColor(255, 0, 255);   // Magenta
		Prefix = TEXT("[FTL]");
		break;
	}

	// EN: Override with config colors if available
	// ES: Sobrescribir con colores de config si estan disponibles
	if (ActiveConfig)
	{
		if (const FLinearColor* ConfigColor = ActiveConfig->VerbosityColors.Find(Entry.Verbosity))
		{
			DisplayColor = ConfigColor->ToFColor(true);
		}
	}

	const FString ScreenMsg = FString::Printf(TEXT("%s[%s] %s"),
		*Prefix, *Entry.Category.ToString(), *Entry.Message);

	GEngine->AddOnScreenDebugMessage(-1, ScreenDisplayDuration, DisplayColor, ScreenMsg);
#endif
}

// ═══════════════════════════════════════════════════════════════
// Session Management / Gestion de Sesiones
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::StartNewSession()
{
	CurrentSession = FPGXLogSession();
	CurrentSession.SessionId = NextSessionId++;
	CurrentSession.StartTime = FDateTime::UtcNow();

	if (OnSessionUpdated.IsBound())
	{
		OnSessionUpdated.Broadcast();
	}
}

void UPGXLogSubsystem::EndCurrentSession()
{
	CurrentSession.EndTime = FDateTime::UtcNow();

	if (OnSessionUpdated.IsBound())
	{
		OnSessionUpdated.Broadcast();
	}
}

void UPGXLogSubsystem::ScheduleAutoSave(float IntervalSeconds)
{
	if (IntervalSeconds <= 0.0f)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				this,
				&UPGXLogSubsystem::SaveToSlotAsync,
				IntervalSeconds,
				true // looping
			);
		}
	}
}

FString UPGXLogSubsystem::GetDefaultJsonExportPath() const
{
	const FString Dir = ActiveConfig ? ActiveConfig->JsonExportDirectory : TEXT("Logs/PGX");
	const FString Filename = FString::Printf(TEXT("PGXLog_Session_%d_%s.json"),
		CurrentSession.SessionId,
		*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));

	return FPaths::Combine(FPaths::ProjectSavedDir(), Dir, Filename);
}

// ═══════════════════════════════════════════════════════════════
// Console Commands / Comandos de Consola
// ═══════════════════════════════════════════════════════════════

void UPGXLogSubsystem::RegisterConsoleCommands()
{
	IConsoleManager& Console = IConsoleManager::Get();

	// EN: pgx.log.stats — Show entry counts by verbosity
	// ES: pgx.log.stats — Mostrar conteos por verbosity
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.stats"),
		TEXT("Show PGX log statistics for current session"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXLog, Log, TEXT("═══ PGX Log Stats ═══"));
			UE_LOG(LogPGXLog, Log, TEXT("Session ID:  %d"), CurrentSession.SessionId);
			UE_LOG(LogPGXLog, Log, TEXT("Total:       %d"), CurrentSession.EntryCount);
			UE_LOG(LogPGXLog, Log, TEXT("Warnings:    %d"), CurrentSession.WarningCount);
			UE_LOG(LogPGXLog, Log, TEXT("Errors:      %d"), CurrentSession.ErrorCount);
			UE_LOG(LogPGXLog, Log, TEXT("Buffer:      %d / capacity"), RingBuffer.Num());
			UE_LOG(LogPGXLog, Log, TEXT("Min Verb:    %d"), static_cast<int32>(GlobalMinVerbosity));
			UE_LOG(LogPGXLog, Log, TEXT("Screen:      %s"), bScreenPrintEnabled ? TEXT("on") : TEXT("off"));
		}),
		ECVF_Default
	));

	// EN: pgx.log.list — List registered categories
	// ES: pgx.log.list — Listar categorias registradas
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.list"),
		TEXT("List all PGX log categories and their verbosity overrides"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXLog, Log, TEXT("═══ PGX Log Categories ═══"));
			UE_LOG(LogPGXLog, Log, TEXT("  LogPGX, LogPGXLog, LogPGXMessages, LogPGXSettings"));
			UE_LOG(LogPGXLog, Log, TEXT("  LogPGXPool, LogPGXData, LogPGXTags, LogPGXStateMachine"));
			if (CategoryVerbosityOverrides.Num() > 0)
			{
				UE_LOG(LogPGXLog, Log, TEXT("Overrides:"));
				for (const auto& Pair : CategoryVerbosityOverrides)
				{
					UE_LOG(LogPGXLog, Log, TEXT("  %s → %d"), *Pair.Key.ToString(), static_cast<int32>(Pair.Value));
				}
			}
		}),
		ECVF_Default
	));

	// EN: pgx.log.export [path] — Export to JSON async
	// ES: pgx.log.export [path] — Exportar a JSON async
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.export"),
		TEXT("Export PGX logs to JSON file. Usage: pgx.log.export [optional_path]"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			const FString Path = Args.Num() > 0 ? Args[0] : TEXT("");
			ExportToJsonAsync(Path);
			UE_LOG(LogPGXLog, Log, TEXT("Export started..."));
		}),
		ECVF_Default
	));

	// EN: pgx.log.filter [category] [level|on|off|screen] — Unified filter control
	// ES: pgx.log.filter [category] [level|on|off|screen] — Control unificado de filtros
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.filter"),
		TEXT("Filter control. Usage: pgx.log.filter [category level] | pgx.log.filter screen [on|off] | pgx.log.filter global [0-5]"),
		FConsoleCommandWithArgsDelegate::CreateWeakLambda(this, [this](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				UE_LOG(LogPGXLog, Log, TEXT("Usage: pgx.log.filter <category> <0-5> | screen <on|off> | global <0-5>"));
				return;
			}

			if (Args[0].Equals(TEXT("screen"), ESearchCase::IgnoreCase))
			{
				const bool bOn = Args.Num() > 1 && Args[1].Equals(TEXT("on"), ESearchCase::IgnoreCase);
				SetScreenPrintEnabled(bOn);
			}
			else if (Args[0].Equals(TEXT("global"), ESearchCase::IgnoreCase) && Args.Num() > 1)
			{
				const int32 Level = FCString::Atoi(*Args[1]);
				SetGlobalMinVerbosity(static_cast<EPGXLogVerbosity>(FMath::Clamp(Level, 0, 5)));
			}
			else if (Args.Num() > 1)
			{
				const FName Cat(*Args[0]);
				const int32 Level = FCString::Atoi(*Args[1]);
				SetCategoryVerbosity(Cat, static_cast<EPGXLogVerbosity>(FMath::Clamp(Level, 0, 5)));
			}
		}),
		ECVF_Default
	));

	// EN: pgx.log.domains — List registered log domains
	// ES: pgx.log.domains — Listar dominios de log registrados
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.domains"),
		TEXT("List all registered PGX log domains with entry counts"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			UE_LOG(LogPGXLog, Log, TEXT("═══ PGX Log Domains ═══"));
			if (DomainRegistry.Num() == 0)
			{
				UE_LOG(LogPGXLog, Log, TEXT("  (no domains registered — create UPGXLogDomainConfig DAs)"));
			}
			for (const auto& Pair : DomainRegistry)
			{
				const FString DomainName = Pair.Value ? Pair.Value->DomainDisplayName.ToString() : TEXT("?");
				const int32 DomainCount = RingBuffer.SnapshotByDomain(Pair.Key).Num();
				UE_LOG(LogPGXLog, Log, TEXT("  %s — %s — %d entries"),
					*Pair.Key.ToString(), *DomainName, DomainCount);
			}
			UE_LOG(LogPGXLog, Log, TEXT("Total domains: %d"), DomainRegistry.Num());
		}),
		ECVF_Default
	));

	// EN: pgx.log.clear — Clear ring buffer
	// ES: pgx.log.clear — Limpiar ring buffer
	RegisteredCommands.Add(Console.RegisterConsoleCommand(
		TEXT("pgx.log.clear"),
		TEXT("Clear the PGX log ring buffer"),
		FConsoleCommandDelegate::CreateWeakLambda(this, [this]()
		{
			RingBuffer.Clear();
			UE_LOG(LogPGXLog, Log, TEXT("Ring buffer cleared"));
		}),
		ECVF_Default
	));
}

void UPGXLogSubsystem::UnregisterConsoleCommands()
{
	IConsoleManager& Console = IConsoleManager::Get();
	for (IConsoleObject* Cmd : RegisteredCommands)
	{
		Console.UnregisterConsoleObject(Cmd);
	}
	RegisteredCommands.Empty();
}

// ═══════════════════════════════════════════════════════════════
// Domain Registry (v3.0)
// ═══════════════════════════════════════════════════════════════

TArray<FPGXLogEntry> UPGXLogSubsystem::GetEntriesForDomain(const FGameplayTag& DomainTag) const
{
	return RingBuffer.SnapshotByDomain(DomainTag);
}

const UPGXLogDomainConfig* UPGXLogSubsystem::FindDomainConfig(const FGameplayTag& DomainTag) const
{
	if (const TObjectPtr<const UPGXLogDomainConfig>* Found = DomainRegistry.Find(DomainTag))
	{
		return Found->Get();
	}
	return nullptr;
}

UPGXLogDomainRendererBase* UPGXLogSubsystem::GetRendererForDomain(const FGameplayTag& DomainTag)
{
	// EN: Check cache first / ES: Verificar cache primero
	if (TObjectPtr<UPGXLogDomainRendererBase>* Cached = RendererCache.Find(DomainTag))
	{
		if (IsValid(*Cached))
		{
			return *Cached;
		}
	}

	// EN: Look up domain config / ES: Buscar config de dominio
	const UPGXLogDomainConfig* Config = FindDomainConfig(DomainTag);
	if (!Config)
	{
		return GetGenericRenderer();
	}

	// EN: Resolve renderer class from config / ES: Resolver clase de renderer desde config
	UClass* RendererClass = nullptr;
	switch (Config->RendererSourceMode)
	{
	case EPGXClassSourceMode::CppClass:
		RendererClass = Config->CppRendererClass;
		break;
	case EPGXClassSourceMode::Blueprint:
		RendererClass = Config->BlueprintRendererClass.LoadSynchronous();
		break;
	default:
		break;
	}

	if (!RendererClass)
	{
		return GetGenericRenderer();
	}

	// EN: Create and cache / ES: Crear y cachear
	UPGXLogDomainRendererBase* Renderer = NewObject<UPGXLogDomainRendererBase>(const_cast<UPGXLogSubsystem*>(this), RendererClass);
	RendererCache.Add(DomainTag, Renderer);
	return Renderer;
}

UPGXLogDomainRendererBase* UPGXLogSubsystem::GetGenericRenderer()
{
	if (!IsValid(GenericRendererInstance))
	{
		GenericRendererInstance = NewObject<UPGXLogDomainRendererBase>(this);
	}
	return GenericRendererInstance;
}

void UPGXLogSubsystem::DiscoverDomainConfigs()
{
	const UPGXLogSettings* Settings = GetDefault<UPGXLogSettings>();

	// EN: Phase 1: Settings-first — resolve Log config DA
	// ES: Fase 1: Settings primero — resolver DA de config de Log
	ActiveConfig = PGX::ResolveSingleConfig<UPGXLogConfig>(Settings->ActiveConfig, TEXT("Log"));

	// EN: Phase 2: Domain configs — Settings array first, then AssetRegistry fallback
	// ES: Fase 2: Configs de dominio — Array de Settings primero, luego fallback AssetRegistry
	TArray<const UPGXLogDomainConfig*> DomainConfigsToProcess;

	if (Settings->DomainConfigs.Num() > 0)
	{
		// EN: Load from Settings (deterministic)
		// ES: Cargar desde Settings (deterministico)
		for (const TSoftObjectPtr<UPGXLogDomainConfig>& DomainRef : Settings->DomainConfigs)
		{
			if (DomainRef.IsNull()) { continue; }
			UPGXLogDomainConfig* DomainCfg = DomainRef.LoadSynchronous();
			if (IsValid(DomainCfg))
			{
				DomainConfigsToProcess.Add(DomainCfg);
			}
			else
			{
				UE_LOG(LogPGXLog, Warning, TEXT("[Log] Domain config assigned in Settings but failed to load: %s"),
					*DomainRef.ToString());
			}
		}
		UE_LOG(LogPGXLog, Log, TEXT("[Log] %d domain configs resolved from Project Settings."), DomainConfigsToProcess.Num());
	}
	else
	{
		// EN: AssetRegistry fallback (deprecated)
		// ES: Fallback AssetRegistry (deprecated)
		FAssetRegistryModule& ARModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AR = ARModule.Get();

		TArray<FAssetData> Assets;
		AR.GetAssetsByClass(UPGXLogDomainConfig::StaticClass()->GetClassPathName(), Assets, true);

		for (const FAssetData& AssetData : Assets)
		{
			if (const UPGXLogDomainConfig* Config = Cast<UPGXLogDomainConfig>(AssetData.GetAsset()))
			{
				DomainConfigsToProcess.Add(Config);
			}
		}

		if (DomainConfigsToProcess.Num() > 0)
		{
			UE_LOG(LogPGXLog, Warning, TEXT("[Log] %d domain configs auto-discovered from AssetRegistry. "
				"Assign them in Project Settings > PGX > Log System > Domain Configs to remove this warning. "
				"Auto-discovery is deprecated and will be removed in v0.6.0."),
				DomainConfigsToProcess.Num());
		}
	}

	// EN: Phase 3: Process all domain configs (same logic regardless of source)
	// ES: Fase 3: Procesar todos los configs de dominio (misma logica sin importar fuente)
	int32 DataTableRowsIngested = 0;

	for (const UPGXLogDomainConfig* Config : DomainConfigsToProcess)
	{
		RegisterDomain(Config);

		// EN: Ingest DataTable rows if the DA has a DomainDataTable assigned
		// ES: Ingestar filas de DataTable si el DA tiene un DomainDataTable asignado
		if (IsValid(Config->DomainDataTable))
		{
			const TMap<FName, uint8*>& RowMap = Config->DomainDataTable->GetRowMap();
			for (const auto& Pair : RowMap)
			{
				const FPGXLogDomainTableRow* Row = reinterpret_cast<const FPGXLogDomainTableRow*>(Pair.Value);
				if (!Row || !Row->DomainTag.IsValid())
				{
					continue;
				}

				// EN: Skip if this domain is already registered (DA takes priority over DataTable rows)
				// ES: Saltar si este dominio ya esta registrado (DA tiene prioridad sobre filas de DataTable)
				if (DomainRegistry.Contains(Row->DomainTag))
				{
					continue;
				}

				// EN: Build auto-infer map from DataTable row categories
				// ES: Construir mapa auto-infer desde categorias de fila DataTable
				for (const FName& Cat : Row->AutoInferCategories)
				{
					CategoryToDomainMap.Add(Cat, Row->DomainTag);
				}

				DataTableRowsIngested++;
			}
		}
	}

	UE_LOG(LogPGXLog, Log, TEXT("Domain discovery complete — %d domains registered, %d auto-infer categories, %d DataTable rows ingested"),
		DomainRegistry.Num(), CategoryToDomainMap.Num(), DataTableRowsIngested);
}

void UPGXLogSubsystem::RegisterDomain(const UPGXLogDomainConfig* Config)
{
	if (!Config || !Config->DomainTag.IsValid())
	{
		return;
	}

	DomainRegistry.Add(Config->DomainTag, Config);

	// EN: Build auto-infer map from DA's category list / ES: Construir mapa auto-infer desde lista de categorias del DA
	for (const FName& Cat : Config->AutoInferCategories)
	{
		CategoryToDomainMap.Add(Cat, Config->DomainTag);
	}

	OnDomainRegistered.Broadcast(Config->DomainTag);
}

void UPGXLogSubsystem::AutoPopulateDomainTag(FPGXLogEntry& Entry) const
{
	// EN: 1. Check DA-defined category→domain map / ES: 1. Verificar mapa categoria→dominio definido por DA
	if (const FGameplayTag* Found = CategoryToDomainMap.Find(Entry.Category))
	{
		Entry.DomainTag = *Found;
		return;
	}

	// EN: 2. Fallback to built-in heuristic / ES: 2. Fallback a heuristica built-in
	Entry.DomainTag = FPGXLogTagHelper::AutoInferDomainTag(Entry.Category);
}

// ============================================================================
// Profile Integration
// ============================================================================

void UPGXLogSubsystem::ApplyProfileConstraints(const FPGXResolvedProfile& Profile)
{
	// EN: Enforce Log platform budgets from active PlatformConfig
	// ES: Aplicar presupuestos de plataforma Log desde PlatformConfig activa
	int32 EnforcedRingBufferCapacity = 0;
	int32 EnforcedMaxLogFiles = 0;

	if (auto* ProfileSS = UPGXProfileSubsystem::GetCachedInstance())
	{
		if (const UPGXPlatformConfig* PlatformCfg = ProfileSS->GetActivePlatformConfig())
		{
			const auto& B = PlatformCfg->LogBudgets;
			EnforcedRingBufferCapacity = B.RingBufferCapacity;
			EnforcedMaxLogFiles = B.MaxLogFiles;
		}
	}

	UE_LOG(LogPGXLog, Log, TEXT("[LogSubsystem] Profile constraints enforced — RingBuffer=%d, MaxFiles=%d, PersistentLogs=%d, Backend=%d, StripDebug=%d"),
		EnforcedRingBufferCapacity, EnforcedMaxLogFiles,
		Profile.Capabilities.bAllowPersistentLogs,
		static_cast<int32>(Profile.Policies.Persistence.LogBackend),
		Profile.Policies.Security.bStripDebugStrings);
}

void UPGXLogSubsystem::HandleProfileChanged(const FPGXResolvedProfile& /*OldProfile*/, const FPGXResolvedProfile& NewProfile)
{
	ApplyProfileConstraints(NewProfile);
}
