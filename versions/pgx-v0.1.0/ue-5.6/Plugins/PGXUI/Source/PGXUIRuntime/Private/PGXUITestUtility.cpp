// Copyright PGX Framework. All Rights Reserved.

#include "PGXUITestUtility.h"
#include "PGXNotificationProfile.h"
#include "PGXScreenDefinition.h"
#include "PGXUISettings.h"
#include "PGXWidgetPoolProfile.h"
#include "Tags/PGXUITags.h"
#include "Logging/PGXLogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogPGXUITest, Log, All);

void UPGXUITestUtility::RecordResult(TArray<FString>& OutIssues, const FString& TestName, bool bPassed, const FString& Details)
{
	const FString Suffix = Details.IsEmpty() ? FString() : FString::Printf(TEXT(" (%s)"), *Details);
	const FString Tag = bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
	OutIssues.Add(FString::Printf(TEXT("%s %s%s"), *Tag, *TestName, *Suffix));
	if (bPassed)
	{
		PGX_LOG_INFO(LogPGXUITest, TEXT("[PGX UI Test] PASS: %s%s"), *TestName, *Suffix);
	}
	else
	{
		PGX_LOG_ERROR(LogPGXUITest, TEXT("[PGX UI Test] FAIL: %s%s"), *TestName, *Suffix);
	}
}

// ============================================================================
// 1. ValidateScreenDefinition — DataAsset UPGXScreenDefinition Object DA
// ============================================================================

bool UPGXUITestUtility::ValidateScreenDefinition(const UPGXScreenDefinition* Definition, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	if (!Definition)
	{
		RecordResult(OutIssues, TEXT("ScreenDefinition.NotNull"), false, TEXT("Definition pointer is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("ScreenDefinition.NotNull"), true);

	// Validation rule: ScreenTag resolves under PGX.UI.Screen.Type.*
	const bool bScreenTagValid = Definition->ScreenTag.IsValid()
		&& Definition->ScreenTag.MatchesTag(TAG_PGX_UI_Screen_Type.GetTag());
	AssertPass(TEXT("ScreenDefinition.ScreenTagInTypeBranch"), bScreenTagValid,
		Definition->ScreenTag.IsValid() ? Definition->ScreenTag.ToString() : TEXT("invalid tag"));

	// Validation rule: LayerTag resolves under PGX.UI.Screen.Layer.*
	const bool bLayerTagValid = Definition->LayerTag.IsValid()
		&& Definition->LayerTag.MatchesTag(TAG_PGX_UI_Screen_Layer.GetTag());
	AssertPass(TEXT("ScreenDefinition.LayerTagInLayerBranch"), bLayerTagValid,
		Definition->LayerTag.IsValid() ? Definition->LayerTag.ToString() : TEXT("invalid tag"));

	// Validation rule: WidgetClass soft reference path non-null
	const bool bWidgetClassValid = !Definition->WidgetClass.IsNull();
	AssertPass(TEXT("ScreenDefinition.WidgetClassNonNull"), bWidgetClassValid,
		bWidgetClassValid ? Definition->WidgetClass.ToString() : TEXT("soft class pointer is null"));

	// Defensive: LayerOrder within authored bounds
	const bool bLayerOrderInRange = Definition->LayerOrder >= 0 && Definition->LayerOrder <= 100;
	AssertPass(TEXT("ScreenDefinition.LayerOrderInRange"), bLayerOrderInRange,
		FString::Printf(TEXT("LayerOrder=%d, expected 0..100"), Definition->LayerOrder));

	return bAllPassed;
}

// ============================================================================
// 2. ValidateNotificationProfile — DataAsset UPGXNotificationProfile Object DA
// ============================================================================

bool UPGXUITestUtility::ValidateNotificationProfile(const UPGXNotificationProfile* Profile, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	if (!Profile)
	{
		RecordResult(OutIssues, TEXT("NotificationProfile.NotNull"), false, TEXT("Profile pointer is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("NotificationProfile.NotNull"), true);

	// Validation rule: CategoryTag resolves under PGX.UI.Notification.Category.*
	const bool bCategoryTagValid = Profile->CategoryTag.IsValid()
		&& Profile->CategoryTag.MatchesTag(TAG_PGX_UI_Notification_Category.GetTag());
	AssertPass(TEXT("NotificationProfile.CategoryTagInCategoryBranch"), bCategoryTagValid,
		Profile->CategoryTag.IsValid() ? Profile->CategoryTag.ToString() : TEXT("invalid tag"));

	// Validation rule: PriorityTag resolves under PGX.UI.Notification.Priority.*
	const bool bPriorityTagValid = Profile->PriorityTag.IsValid()
		&& Profile->PriorityTag.MatchesTag(TAG_PGX_UI_Notification_Priority.GetTag());
	AssertPass(TEXT("NotificationProfile.PriorityTagInPriorityBranch"), bPriorityTagValid,
		Profile->PriorityTag.IsValid() ? Profile->PriorityTag.ToString() : TEXT("invalid tag"));

	// Validation rule: coalescing policy explicit (MaxQueueDepth coherent — bAllowCoalescing=false acceptable, but MaxQueueDepth must be in clamp range)
	const bool bCoalescingExplicit = Profile->MaxQueueDepth >= 0 && Profile->MaxQueueDepth <= 32;
	AssertPass(TEXT("NotificationProfile.CoalescingPolicyExplicit"), bCoalescingExplicit,
		FString::Printf(TEXT("MaxQueueDepth=%d, expected 0..32"), Profile->MaxQueueDepth));

	// Validation rule: dismissal policy explicit (DefaultDisplayTimeSeconds non-negative)
	const bool bDismissalExplicit = Profile->DefaultDisplayTimeSeconds >= 0.0f && Profile->DefaultDisplayTimeSeconds <= 60.0f;
	AssertPass(TEXT("NotificationProfile.DismissalPolicyExplicit"), bDismissalExplicit,
		FString::Printf(TEXT("DefaultDisplayTimeSeconds=%.2f, expected 0..60"), Profile->DefaultDisplayTimeSeconds));

	// Defensive: PriorityNumeric within authored bounds
	const bool bPriorityInRange = Profile->PriorityNumeric >= 0 && Profile->PriorityNumeric <= 100;
	AssertPass(TEXT("NotificationProfile.PriorityNumericInRange"), bPriorityInRange,
		FString::Printf(TEXT("PriorityNumeric=%d, expected 0..100"), Profile->PriorityNumeric));

	return bAllPassed;
}

// ============================================================================
// 3. ValidateWidgetPoolProfile — DataAsset UPGXWidgetPoolProfile Object DA
// ============================================================================

bool UPGXUITestUtility::ValidateWidgetPoolProfile(const UPGXWidgetPoolProfile* Profile, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	if (!Profile)
	{
		RecordResult(OutIssues, TEXT("WidgetPoolProfile.NotNull"), false, TEXT("Profile pointer is null"));
		return false;
	}

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	AssertPass(TEXT("WidgetPoolProfile.NotNull"), true);

	// Validation rule: PoolTypeTag resolves under PGX.UI.WidgetPool.Type.*
	const bool bPoolTypeTagValid = Profile->PoolTypeTag.IsValid()
		&& Profile->PoolTypeTag.MatchesTag(TAG_PGX_UI_WidgetPool_Type.GetTag());
	AssertPass(TEXT("WidgetPoolProfile.PoolTypeTagInTypeBranch"), bPoolTypeTagValid,
		Profile->PoolTypeTag.IsValid() ? Profile->PoolTypeTag.ToString() : TEXT("invalid tag"));

	// Validation rule: widget class policy present (WidgetClass non-null OR bIsAbstractPool)
	const bool bWidgetClassPolicyPresent = !Profile->WidgetClass.IsNull() || Profile->bIsAbstractPool;
	AssertPass(TEXT("WidgetPoolProfile.WidgetClassPolicyPresent"), bWidgetClassPolicyPresent,
		Profile->bIsAbstractPool ? TEXT("abstract pool") : Profile->WidgetClass.ToString());

	// Validation rule: capacity coherent (InitialCapacity <= MaxCapacity, MaxCapacity >= 1)
	const bool bCapacityCoherent = Profile->InitialCapacity <= Profile->MaxCapacity && Profile->MaxCapacity >= 1;
	AssertPass(TEXT("WidgetPoolProfile.CapacityCoherent"), bCapacityCoherent,
		FString::Printf(TEXT("Initial=%d, Max=%d"), Profile->InitialCapacity, Profile->MaxCapacity));

	// Validation rule: reset validator policy explicit (MaxReuseCount in range; bResetOnRelease is bool — always present)
	const bool bResetPolicyExplicit = Profile->MaxReuseCount >= 0 && Profile->MaxReuseCount <= 10000;
	AssertPass(TEXT("WidgetPoolProfile.ResetPolicyExplicit"), bResetPolicyExplicit,
		FString::Printf(TEXT("MaxReuseCount=%d, expected 0..10000"), Profile->MaxReuseCount));

	return bAllPassed;
}

// ============================================================================
// 4. ValidateTagInNamespace — tag generic tag namespace guard
// ============================================================================

bool UPGXUITestUtility::ValidateTagInNamespace(FGameplayTag Tag, FGameplayTag NamespaceRoot, TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const FString TagStr = Tag.IsValid() ? Tag.ToString() : TEXT("<invalid>");
	const FString RootStr = NamespaceRoot.IsValid() ? NamespaceRoot.ToString() : TEXT("<invalid>");

	const bool bTagValid = Tag.IsValid();
	AssertPass(TEXT("TagInNamespace.TagValid"), bTagValid, TagStr);

	const bool bRootValid = NamespaceRoot.IsValid();
	AssertPass(TEXT("TagInNamespace.NamespaceRootValid"), bRootValid, RootStr);

	if (bTagValid && bRootValid)
	{
		const bool bMatches = Tag.MatchesTag(NamespaceRoot);
		AssertPass(TEXT("TagInNamespace.TagMatchesRoot"), bMatches,
			FString::Printf(TEXT("%s in %s"), *TagStr, *RootStr));
	}

	return bAllPassed;
}

// ============================================================================
// 5. ValidateSettingsAccessor — settings UPGXUISettings canonical surface
// ============================================================================

bool UPGXUITestUtility::ValidateSettingsAccessor(TArray<FString>& OutIssues)
{
	OutIssues.Empty();

	bool bAllPassed = true;
	auto AssertPass = [&](const FString& Name, bool bPassed, const FString& Details = TEXT(""))
	{
		RecordResult(OutIssues, Name, bPassed, Details);
		if (!bPassed) bAllPassed = false;
	};

	const UPGXUISettings* Settings = GetDefault<UPGXUISettings>();
	AssertPass(TEXT("SettingsAccessor.GetDefaultNonNull"), Settings != nullptr);
	if (!Settings)
	{
		return false;
	}

	// Default DiscoveryMode is AssetRegistryScan (deprecated fallback shape)
	AssertPass(TEXT("SettingsAccessor.DiscoveryModeDefault"),
		Settings->DiscoveryMode == EPGXUIDiscoveryMode::AssetRegistryScan,
		FString::Printf(TEXT("mode=%d"), static_cast<int32>(Settings->DiscoveryMode)));

	// ActiveConfig accessor reachable; soft pointer can be null/empty without error (NOT CONSUMED AT RUNTIME)
	const TSoftObjectPtr<UPGXUIConfig>& ActiveConfig = Settings->ActiveConfig;
	AssertPass(TEXT("SettingsAccessor.ActiveConfigAccessor"), true,
		FString::Printf(TEXT("IsNull=%d"), ActiveConfig.IsNull() ? 1 : 0));

	// Verbose flag default false (production-safe)
	AssertPass(TEXT("SettingsAccessor.VerboseDefaultFalse"), Settings->bVerboseConfigResolution == false);

	// GetCategoryName must return "PGX" for project-settings grouping consistency
	AssertPass(TEXT("SettingsAccessor.CategoryName"),
		Settings->GetCategoryName() == TEXT("PGX"),
		Settings->GetCategoryName().ToString());

	return bAllPassed;
}
