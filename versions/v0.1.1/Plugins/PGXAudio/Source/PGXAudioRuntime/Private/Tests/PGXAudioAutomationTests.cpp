// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#if WITH_DEV_AUTOMATION_TESTS

#include "PGXAudioConfig.h"
#include "PGXAudioSubsystem.h"
#include "Backend/PGXAudioBackendModulation.h"
#include "Data/PGXAudioChannelConfig.h"
#include "Data/PGXAudioDuckingConfig.h"
#include "Data/PGXAudioProfile.h"
#include "Data/PGXLevelAudioConfig.h"
#include "Data/PGXMusicPlaylist.h"
#include "Data/PGXSoundDefinition.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Tags/PGXAudioTags.h"
#include "Observability/PGXObservable.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PGXAudioAutomation
{
#define PGX_AUDIO_AUTOMATION_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

	struct FScopedGameInstanceFixture
	{
		explicit FScopedGameInstanceFixture(FAutomationTestBase& InTest)
			: Test(InTest)
		{
			if (!GEngine)
			{
				Test.AddError(TEXT("PGXAudio automation setup failed: engine is unavailable."));
				return;
			}

			GameInstance = NewObject<UGameInstance>(
				GEngine,
				UGameInstance::StaticClass(),
				NAME_None,
				RF_Transient);
			if (!GameInstance)
			{
				Test.AddError(TEXT("PGXAudio automation setup failed: could not create transient GameInstance."));
				return;
			}

			GameInstance->AddToRoot();
			GameInstance->InitializeStandalone(TEXT("PGXAudioAutomationWorld"));
		}

		~FScopedGameInstanceFixture()
		{
			if (!GameInstance)
			{
				return;
			}

			GameInstance->Shutdown();
			if (GameInstance->IsRooted())
			{
				GameInstance->RemoveFromRoot();
			}
		}

		FScopedGameInstanceFixture(const FScopedGameInstanceFixture&) = delete;
		FScopedGameInstanceFixture& operator=(const FScopedGameInstanceFixture&) = delete;

		UGameInstance* Get() const { return GameInstance; }

	private:
		FAutomationTestBase& Test;
		UGameInstance* GameInstance = nullptr;
	};

	UPGXAudioSubsystem* FindAudio(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		if (!GameInstance)
		{
			Test.AddError(TEXT("PGXAudio automation setup failed: no transient GameInstance available."));
			return nullptr;
		}

		UPGXAudioSubsystem* Audio = GameInstance->GetSubsystem<UPGXAudioSubsystem>();
		if (!Audio)
		{
			Test.AddError(TEXT("PGXAudio automation setup failed: UPGXAudioSubsystem missing."));
		}
		return Audio;
	}

	UPGXAudioSubsystem* MakeStandaloneAudio(FAutomationTestBase& Test, UGameInstance* GameInstance)
	{
		return FindAudio(Test, GameInstance);
	}

	UPGXAudioConfig* MakeConfig(const TCHAR* Name, int32 MaxEventHistorySize = 8)
	{
		UPGXAudioConfig* Config = NewObject<UPGXAudioConfig>(
			GetTransientPackage(),
			UPGXAudioConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		Config->BackendType = EPGXAudioBackendType::Legacy;
		Config->MaxEventHistorySize = MaxEventHistorySize;
		Config->bAllowConsoleMutations = false;
		Config->bRecordEventHistoryInShipping = false;
		Config->bExposeEventHistoryInShipping = false;
		return Config;
	}

	UPGXAudioChannelConfig* MakeChannelConfig(const TCHAR* Name, const FGameplayTag& ChannelTag, float DefaultVolume = 1.0f)
	{
		UPGXAudioChannelConfig* Config = NewObject<UPGXAudioChannelConfig>(
			GetTransientPackage(),
			UPGXAudioChannelConfig::StaticClass(),
			FName(Name),
			RF_Transient);
		Config->ChannelTag = ChannelTag;
		Config->ChannelDisplayName = FText::FromName(FName(Name));
		Config->DefaultVolume = DefaultVolume;
		return Config;
	}

	UPGXSoundDefinition* MakeSoundDefinition(const TCHAR* Name, const FGameplayTag& SoundTag)
	{
		UPGXSoundDefinition* Definition = NewObject<UPGXSoundDefinition>(
			GetTransientPackage(),
			UPGXSoundDefinition::StaticClass(),
			FName(Name),
			RF_Transient);
		Definition->SoundTag = SoundTag;
		Definition->DefaultChannelTag = TAG_PGX_Audio_Channel_SFX.GetTag();
		Definition->DefaultProfileTag = TAG_PGX_Audio_Profile_Default.GetTag();
		return Definition;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_BackendSwitchSuccessAutomationTest,
	"PGX.Audio.PhaseB.BackendSwitchSuccess", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_BackendSwitchSuccessAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::FindAudio(*this, Fixture.Get());
	if (!Audio || !Audio->GetActiveBackend())
	{
		AddError(TEXT("PGXAudio backend switch success setup failed: active backend missing."));
		return true;
	}

	const FGameplayTag ChannelTag = TAG_PGX_Audio_Channel_Master.GetTag();
	UPGXAudioChannelConfig* ChannelConfig = PGXAudioAutomation::MakeChannelConfig(TEXT("PGXAudio_Automation_Master"), ChannelTag, 1.0f);
	Audio->InjectTestChannelConfig(ChannelConfig);
	Audio->SetChannelVolume(ChannelTag, 0.42f);
	Audio->SetChannelMuted(ChannelTag, true);

	const FPGXAudioBackendSwitchResult Result = Audio->SwitchBackendDetailed(EPGXAudioBackendType::Auto);
	TestTrue(TEXT("BackendSwitchSuccess status"), Result.Status == EPGXAudioBackendSwitchStatus::Success);
	TestTrue(TEXT("BackendSwitchSuccess bSuccess"), Result.bSuccess);
	TestTrue(TEXT("BackendSwitchSuccess restored at least injected channel"), Result.RestoredChannelCount >= 1);
	TestTrue(TEXT("BackendSwitchSuccess preserves channel volume"), FMath::IsNearlyEqual(Audio->GetChannelVolume(ChannelTag), 0.42f));
	TestTrue(TEXT("BackendSwitchSuccess preserves channel mute"), Audio->IsChannelMuted(ChannelTag));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_BackendSwitchAlreadyActiveAutomationTest,
	"PGX.Audio.PhaseB.BackendSwitchAlreadyActive", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_BackendSwitchAlreadyActiveAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::FindAudio(*this, Fixture.Get());
	if (!Audio || !Audio->GetActiveBackend())
	{
		AddError(TEXT("PGXAudio backend switch already-active setup failed: active backend missing."));
		return true;
	}

	const EPGXAudioBackendType ActiveType = Audio->GetActiveBackendType();
	const FPGXAudioBackendSwitchResult Result = Audio->SwitchBackendDetailed(ActiveType);
	TestTrue(TEXT("BackendSwitchAlreadyActive status"), Result.Status == EPGXAudioBackendSwitchStatus::AlreadyActive);
	TestTrue(TEXT("BackendSwitchAlreadyActive bSuccess"), Result.bSuccess);
	TestTrue(TEXT("BackendSwitchAlreadyActive active type unchanged"), Result.ActiveBackend == ActiveType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_BackendSwitchTargetMismatchAutomationTest,
	"PGX.Audio.PhaseB.BackendSwitchTargetMismatch", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_BackendSwitchTargetMismatchAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		AddError(TEXT("PGXAudio target-mismatch setup failed: subsystem allocation failed."));
		return true;
	}

	UPGXAudioBackendModulation* SourceBackend = NewObject<UPGXAudioBackendModulation>(Audio, UPGXAudioBackendModulation::StaticClass(), NAME_None, RF_Transient);
	Audio->SetActiveBackendForTesting(SourceBackend);
	Audio->ForceNextSwitchActiveBackendTypeForTesting(EPGXAudioBackendType::Modulation);

	const FPGXAudioBackendSwitchResult Result = Audio->SwitchBackendDetailed(EPGXAudioBackendType::Legacy);
	TestTrue(TEXT("BackendSwitchTargetMismatch status"), Result.Status == EPGXAudioBackendSwitchStatus::Failed_TargetMismatch);
	TestFalse(TEXT("BackendSwitchTargetMismatch bSuccess"), Result.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_ConsoleMutationGatedAutomationTest,
	"PGX.Audio.PhaseB.ConsoleMutationGated", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_ConsoleMutationGatedAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	UPGXAudioConfig* Config = PGXAudioAutomation::MakeConfig(TEXT("PGXAudio_Automation_ConsolePolicy"));
	Audio->InjectTestAudioConfig(Config);

	Config->bAllowConsoleMutations = false;
	TestFalse(TEXT("Console mutations default gated"), Audio->AreConsoleMutationsAllowedForTesting());

#if UE_BUILD_SHIPPING
	Config->bAllowConsoleMutations = true;
	TestFalse(TEXT("Console mutations always blocked in Shipping"), Audio->AreConsoleMutationsAllowedForTesting());
#else
	Config->bAllowConsoleMutations = true;
	TestTrue(TEXT("Console mutations can be explicitly enabled outside Shipping"), Audio->AreConsoleMutationsAllowedForTesting());
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_EventHistoryShippingPolicyAutomationTest,
	"PGX.Audio.PhaseB.EventHistoryShippingPolicy", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_EventHistoryShippingPolicyAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	UPGXAudioConfig* Config = PGXAudioAutomation::MakeConfig(TEXT("PGXAudio_Automation_ShippingPolicy"));
	Audio->InjectTestAudioConfig(Config);
	Audio->ClearEventHistoryForTesting();
	Audio->RecordEventForTesting(TAG_PGX_Audio_Event_Play.GetTag(), TEXT("ShippingPolicyProbe"));

#if UE_BUILD_SHIPPING
	TestFalse(TEXT("Shipping record policy defaults off"), Audio->ShouldRecordEventHistoryForTesting());
	TestFalse(TEXT("Shipping expose policy defaults off"), Audio->ShouldExposeEventHistoryForTesting());
	TestEqual(TEXT("Shipping history hidden by default"), Audio->GetEventHistory(8).Num(), 0);
#else
	TestTrue(TEXT("Non-shipping record policy keeps diagnostics active"), Audio->ShouldRecordEventHistoryForTesting());
	TestTrue(TEXT("Non-shipping expose policy keeps diagnostics visible"), Audio->ShouldExposeEventHistoryForTesting());
	TestEqual(TEXT("Non-shipping history records diagnostic probe"), Audio->GetEventHistory(8).Num(), 1);
	AddInfo(TEXT("Shipping branch is compile-gated: defaults are asserted when this automation is compiled for Shipping."));
#endif
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_EventHistoryBoundedAutomationTest,
	"PGX.Audio.PhaseB.EventHistoryBounded", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_EventHistoryBoundedAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	UPGXAudioConfig* Config = PGXAudioAutomation::MakeConfig(TEXT("PGXAudio_Automation_HistoryBounded"), 3);
	Audio->InjectTestAudioConfig(Config);
	Audio->ClearEventHistoryForTesting();

	for (int32 Index = 0; Index < 5; ++Index)
	{
		Audio->RecordEventForTesting(TAG_PGX_Audio_Event_Play.GetTag(), FString::Printf(TEXT("History_%d"), Index));
	}

	const TArray<FPGXAudioEventRecord> History = Audio->GetEventHistory(10);
	TestEqual(TEXT("EventHistoryBounded keeps configured max"), History.Num(), 3);
	if (History.Num() == 3)
	{
		TestEqual(TEXT("EventHistoryBounded drops oldest entries"), History[0].SoundName, FString(TEXT("History_2")));
		TestEqual(TEXT("EventHistoryBounded keeps newest entry"), History[2].SoundName, FString(TEXT("History_4")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_CachedSoundDefinitionLookupAutomationTest,
	"PGX.Audio.PhaseB.CachedSoundDefinitionLookup", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_CachedSoundDefinitionLookupAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	const FGameplayTag SoundTag = TAG_PGX_Audio_Event_Play.GetTag();
	UPGXSoundDefinition* First = PGXAudioAutomation::MakeSoundDefinition(TEXT("PGXAudio_Automation_Sound_First"), SoundTag);
	UPGXSoundDefinition* Duplicate = PGXAudioAutomation::MakeSoundDefinition(TEXT("PGXAudio_Automation_Sound_Duplicate"), SoundTag);

	Audio->InjectTestSoundDefinition(First);
	Audio->InjectTestSoundDefinition(Duplicate);

	TestTrue(TEXT("CachedSoundDefinitionLookup returns first injected definition"), Audio->FindDefinitionByTag(SoundTag) == First);
	Audio->ClearTestSoundDefinitions();
	TestTrue(TEXT("CachedSoundDefinitionLookup clear removes transient definitions"), Audio->FindDefinitionByTag(SoundTag) == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_NoSilentFailureOnInvalidVolumeAutomationTest,
	"PGX.Audio.PhaseB.NoSilentFailureOnInvalidVolume", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_NoSilentFailureOnInvalidVolumeAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	UPGXAudioConfig* Config = PGXAudioAutomation::MakeConfig(TEXT("PGXAudio_Automation_InvalidVolume"));
	Audio->InjectTestAudioConfig(Config);
	Audio->ClearEventHistoryForTesting();

	Audio->SetChannelVolume(FGameplayTag::EmptyTag, 0.5f);
	const TArray<FPGXAudioEventRecord> History = Audio->GetEventHistory(8);
	TestEqual(TEXT("InvalidVolume records failed event"), History.Num(), 1);
	if (History.Num() == 1)
	{
		TestFalse(TEXT("InvalidVolume event marks failure"), History[0].bSuccess);
		TestTrue(TEXT("InvalidVolume event uses volume tag"), History[0].EventTag == TAG_PGX_Audio_Event_VolumeChanged.GetTag());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPGXAudio_PlaySoundReturnsInvalidHandleOnFailureAutomationTest,
	"PGX.Audio.PhaseB.PlaySoundReturnsInvalidHandleOnFailure", PGX_AUDIO_AUTOMATION_FLAGS)
bool FPGXAudio_PlaySoundReturnsInvalidHandleOnFailureAutomationTest::RunTest(const FString& Parameters)
{
	PGXAudioAutomation::FScopedGameInstanceFixture Fixture(*this);
	UPGXAudioSubsystem* Audio = PGXAudioAutomation::MakeStandaloneAudio(*this, Fixture.Get());
	if (!Audio)
	{
		return true;
	}
	UPGXAudioConfig* Config = PGXAudioAutomation::MakeConfig(TEXT("PGXAudio_Automation_PlayFailure"));
	Audio->InjectTestAudioConfig(Config);
	Audio->ClearEventHistoryForTesting();

	FPGXAudioPlayParams Params;
	Params.ChannelTag = TAG_PGX_Audio_Channel_SFX.GetTag();
	const FPGXSoundHandle Handle = Audio->PlaySound2D(nullptr, Params);
	TestFalse(TEXT("PlaySoundReturnsInvalidHandleOnFailure"), Handle.IsValid());

	const TArray<FPGXAudioEventRecord> History = Audio->GetEventHistory(8);
	TestEqual(TEXT("PlaySound failure records event"), History.Num(), 1);
	if (History.Num() == 1)
	{
		TestFalse(TEXT("PlaySound failure event marks failure"), History[0].bSuccess);
		TestTrue(TEXT("PlaySound failure event uses play tag"), History[0].EventTag == TAG_PGX_Audio_Event_Play.GetTag());
	}
	return true;
}

// ============================================================================
// EN: IPGXObservable adoption schema validation tests
//     for all 7 PGXAudio authoring DA classes. Mirror PGXUI 8.3.C shared
//     template helper precedent (DRY pattern saves ~LOC across multi-class
//     plugins).
// ES: Tests de validacion del schema de adopcion
//     IPGXObservable de las 7 clases authoring DA PGXAudio. Mirror
//     precedent de helper template compartido PGXUI 8.3.C.
// ============================================================================

namespace PGXAudioObservabilityAutomation
{
	template<typename TConfigClass>
	bool ValidateObservableContract(FAutomationTestBase& Test, const TCHAR* ExpectedTypeName)
	{
		TConfigClass* Instance = NewObject<TConfigClass>(
			GetTransientPackage(), TConfigClass::StaticClass(), NAME_None, RF_Transient);
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s instance"), ExpectedTypeName), Instance))
		{
			return false;
		}

		const FName SchemaVersion = Instance->GetSchemaVersion();
		Test.TestEqual(*FString::Printf(TEXT("%s::GetSchemaVersion is 1.0"), ExpectedTypeName),
			SchemaVersion, FName(TEXT("1.0")));

		const FPGXSchemaDescriptor Descriptor = Instance->GetSchemaDescriptor();
		Test.TestEqual(*FString::Printf(TEXT("%s schema TypeName matches"), ExpectedTypeName),
			Descriptor.TypeName, TConfigClass::StaticClass()->GetFName());
		Test.TestEqual(*FString::Printf(TEXT("%s schema SchemaVersion matches"), ExpectedTypeName),
			Descriptor.SchemaVersion, SchemaVersion);
		Test.TestTrue(*FString::Printf(TEXT("%s schema Fields > 0"), ExpectedTypeName),
			Descriptor.Fields.Num() > 0);

		const FPGXJsonValue Envelope = Instance->ToJson();
		Test.TestFalse(*FString::Printf(TEXT("%s ToJson envelope non-empty"), ExpectedTypeName),
			Envelope.IsEmpty());
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains type"), ExpectedTypeName),
			Envelope.JsonString.Contains(FString::Printf(TEXT("\"type\":\"%s\""), ExpectedTypeName)));
		Test.TestTrue(*FString::Printf(TEXT("%s envelope contains 1.0 version"), ExpectedTypeName),
			Envelope.JsonString.Contains(TEXT("\"version\":\"1.0\"")));

		const FPGXJsonValue EmptyJson;
		const FPGXValidationResult EmptyResult = Instance->FromJson(EmptyJson);
		Test.TestFalse(*FString::Printf(TEXT("%s FromJson rejects empty"), ExpectedTypeName),
			EmptyResult.bValid);

		const FPGXValidationResult OkResult = Instance->FromJson(Envelope);
		Test.TestTrue(*FString::Printf(TEXT("%s FromJson accepts envelope"), ExpectedTypeName),
			OkResult.bValid);

		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXAudioConfigObservableSchema,
	"PGX.Audio.ConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXAudioConfigObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXAudioConfig>(*this, TEXT("PGXAudioConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXAudioChannelConfigObservableSchema,
	"PGX.Audio.ChannelConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXAudioChannelConfigObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXAudioChannelConfig>(*this, TEXT("PGXAudioChannelConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXAudioDuckingConfigObservableSchema,
	"PGX.Audio.DuckingConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXAudioDuckingConfigObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXAudioDuckingConfig>(*this, TEXT("PGXAudioDuckingConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXAudioProfileObservableSchema,
	"PGX.Audio.ProfileObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXAudioProfileObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXAudioProfile>(*this, TEXT("PGXAudioProfile"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXLevelAudioConfigObservableSchema,
	"PGX.Audio.LevelAudioConfigObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXLevelAudioConfigObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXLevelAudioConfig>(*this, TEXT("PGXLevelAudioConfig"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXMusicPlaylistObservableSchema,
	"PGX.Audio.MusicPlaylistObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXMusicPlaylistObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXMusicPlaylist>(*this, TEXT("PGXMusicPlaylist"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPGXSoundDefinitionObservableSchema,
	"PGX.Audio.SoundDefinitionObservableSchema",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FPGXSoundDefinitionObservableSchema::RunTest(const FString& /*P*/)
{
	return PGXAudioObservabilityAutomation::ValidateObservableContract<UPGXSoundDefinition>(*this, TEXT("PGXSoundDefinition"));
}

#endif // WITH_DEV_AUTOMATION_TESTS
