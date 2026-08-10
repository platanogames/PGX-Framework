// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXAudioTestUtility.h"
#include "Logging/PGXLogMacros.h"
#include "PGXAudioSubsystem.h"
#include "PGXAudioLog.h"
#include "PGXAudioConfig.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXSoundDefinition.h"
#include "Data/PGXMusicPlaylist.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// EN: Audio test utility implementation
// ES: Implementacion de utilidad de test de audio

static UPGXAudioSubsystem* GetSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}
	const UGameInstance* GI = World->GetGameInstance();
	return GI ? GI->GetSubsystem<UPGXAudioSubsystem>() : nullptr;
}

bool UPGXAudioTestUtility::ValidateSystemHealth(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	const UPGXAudioSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("AudioSubsystem not found"));
		return false;
	}

	if (Sub->GetAudioState() != EPGXAudioState::Ready)
	{
		OutIssues.Add(FString::Printf(TEXT("AudioState is %s (expected Ready)"),
			*UEnum::GetValueAsString(Sub->GetAudioState())));
	}

	if (!Sub->GetActiveBackend())
	{
		OutIssues.Add(TEXT("No active backend"));
	}

	if (!Sub->GetMusicManager())
	{
		OutIssues.Add(TEXT("MusicManager is null"));
	}

	if (!Sub->GetSoundPool())
	{
		OutIssues.Add(TEXT("SoundPool is null"));
	}

	if (!Sub->GetDialogueManager())
	{
		OutIssues.Add(TEXT("DialogueManager is null"));
	}

	if (!Sub->GetAudioConfig())
	{
		OutIssues.Add(TEXT("AudioConfig DA not loaded (using defaults)"));
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("ValidateSystemHealth — %d issues found"), OutIssues.Num());
	return OutIssues.Num() == 0;
}

bool UPGXAudioTestUtility::ValidateChannelConfigs(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	// EN: Use subsystem cached configs instead of AssetRegistry — validates what the system actually has
	// ES: Usar configs cacheados del subsistema en vez de AssetRegistry — valida lo que el sistema realmente tiene
	const UPGXAudioSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("AudioSubsystem not found"));
		return false;
	}

	const auto& Configs = Sub->GetCachedChannelConfigs();
	if (Configs.Num() == 0)
	{
		OutIssues.Add(TEXT("No channel configs in subsystem cache"));
		return false;
	}

	int32 ValidCount = 0;
	for (const TWeakObjectPtr<const UPGXAudioChannelConfig>& WeakCfg : Configs)
	{
		const UPGXAudioChannelConfig* Config = WeakCfg.Get();
		if (!Config)
		{
			OutIssues.Add(TEXT("Stale weak pointer in channel config cache"));
			continue;
		}

		if (!Config->ChannelTag.IsValid())
		{
			OutIssues.Add(FString::Printf(TEXT("%s: Invalid ChannelTag"), *Config->GetName()));
		}

		if (Config->ChannelDisplayName.IsEmpty())
		{
			OutIssues.Add(FString::Printf(TEXT("%s: Empty ChannelDisplayName"), *Config->GetName()));
		}

		ValidCount++;
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("ValidateChannelConfigs — %d configs in cache, %d issues"), ValidCount, OutIssues.Num());
	return OutIssues.Num() == 0;
}

bool UPGXAudioTestUtility::ValidateSoundDefinitions(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	const FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& Registry = Module.Get();

	TArray<FAssetData> FoundAssets;
	Registry.GetAssetsByClass(UPGXSoundDefinition::StaticClass()->GetClassPathName(), FoundAssets);

	if (FoundAssets.Num() == 0)
	{
		OutIssues.Add(TEXT("No UPGXSoundDefinition DAs found (not necessarily an error)"));
		return true;
	}

	for (const FAssetData& Asset : FoundAssets)
	{
		const UPGXSoundDefinition* Def = Cast<UPGXSoundDefinition>(Asset.GetAsset());
		if (!Def)
		{
			OutIssues.Add(FString::Printf(TEXT("Failed to load: %s"), *Asset.GetObjectPathString()));
			continue;
		}

		if (!Def->SoundTag.IsValid())
		{
			OutIssues.Add(FString::Printf(TEXT("%s: Invalid SoundTag"), *Def->GetName()));
		}

		if (Def->Variants.Num() == 0)
		{
			OutIssues.Add(FString::Printf(TEXT("%s: No variants defined"), *Def->GetName()));
		}

		for (int32 i = 0; i < Def->Variants.Num(); ++i)
		{
			if (Def->Variants[i].Sounds.Num() == 0)
			{
				OutIssues.Add(FString::Printf(TEXT("%s: Variant %d has no sound assets"), *Def->GetName(), i));
			}
		}
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("ValidateSoundDefinitions — %d definitions, %d issues"), FoundAssets.Num(), OutIssues.Num());
	return OutIssues.Num() == 0;
}

bool UPGXAudioTestUtility::TestSoundResolution(const UObject* WorldContextObject, FGameplayTag SoundTag,
	const FGameplayTagContainer& ContextTags, FString& OutResolvedName)
{
	OutResolvedName.Empty();

	UPGXAudioSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		return false;
	}

	// EN: Find SoundDefinition by tag / ES: Encontrar SoundDefinition por tag
	const FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& Registry = Module.Get();

	TArray<FAssetData> FoundAssets;
	Registry.GetAssetsByClass(UPGXSoundDefinition::StaticClass()->GetClassPathName(), FoundAssets);

	for (const FAssetData& Asset : FoundAssets)
	{
		const UPGXSoundDefinition* Def = Cast<UPGXSoundDefinition>(Asset.GetAsset());
		if (Def && Def->SoundTag == SoundTag)
		{
			USoundBase* Resolved = Sub->ResolveSound(Def, ContextTags);
			if (Resolved)
			{
				OutResolvedName = Resolved->GetName();
				return true;
			}
			break;
		}
	}

	return false;
}

bool UPGXAudioTestUtility::TestBackendSwitch(const UObject* WorldContextObject, EPGXAudioBackendType TargetType,
	TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	UPGXAudioSubsystem* Sub = GetSubsystem(WorldContextObject);
	if (!Sub)
	{
		OutIssues.Add(TEXT("AudioSubsystem not found"));
		return false;
	}

	const EPGXAudioBackendType OriginalType = Sub->GetActiveBackendType();

	if (!Sub->SwitchBackend(TargetType))
	{
		OutIssues.Add(FString::Printf(TEXT("Failed to switch to %s"), *UEnum::GetValueAsString(TargetType)));
		return false;
	}

	if (Sub->GetActiveBackendType() != TargetType && TargetType != EPGXAudioBackendType::Auto)
	{
		OutIssues.Add(TEXT("Backend type doesn't match after switch"));
	}

	// EN: Switch back to original / ES: Volver al original
	if (!Sub->SwitchBackend(OriginalType))
	{
		OutIssues.Add(TEXT("Failed to switch back to original backend"));
	}

	return OutIssues.Num() == 0;
}

bool UPGXAudioTestUtility::ValidateMusicPlaylists(const UObject* /*WorldContextObject*/, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	const FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	const IAssetRegistry& Registry = Module.Get();

	TArray<FAssetData> FoundAssets;
	Registry.GetAssetsByClass(UPGXMusicPlaylist::StaticClass()->GetClassPathName(), FoundAssets);

	for (const FAssetData& Asset : FoundAssets)
	{
		const UPGXMusicPlaylist* Playlist = Cast<UPGXMusicPlaylist>(Asset.GetAsset());
		if (!Playlist)
		{
			OutIssues.Add(FString::Printf(TEXT("Failed to load: %s"), *Asset.GetObjectPathString()));
			continue;
		}

		if (Playlist->Tracks.Num() == 0)
		{
			OutIssues.Add(FString::Printf(TEXT("%s: No tracks defined"), *Playlist->GetName()));
		}

		for (int32 i = 0; i < Playlist->Tracks.Num(); ++i)
		{
			if (Playlist->Tracks[i].Music.IsNull())
			{
				OutIssues.Add(FString::Printf(TEXT("%s: Track %d has null music reference"), *Playlist->GetName(), i));
			}
		}
	}

	PGX_LOG_INFO(LogPGXAudio, TEXT("ValidateMusicPlaylists — %d playlists, %d issues"), FoundAssets.Num(), OutIssues.Num());
	return OutIssues.Num() == 0;
}

bool UPGXAudioTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	TArray<FString> TestIssues;
	bool bAllPassed = true;

	OutIssues.Add(TEXT("=== PGX Audio System Test Suite ==="));

	// EN: Test 1: System Health / ES: Test 1: Salud del sistema
	TestIssues.Empty();
	if (!ValidateSystemHealth(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] System Health"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] System Health"));
	}

	// EN: Test 2: Channel Configs / ES: Test 2: Configs de canal
	TestIssues.Empty();
	if (!ValidateChannelConfigs(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] Channel Configs"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Channel Configs"));
	}

	// EN: Test 3: Sound Definitions / ES: Test 3: Definiciones de sonido
	TestIssues.Empty();
	if (!ValidateSoundDefinitions(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] Sound Definitions"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Sound Definitions"));
	}

	// EN: Test 4: Music Playlists / ES: Test 4: Playlists de musica
	TestIssues.Empty();
	if (!ValidateMusicPlaylists(WorldContextObject, TestIssues))
	{
		bAllPassed = false;
		OutIssues.Add(TEXT("[FAIL] Music Playlists"));
		OutIssues.Append(TestIssues);
	}
	else
	{
		OutIssues.Add(TEXT("[PASS] Music Playlists"));
	}

	OutIssues.Add(FString::Printf(TEXT("=== Result: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED")));

	PGX_LOG_INFO(LogPGXAudio, TEXT("RunAllTests — %s"), bAllPassed ? TEXT("ALL PASSED") : TEXT("SOME FAILED"));
	return bAllPassed;
}
