// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Messages/PGXMessageTestUtility.h"
#include "Messages/PGXMessageSubsystem.h"
#include "Messages/PGXMessageConfig.h"
#include "Messages/PGXMessageSettings.h"
#include "Messages/PGXMessageLog.h"
#include "Messages/PGXBridgeMessages.h"
#include "Messages/Tags/PGXBridgeTags.h"
#include "Messages/Tags/PGXMessageTags.h"

// EN: PGX Message System test utility implementation
// ES: Implementacion de utilidad de test del Sistema de Mensajes PGX

bool UPGXMessageTestUtility::QuickTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	const FPGXMessageStats Stats = MsgSub->GetStats();
	OutIssues.Add(FString::Printf(TEXT("OK: MessageSubsystem active. Channels=%d Listeners=%d Broadcasts=%d"),
		Stats.ActiveChannels, Stats.ActiveListeners, Stats.TotalBroadcasts));

	return true;
}

bool UPGXMessageTestUtility::BroadcastReceiveTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	bool bReceived = false;
	const FGameplayTag TestChannel = TAG_PGX_Message_System.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tag PGX.Message.System not registered (native tag)"));
		return false;
	}

	// EN: Register listener / ES: Registrar listener
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&bReceived](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/)
		{
			bReceived = true;
		});

	if (!Handle.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Could not register listener"));
		return false;
	}

	// EN: Broadcast test message / ES: Emitir mensaje de prueba
	FPGXMessage TestMsg;
	TestMsg.MessageTag = TestChannel;
	TestMsg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, TestMsg);

	Handle.Unregister();

	if (!bReceived)
	{
		OutIssues.Add(TEXT("FAIL: Message was not received by listener"));
		return false;
	}

	OutIssues.Add(TEXT("OK: Broadcast-Receive pipeline working"));
	return true;
}

bool UPGXMessageTestUtility::FilterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	int32 ReceivedCount = 0;
	const FGameplayTag ChannelA = TAG_PGX_Message_System.GetTag();
	const FGameplayTag ChannelB = TAG_PGX_Message_Gameplay.GetTag();

	if (!ChannelA.IsValid() || !ChannelB.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tags not registered (native tags)"));
		return false;
	}

	// EN: Listen on channel A only / ES: Escuchar solo en canal A
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(ChannelA,
		[&ReceivedCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { ReceivedCount++; });

	FPGXMessage TestMsg;
	TestMsg.Timestamp = FPlatformTime::Seconds();

	// EN: Broadcast on A (should receive) / ES: Broadcast en A (deberia recibir)
	TestMsg.MessageTag = ChannelA;
	MsgSub->BroadcastMessage<FPGXMessage>(ChannelA, TestMsg);

	// EN: Broadcast on B (should NOT receive) / ES: Broadcast en B (NO deberia recibir)
	TestMsg.MessageTag = ChannelB;
	MsgSub->BroadcastMessage<FPGXMessage>(ChannelB, TestMsg);

	Handle.Unregister();

	if (ReceivedCount != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Expected 1 receive, got %d"), ReceivedCount));
		return false;
	}

	OutIssues.Add(TEXT("OK: Channel filtering works correctly"));
	return true;
}

bool UPGXMessageTestUtility::MatchTypeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	int32 ExactCount = 0;
	int32 PartialCount = 0;
	const FGameplayTag ParentTag = TAG_PGX_Message.GetTag();
	const FGameplayTag ChildTag = TAG_PGX_Message_System.GetTag();

	if (!ParentTag.IsValid() || !ChildTag.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tags not registered (native tags)"));
		return false;
	}

	// EN: Register exact listener on parent / ES: Registrar listener exacto en padre
	FPGXMessageListenerHandle ExactHandle = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&ExactCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { ExactCount++; },
		EPGXMessageMatch::ExactMatch);

	// EN: Register partial listener on parent / ES: Registrar listener parcial en padre
	FPGXMessageListenerHandle PartialHandle = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&PartialCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { PartialCount++; },
		EPGXMessageMatch::PartialMatch);

	// EN: Broadcast on child tag / ES: Broadcast en tag hijo
	FPGXMessage TestMsg;
	TestMsg.MessageTag = ChildTag;
	TestMsg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(ChildTag, TestMsg);

	ExactHandle.Unregister();
	PartialHandle.Unregister();

	bool bSuccess = true;
	if (ExactCount != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Exact listener should not fire for child. Got %d"), ExactCount));
		bSuccess = false;
	}
	if (PartialCount != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Partial listener should fire once. Got %d"), PartialCount));
		bSuccess = false;
	}

	if (bSuccess)
	{
		OutIssues.Add(TEXT("OK: Match types (exact vs partial) work correctly"));
	}
	return bSuccess;
}

bool UPGXMessageTestUtility::HistoryTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	const int32 PreBroadcasts = MsgSub->GetStats().TotalBroadcasts;

	// EN: Broadcast a test message / ES: Emitir un mensaje de prueba
	const FGameplayTag TestChannel = TAG_PGX_Message_System.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tag not registered (native tag)"));
		return false;
	}

	FPGXMessage TestMsg;
	TestMsg.MessageTag = TestChannel;
	TestMsg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, TestMsg);

	// EN: Check history / ES: Verificar historial
	const TArray<FPGXMessageRecord> History = MsgSub->GetMessageHistory(TestChannel, 5);
	if (History.Num() == 0)
	{
		OutIssues.Add(TEXT("FAIL: No history recorded after broadcast"));
		return false;
	}

	if (History[0].Channel != TestChannel)
	{
		OutIssues.Add(TEXT("FAIL: History channel mismatch"));
		return false;
	}

	const int32 PostBroadcasts = MsgSub->GetStats().TotalBroadcasts;
	if (PostBroadcasts <= PreBroadcasts)
	{
		OutIssues.Add(TEXT("FAIL: Broadcast counter did not increment"));
		return false;
	}

	OutIssues.Add(TEXT("OK: Message history recording works"));
	return true;
}

bool UPGXMessageTestUtility::UnregisterTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	int32 ReceivedCount = 0;
	const FGameplayTag TestChannel = TAG_PGX_Message_System.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tag not registered (native tag)"));
		return false;
	}

	// EN: Register and then unregister / ES: Registrar y luego desregistrar
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&ReceivedCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { ReceivedCount++; });

	const int32 ListenersAfterAdd = MsgSub->GetListenerCount(TestChannel);
	Handle.Unregister();
	const int32 ListenersAfterRemove = MsgSub->GetListenerCount(TestChannel);

	// EN: Broadcast after unregister (should NOT receive) / ES: Broadcast despues de desregistrar
	FPGXMessage TestMsg;
	TestMsg.MessageTag = TestChannel;
	TestMsg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, TestMsg);

	bool bSuccess = true;
	if (ListenersAfterAdd < 1)
	{
		OutIssues.Add(TEXT("FAIL: Listener was not registered"));
		bSuccess = false;
	}
	if (ListenersAfterRemove >= ListenersAfterAdd)
	{
		OutIssues.Add(TEXT("FAIL: Listener count did not decrease after unregister"));
		bSuccess = false;
	}
	if (ReceivedCount != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Received %d messages after unregister"), ReceivedCount));
		bSuccess = false;
	}

	if (bSuccess)
	{
		OutIssues.Add(TEXT("OK: Unregister and cleanup work correctly"));
	}
	return bSuccess;
}

bool UPGXMessageTestUtility::RunAllTests(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	bool bAllPassed = true;
	TArray<FString> TestIssues;

	auto RunTest = [&](const FString& Name, bool (*TestFunc)(const UObject*, TArray<FString>&))
	{
		TestIssues.Empty();
		const bool bPassed = TestFunc(WorldContextObject, TestIssues);
		OutIssues.Add(FString::Printf(TEXT("[%s] %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *Name));
		OutIssues.Append(TestIssues);
		if (!bPassed) bAllPassed = false;
	};

	// Baseline tests (pre-M3)
	RunTest(TEXT("QuickTest"), &QuickTest);
	RunTest(TEXT("BroadcastReceiveTest"), &BroadcastReceiveTest);
	RunTest(TEXT("FilterTest"), &FilterTest);
	RunTest(TEXT("MatchTypeTest"), &MatchTypeTest);
	RunTest(TEXT("HistoryTest"), &HistoryTest);
	RunTest(TEXT("UnregisterTest"), &UnregisterTest);

	// Compatibility tests — Listener safety: listener safety
	RunTest(TEXT("TypeMismatchRejectedTest"), &TypeMismatchRejectedTest);
	RunTest(TEXT("DoubleUnregisterSafeTest"), &DoubleUnregisterSafeTest);
	RunTest(TEXT("CallbackRemovalSafeTest"), &CallbackRemovalSafeTest);

	// Partial-matching tests — partial matching
	RunTest(TEXT("ExactParentNoChildTest"), &ExactParentNoChildTest);
	RunTest(TEXT("PartialMatchGlobalDisabledTest"), &PartialMatchGlobalDisabledTest);
	RunTest(TEXT("ChannelIsolationTest"), &ChannelIsolationTest);

	// Payload compatibility tests — payload backward compat
	RunTest(TEXT("PayloadBackwardCompatTest"), &PayloadBackwardCompatTest);

	// Configuration and history tests — config / history
	RunTest(TEXT("HistoryBoundedByPolicyTest"), &HistoryBoundedByPolicyTest);
	RunTest(TEXT("ClearHistoryAdditionalTest"), &ClearHistoryAdditionalTest);
	RunTest(TEXT("EmergencyHistoryFallbackTest"), &EmergencyHistoryFallbackTest);

	// Telemetry tests — telemetry
	RunTest(TEXT("FanOutTelemetryTest"), &FanOutTelemetryTest);
	RunTest(TEXT("NestedBroadcastOrderingTest"), &NestedBroadcastOrderingTest);
	RunTest(TEXT("TimeAccelBoundaryReadOnlyTest"), &TimeAccelBoundaryReadOnlyTest);

	OutIssues.Add(FString::Printf(TEXT("=== Message System Tests: %s ==="), bAllPassed ? TEXT("ALL PASSED") : TEXT("FAILURES")));
	return bAllPassed;
}

// ============================================================
// Compatibility tests — Listener safety (listener safety)
// ============================================================

bool UPGXMessageTestUtility::TypeMismatchRejectedTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Listener registered with FPGXMessage filter receives a broadcast of FPGXBridgeGameFlowChanged
	//     (an unrelated USTRUCT — NOT a child of FPGXMessage). BroadcastMessageInternal must log
	//     "Struct type mismatch" and NOT invoke the listener callback (the type-mismatch case).
	// ES: Listener registrado con filtro FPGXMessage recibe un broadcast de FPGXBridgeGameFlowChanged
	//     (USTRUCT no relacionado). BroadcastMessageInternal debe loggear mismatch y NO invocar callback.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: TAG_PGX_Message_Debug not registered (native tag)"));
		return false;
	}

	int32 ReceivedCount = 0;
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&ReceivedCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { ReceivedCount++; });

	// EN: Broadcast a struct that is NOT a child of FPGXMessage. Must be rejected.
	FPGXBridgeGameFlowChanged Mismatch;
	Mismatch.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, Mismatch);

	Handle.Unregister();

	if (ReceivedCount != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: type mismatch listener fired %d times (expected 0)"), ReceivedCount));
		return false;
	}
	OutIssues.Add(TEXT("OK: type mismatch broadcast rejected; callback not invoked"));
	return true;
}

bool UPGXMessageTestUtility::DoubleUnregisterSafeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Calling Handle.Unregister() twice in a row must not crash. After first Unregister(),
	//     handle is reset (Subsystem cleared, ID=0); second call short-circuits via IsValid()
	//     check and logs a warning instead of crashing (the repeated-unregister case).
	// ES: Llamar Handle.Unregister() dos veces no debe crashear. Tras el primer Unregister el handle
	//     se resetea; el segundo call cortocircuita via IsValid() y loggea warning en vez de crashear.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: TAG_PGX_Message_Debug not registered"));
		return false;
	}

	const int32 PreListenerCount = MsgSub->GetListenerCount(TestChannel);

	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) {});

	if (!Handle.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: initial RegisterListener returned invalid handle"));
		return false;
	}

	// First Unregister — handle becomes invalid.
	Handle.Unregister();
	if (Handle.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: handle still valid after first Unregister"));
		return false;
	}

	// Second Unregister — must not crash. IsValid() short-circuits internal call path.
	Handle.Unregister();

	const int32 PostListenerCount = MsgSub->GetListenerCount(TestChannel);
	if (PostListenerCount != PreListenerCount)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: listener count drift (pre=%d post=%d)"),
			PreListenerCount, PostListenerCount));
		return false;
	}

	OutIssues.Add(TEXT("OK: double unregister safe (no crash, no listener leak)"));
	return true;
}

bool UPGXMessageTestUtility::CallbackRemovalSafeTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: A listener that calls Handle.Unregister() inside its own callback must not break
	//     iteration of remaining listeners on the same channel. BroadcastMessageInternal copies
	//     ListenerArray before iterating (Subsystem.cpp:174) — removal of the live entry only
	//     mutates ListenerMap, not the local copy (the callback-removal case).
	// ES: Un listener que llama Handle.Unregister() dentro de su propio callback no debe romper
	//     la iteracion de los listeners restantes en el mismo canal.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub))
	{
		OutIssues.Add(TEXT("FAIL: MessageSubsystem not available"));
		return false;
	}

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: TAG_PGX_Message_Debug not registered"));
		return false;
	}

	int32 SelfRemovingCount = 0;
	int32 OtherCount = 0;

	// EN: Captured by ref so the callback can unregister itself.
	FPGXMessageListenerHandle SelfHandle;
	SelfHandle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&SelfRemovingCount, &SelfHandle](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/)
		{
			SelfRemovingCount++;
			SelfHandle.Unregister();
		});

	// EN: Second listener on the SAME channel — must still fire after the self-removing listener removes itself.
	FPGXMessageListenerHandle OtherHandle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&OtherCount](FGameplayTag /*Channel*/, const FPGXMessage& /*Msg*/) { OtherCount++; });

	FPGXMessage Msg;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);

	OtherHandle.Unregister();

	if (SelfRemovingCount != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: self-removing listener fired %d times (expected 1)"), SelfRemovingCount));
		return false;
	}
	if (OtherCount != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: other listener fired %d times (expected 1) — iteration was broken"), OtherCount));
		return false;
	}

	// EN: Second broadcast — self-removing listener should be gone, only OtherCount would have fired.
	//     OtherHandle is unregistered above so neither fires; verify count remains stable.
	const int32 ChannelListeners = MsgSub->GetListenerCount(TestChannel);
	if (ChannelListeners != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: listener count after teardown = %d (expected 0)"), ChannelListeners));
		return false;
	}

	OutIssues.Add(TEXT("OK: callback-time removal safe (iteration not broken; cleanup correct)"));
	return true;
}

// ============================================================
// Compatibility tests — Partial matching (partial matching)
// ============================================================

bool UPGXMessageTestUtility::ExactParentNoChildTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag ParentTag = TAG_PGX_Message.GetTag();
	const FGameplayTag ChildTag = TAG_PGX_Message_System.GetTag();
	if (!ParentTag.IsValid() || !ChildTag.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: Test tags not registered (native tags)"));
		return false;
	}

	int32 ExactParentCount = 0;
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&ExactParentCount](FGameplayTag /*C*/, const FPGXMessage& /*M*/) { ExactParentCount++; },
		EPGXMessageMatch::ExactMatch);

	FPGXMessage Msg;
	Msg.MessageTag = ChildTag;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(ChildTag, Msg);

	Handle.Unregister();

	if (ExactParentCount != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: exact parent listener received child broadcast (count=%d)"), ExactParentCount));
		return false;
	}
	OutIssues.Add(TEXT("OK: exact parent listener does not receive child broadcast"));
	return true;
}

bool UPGXMessageTestUtility::PartialMatchGlobalDisabledTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
#if WITH_EDITOR
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag ParentTag = TAG_PGX_Message.GetTag();
	const FGameplayTag ChildTag = TAG_PGX_Message_System.GetTag();
	if (!ParentTag.IsValid() || !ChildTag.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: tags not registered"));
		return false;
	}

	// EN: Inject test config with bEnablePartialMatching=false to gate global partial fan-out.
	UPGXMessageConfig* TestCfg = NewObject<UPGXMessageConfig>();
	TestCfg->bEnablePartialMatching = false;
	TestCfg->MaxMessageHistory = 50;
	MsgSub->InjectTestConfig(TestCfg);

	int32 PartialParentCount = 0;
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&PartialParentCount](FGameplayTag /*C*/, const FPGXMessage& /*M*/) { PartialParentCount++; },
		EPGXMessageMatch::PartialMatch);

	FPGXMessage Msg;
	Msg.MessageTag = ChildTag;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(ChildTag, Msg);

	Handle.Unregister();
	MsgSub->ClearTestConfigs();

	if (PartialParentCount != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: partial parent listener fired with global gate disabled (count=%d)"), PartialParentCount));
		return false;
	}
	OutIssues.Add(TEXT("OK: global bEnablePartialMatching=false silences parent fan-out"));
	return true;
#else
	OutIssues.Add(TEXT("SKIP: requires WITH_EDITOR (InjectTestConfig is editor-only)"));
	return true;
#endif
}

bool UPGXMessageTestUtility::ChannelIsolationTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag ChannelA = TAG_PGX_Message_System.GetTag();
	const FGameplayTag ChannelB = TAG_PGX_Message_Gameplay.GetTag();
	const FGameplayTag ChannelC = TAG_PGX_Message_UI.GetTag();
	if (!ChannelA.IsValid() || !ChannelB.IsValid() || !ChannelC.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: tags not registered"));
		return false;
	}

	int32 CountA = 0, CountB = 0, CountC = 0;
	FPGXMessageListenerHandle HA = MsgSub->RegisterListener<FPGXMessage>(ChannelA,
		[&CountA](FGameplayTag, const FPGXMessage&) { CountA++; });
	FPGXMessageListenerHandle HB = MsgSub->RegisterListener<FPGXMessage>(ChannelB,
		[&CountB](FGameplayTag, const FPGXMessage&) { CountB++; });
	FPGXMessageListenerHandle HC = MsgSub->RegisterListener<FPGXMessage>(ChannelC,
		[&CountC](FGameplayTag, const FPGXMessage&) { CountC++; });

	FPGXMessage Msg;
	Msg.Timestamp = FPlatformTime::Seconds();
	Msg.MessageTag = ChannelB;
	MsgSub->BroadcastMessage<FPGXMessage>(ChannelB, Msg);

	HA.Unregister(); HB.Unregister(); HC.Unregister();

	if (CountA != 0 || CountB != 1 || CountC != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: isolation broken — A=%d B=%d C=%d (expected 0/1/0)"), CountA, CountB, CountC));
		return false;
	}
	OutIssues.Add(TEXT("OK: channel isolation — only target channel receives"));
	return true;
}

// ============================================================
// Compatibility tests — Payload backward compatibility (payload backward compat)
// ============================================================

bool UPGXMessageTestUtility::PayloadBackwardCompatTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Older listener compiled with FPGXMessage (parent) filter receives broadcast of
	//     FPGXTestChildMessage (child). HandleMessageReceived/BroadcastMessageInternal use
	//     IsChildOf compatibility — listener fires and parent-layout
	//     copy from child bytes is safe per UStruct inheritance prefix invariant.
	// ES: Listener antiguo con filtro FPGXMessage recibe broadcast de FPGXTestChildMessage.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	int32 ReceivedCount = 0;
	double ReceivedTimestamp = 0.0;

	// Older listener: filter is parent FPGXMessage.
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&ReceivedCount, &ReceivedTimestamp](FGameplayTag /*Channel*/, const FPGXMessage& Msg)
		{
			ReceivedCount++;
			ReceivedTimestamp = Msg.Timestamp;
		});

	// Newer broadcast: child struct with extra field.
	FPGXTestChildMessage ChildMsg;
	ChildMsg.MessageTag = TestChannel;
	ChildMsg.Timestamp = 1234.5;
	ChildMsg.ChildOnlyValue = 42;
	MsgSub->BroadcastMessage<FPGXTestChildMessage>(TestChannel, ChildMsg);

	Handle.Unregister();

	if (ReceivedCount != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: parent-typed listener did not receive child broadcast (count=%d, expected 1)"), ReceivedCount));
		return false;
	}
	if (FMath::Abs(ReceivedTimestamp - 1234.5) > 0.001)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: parent-layout copy lost child Timestamp field (got %.3f, expected 1234.5)"), ReceivedTimestamp));
		return false;
	}
	OutIssues.Add(TEXT("OK: payload backward compat (parent listener receives child broadcast; parent fields copied from child bytes)"));
	return true;
}

// ============================================================
// Compatibility tests — Configuration and history (config / history) WITH_EDITOR
// ============================================================

bool UPGXMessageTestUtility::HistoryBoundedByPolicyTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
#if WITH_EDITOR
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	UPGXMessageConfig* TestCfg = NewObject<UPGXMessageConfig>();
	TestCfg->MaxMessageHistory = 16;  // ClampMin per spec
	MsgSub->InjectTestConfig(TestCfg);

	// Snapshot existing history; compute pre-broadcast size (may have prior records).
	const int32 PreSize = MsgSub->GetStats().HistorySize;

	for (int32 i = 0; i < 25; ++i)
	{
		FPGXMessage Msg;
		Msg.MessageTag = TestChannel;
		Msg.Timestamp = FPlatformTime::Seconds();
		MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);
	}

	const int32 PostSize = MsgSub->GetStats().HistorySize;
	MsgSub->ClearTestConfigs();

	// History was trimmed at MaxMessageHistory=16 even though we sent 25 (+ pre-existing).
	if (PostSize > 16)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: history not trimmed (Pre=%d Post=%d expected <=16)"), PreSize, PostSize));
		return false;
	}
	OutIssues.Add(FString::Printf(TEXT("OK: history bounded by MaxMessageHistory=16 (Pre=%d Post=%d)"), PreSize, PostSize));
	return true;
#else
	OutIssues.Add(TEXT("SKIP: requires WITH_EDITOR (InjectTestConfig is editor-only)"));
	return true;
#endif
}

bool UPGXMessageTestUtility::ClearHistoryAdditionalTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
#if WITH_EDITOR
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	UPGXMessageConfig* TestCfg = NewObject<UPGXMessageConfig>();
	TestCfg->MaxMessageHistory = 32;
	MsgSub->InjectTestConfig(TestCfg);

	int32 PostClearCount = 0;
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXMessage>(TestChannel,
		[&PostClearCount](FGameplayTag, const FPGXMessage&) { PostClearCount++; });

	// Broadcast a few, populate history.
	for (int32 i = 0; i < 5; ++i)
	{
		FPGXMessage Msg;
		Msg.Timestamp = FPlatformTime::Seconds();
		MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);
	}

	// Step A: capture pre-clear state.
	const int32 PreClearListeners = MsgSub->GetListenerCount(TestChannel);

	// Step B: ClearHistory and verify it actually empties records (not 1, not retained — exactly 0).
	MsgSub->ClearHistory();
	const int32 PostClearListeners = MsgSub->GetListenerCount(TestChannel);
	const int32 PostClearImmediateSize = MsgSub->GetStats().HistorySize;

	// Step C: post-clear broadcast — listener should still fire (ClearHistory only touches records).
	FPGXMessage MsgAfter;
	MsgAfter.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, MsgAfter);

	// Step D: capture post-broadcast state.
	const int32 PostBroadcastHistorySize = MsgSub->GetStats().HistorySize;

	Handle.Unregister();
	MsgSub->ClearTestConfigs();

	// Assertions — sequence matters: ClearHistory size==0, post-broadcast size==1.
	if (PostClearImmediateSize != 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: ClearHistory did not empty records (size=%d, expected 0)"), PostClearImmediateSize));
		return false;
	}
	if (PostBroadcastHistorySize != 1)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: post-clear broadcast did not append record (size=%d, expected 1)"), PostBroadcastHistorySize));
		return false;
	}
	if (PreClearListeners != PostClearListeners)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: ClearHistory mutated listener count (pre=%d post=%d)"), PreClearListeners, PostClearListeners));
		return false;
	}
	if (PostClearCount != 6)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: listener fire count = %d (expected 6 = 5 pre + 1 post)"), PostClearCount));
		return false;
	}
	OutIssues.Add(TEXT("OK: ClearHistory empties records (size=0), post-clear broadcast appends (size=1), listeners untouched, all 6 fires accounted"));
	return true;
#else
	OutIssues.Add(TEXT("SKIP: requires WITH_EDITOR (InjectTestConfig is editor-only)"));
	return true;
#endif
}

bool UPGXMessageTestUtility::EmergencyHistoryFallbackTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
#if WITH_EDITOR
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	// EN: Inject nullptr config to force fallback chain (CachedConfig invalid →
	//     Settings.EmergencyHistoryFallback). ClearTestConfigs at end re-discovers normally.
	MsgSub->InjectTestConfig(nullptr);

	FPGXMessage Msg;
	Msg.MessageTag = TestChannel;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);

	const int32 HistorySize = MsgSub->GetStats().HistorySize;

	MsgSub->ClearTestConfigs();

	// Settings.EmergencyHistoryFallback default 100 → at least 1 record retained.
	// (If user set it to 0 in their .ini, history is disabled and HistorySize stays at pre-test value.)
	const UPGXMessageSettings* Settings = GetDefault<UPGXMessageSettings>();
	const int32 ExpectedFallback = Settings ? Settings->EmergencyHistoryFallback : 0;

	if (ExpectedFallback > 0 && HistorySize == 0)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Settings.EmergencyHistoryFallback=%d but history is empty after broadcast"), ExpectedFallback));
		return false;
	}
	if (ExpectedFallback == 0 && HistorySize > 0)
	{
		OutIssues.Add(TEXT("INFO: Settings.EmergencyHistoryFallback=0 but history has records — possibly pre-existing; not a failure"));
	}

	OutIssues.Add(FString::Printf(TEXT("OK: emergency fallback path active (Settings.EmergencyHistoryFallback=%d, HistorySize=%d)"),
		ExpectedFallback, HistorySize));
	return true;
#else
	OutIssues.Add(TEXT("SKIP: requires WITH_EDITOR (InjectTestConfig is editor-only)"));
	return true;
#endif
}

// ============================================================
// Compatibility tests — Telemetry (telemetry + deferred-dispatch behavior)
// ============================================================

bool UPGXMessageTestUtility::FanOutTelemetryTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	const int32 ListenerCount = 5;
	TArray<FPGXMessageListenerHandle> Handles;
	Handles.Reserve(ListenerCount);
	for (int32 i = 0; i < ListenerCount; ++i)
	{
		Handles.Add(MsgSub->RegisterListener<FPGXMessage>(TestChannel,
			[](FGameplayTag, const FPGXMessage&) {}));
	}

	FPGXMessage Msg;
	Msg.MessageTag = TestChannel;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);

	const int32 PostMaxFanOut = MsgSub->GetStats().MaxFanOutOnSingleBroadcast;

	for (FPGXMessageListenerHandle& H : Handles) { H.Unregister(); }

	if (PostMaxFanOut < ListenerCount)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: MaxFanOutOnSingleBroadcast=%d, expected >=%d"), PostMaxFanOut, ListenerCount));
		return false;
	}
	OutIssues.Add(FString::Printf(TEXT("OK: fan-out telemetry tracks listener count (MaxFanOut=%d, listeners=%d)"), PostMaxFanOut, ListenerCount));
	return true;
}

bool UPGXMessageTestUtility::NestedBroadcastOrderingTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Deferred-dispatch ordering requires the parent fan-out to complete BEFORE a nested broadcast begins. Listener A
	//     on parent triggers a nested broadcast on child; listener B on parent must fire BEFORE
	//     the child listener fires. Nested broadcasts queue and drain FIFO after the parent loop.
	// ES: El orden deferred-dispatch exige completar el fan-out padre ANTES del nested broadcast. Listener B padre fires
	//     ANTES del child listener.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag ParentTag = TAG_PGX_Message_Gameplay.GetTag();
	const FGameplayTag ChildTag = TAG_PGX_Message_Audio.GetTag();
	if (!ParentTag.IsValid() || !ChildTag.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: tags not registered"));
		return false;
	}

	TArray<int32> FireOrder;  // sequence of fire IDs: 1=A, 2=B, 3=child.

	// Listener A on parent: fires first, broadcasts on child.
	FPGXMessageListenerHandle HandleA = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&FireOrder, MsgSub, ChildTag](FGameplayTag, const FPGXMessage&)
		{
			FireOrder.Add(1);
			FPGXMessage NestedMsg;
			NestedMsg.MessageTag = ChildTag;
			NestedMsg.Timestamp = FPlatformTime::Seconds();
			MsgSub->BroadcastMessage<FPGXMessage>(ChildTag, NestedMsg);
		});

	// Listener B on parent: must fire BEFORE the child listener with the deferred-dispatch ordering invariant.
	FPGXMessageListenerHandle HandleB = MsgSub->RegisterListener<FPGXMessage>(ParentTag,
		[&FireOrder](FGameplayTag, const FPGXMessage&) { FireOrder.Add(2); });

	// Listener on child: must fire LAST (after parent fan-out, drained from queue).
	FPGXMessageListenerHandle HandleChild = MsgSub->RegisterListener<FPGXMessage>(ChildTag,
		[&FireOrder](FGameplayTag, const FPGXMessage&) { FireOrder.Add(3); });

	FPGXMessage Msg;
	Msg.MessageTag = ParentTag;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(ParentTag, Msg);

	HandleA.Unregister(); HandleB.Unregister(); HandleChild.Unregister();

	// Expected deferred-dispatch order: 1, 2, 3.
	if (FireOrder.Num() != 3 || FireOrder[0] != 1 || FireOrder[1] != 2 || FireOrder[2] != 3)
	{
		FString Got;
		for (int32 v : FireOrder) { Got += FString::Printf(TEXT("%d "), v); }
		OutIssues.Add(FString::Printf(TEXT("FAIL: nested ordering (got: %s, expected: 1 2 3)"), *Got));
		return false;
	}
	OutIssues.Add(TEXT("OK: same-frame deferred-dispatch ordering — parent fan-out (1,2) completes before nested (3)"));
	return true;
}

bool UPGXMessageTestUtility::TimeAccelBoundaryReadOnlyTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: the wall-clock timestamp policy — Record.Timestamp uses FPlatformTime::Seconds (wall time only).
	//     Verify a broadcast's recorded timestamp matches platform wall time within epsilon.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Message_Debug.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	const double Pre = FPlatformTime::Seconds();
	FPGXMessage Msg;
	Msg.MessageTag = TestChannel;
	Msg.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXMessage>(TestChannel, Msg);
	const double Post = FPlatformTime::Seconds();

	const TArray<FPGXMessageRecord> History = MsgSub->GetMessageHistory(TestChannel, 1);
	if (History.Num() < 1)
	{
		OutIssues.Add(TEXT("FAIL: no history record after broadcast"));
		return false;
	}

	const double RecordedTs = History[0].Timestamp;
	if (RecordedTs < Pre || RecordedTs > Post + 0.5)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Record.Timestamp=%.3f outside [Pre=%.3f, Post=%.3f] window"),
			RecordedTs, Pre, Post));
		return false;
	}
	OutIssues.Add(FString::Printf(TEXT("OK: Record.Timestamp wall-time bounded (Pre=%.3f Recorded=%.3f Post=%.3f)"),
		Pre, RecordedTs, Post));
	return true;
}

// ============================================================
// FPGXBridgeGameFlowChanged appended-field compatibility
// ============================================================

bool UPGXMessageTestUtility::BridgePayloadExtensionPresenceTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Verify default-construct values for the 3 new fields. Compile-time + runtime guarantees:
	//     UStruct GENERATED_BODY default-init runs FGameplayTag() (IsValid==false), FGuid()
	//     (IsValid==false). Backward-compat invariant: legacy publishers that broadcast a
	//     bare FPGXBridgeGameFlowChanged{OldState, NewState, Timestamp} do not need to populate
	//     these — consumers read default-empty/invalid which is the documented "field absent" state.
	OutIssues.Empty();
	(void)WorldContextObject;

	FPGXBridgeGameFlowChanged Default;
	if (Default.Channel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: default Channel must be invalid (empty FGameplayTag)"));
		return false;
	}
	if (Default.TransitionSourceTag.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: default TransitionSourceTag must be invalid (empty FGameplayTag)"));
		return false;
	}
	if (Default.RequestId.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: default RequestId must be invalid (zero FGuid)"));
		return false;
	}
	OutIssues.Add(TEXT("OK: FPGXBridgeGameFlowChanged extension fields default-construct as invalid/empty (legacy publishers compatible)"));
	return true;
}

bool UPGXMessageTestUtility::BridgePayloadExtensionRoundtripTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Populate all extension fields on the publisher side, broadcast through Message bus on
	//     the bridge channel, verify the listener callback observes the populated values intact.
	//     Exercises the full FPGXBridgeGameFlowChanged path including new fields.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Bridge_GameFlow_StateChanged.GetTag();
	if (!TestChannel.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: TAG_PGX_Bridge_GameFlow_StateChanged not registered"));
		return false;
	}

	const FGameplayTag ExpectedChannel = TAG_PGX_Message_Gameplay.GetTag();
	const FGameplayTag ExpectedSource = TAG_PGX_Message_Debug.GetTag();
	const FGuid ExpectedRequestId = FGuid::NewGuid();
	const FGameplayTag ExpectedOldState = TAG_PGX_Message_System.GetTag();
	const FGameplayTag ExpectedNewState = TAG_PGX_Message_UI.GetTag();
	const double ExpectedTimestamp = 4242.42;

	FPGXBridgeGameFlowChanged ObservedPayload;
	bool bReceived = false;

	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXBridgeGameFlowChanged>(TestChannel,
		[&ObservedPayload, &bReceived](FGameplayTag /*Channel*/, const FPGXBridgeGameFlowChanged& Payload)
		{
			ObservedPayload = Payload;
			bReceived = true;
		});

	FPGXBridgeGameFlowChanged Sent;
	Sent.OldState = ExpectedOldState;
	Sent.NewState = ExpectedNewState;
	Sent.Timestamp = ExpectedTimestamp;
	Sent.Channel = ExpectedChannel;
	Sent.TransitionSourceTag = ExpectedSource;
	Sent.RequestId = ExpectedRequestId;

	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, Sent);
	Handle.Unregister();

	if (!bReceived)
	{
		OutIssues.Add(TEXT("FAIL: listener did not receive the bridge payload"));
		return false;
	}
	if (ObservedPayload.OldState != ExpectedOldState || ObservedPayload.NewState != ExpectedNewState)
	{
		OutIssues.Add(TEXT("FAIL: pre-existing OldState/NewState fields lost"));
		return false;
	}
	if (FMath::Abs(ObservedPayload.Timestamp - ExpectedTimestamp) > 0.001)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: Timestamp lost (got %.3f, expected %.3f)"),
			ObservedPayload.Timestamp, ExpectedTimestamp));
		return false;
	}
	if (ObservedPayload.Channel != ExpectedChannel)
	{
		OutIssues.Add(TEXT("FAIL: Channel field not roundtripped"));
		return false;
	}
	if (ObservedPayload.TransitionSourceTag != ExpectedSource)
	{
		OutIssues.Add(TEXT("FAIL: TransitionSourceTag field not roundtripped"));
		return false;
	}
	if (ObservedPayload.RequestId != ExpectedRequestId)
	{
		OutIssues.Add(TEXT("FAIL: RequestId field not roundtripped"));
		return false;
	}
	OutIssues.Add(TEXT("OK: FPGXBridgeGameFlowChanged extension roundtrip — all 6 fields preserved through Message bus"));
	return true;
}

bool UPGXMessageTestUtility::BridgePayloadExtensionBackwardCompatTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: legacy publisher style — only OldState/NewState/Timestamp populated; new fields stay
	//     at default. Consumer reads default-empty without crash; the listener observes the new
	//     fields as IsValid()==false. This is the documented documented backward-compatibility semantic.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Bridge_GameFlow_StateChanged.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	FPGXBridgeGameFlowChanged Observed;
	bool bReceived = false;

	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXBridgeGameFlowChanged>(TestChannel,
		[&Observed, &bReceived](FGameplayTag /*C*/, const FPGXBridgeGameFlowChanged& P)
		{
			Observed = P;
			bReceived = true;
		});

	// legacy publisher style — only the 3 original fields populated.
	FPGXBridgeGameFlowChanged LegacyStylePayload;
	LegacyStylePayload.OldState = TAG_PGX_Message_System.GetTag();
	LegacyStylePayload.NewState = TAG_PGX_Message_Gameplay.GetTag();
	LegacyStylePayload.Timestamp = FPlatformTime::Seconds();
	// Channel/TransitionSourceTag/RequestId left default-construct.

	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, LegacyStylePayload);
	Handle.Unregister();

	if (!bReceived) { OutIssues.Add(TEXT("FAIL: listener did not receive legacy-style payload")); return false; }
	if (Observed.Channel.IsValid()) { OutIssues.Add(TEXT("FAIL: Channel must be invalid in legacy-style payload")); return false; }
	if (Observed.TransitionSourceTag.IsValid()) { OutIssues.Add(TEXT("FAIL: TransitionSourceTag must be invalid in legacy-style payload")); return false; }
	if (Observed.RequestId.IsValid()) { OutIssues.Add(TEXT("FAIL: RequestId must be invalid in legacy-style payload")); return false; }
	if (!Observed.OldState.IsValid() || !Observed.NewState.IsValid())
	{
		OutIssues.Add(TEXT("FAIL: pre-existing fields lost in legacy-style payload"));
		return false;
	}
	OutIssues.Add(TEXT("OK: legacy-style payload (3 fields only) broadcasts and reads cleanly; new fields default-empty as documented"));
	return true;
}

bool UPGXMessageTestUtility::BridgePayloadExtensionRequestIdTest(const UObject* WorldContextObject, TArray<FString>& OutIssues)
{
	// EN: Caller-supplied RequestId is preserved exactly. Two distinct broadcasts with two
	//     distinct FGuid::NewGuid() values yield two distinct received RequestIds. Default-
	//     construct yields IsValid==false. The GameThread-only invariant ensures sequential
	//     observation here.
	OutIssues.Empty();
	UPGXMessageSubsystem* MsgSub = UPGXMessageSubsystem::Get(WorldContextObject);
	if (!IsValid(MsgSub)) { OutIssues.Add(TEXT("FAIL: MessageSubsystem not available")); return false; }

	const FGameplayTag TestChannel = TAG_PGX_Bridge_GameFlow_StateChanged.GetTag();
	if (!TestChannel.IsValid()) { OutIssues.Add(TEXT("FAIL: tag")); return false; }

	TArray<FGuid> ReceivedIds;
	FPGXMessageListenerHandle Handle = MsgSub->RegisterListener<FPGXBridgeGameFlowChanged>(TestChannel,
		[&ReceivedIds](FGameplayTag /*C*/, const FPGXBridgeGameFlowChanged& P)
		{
			ReceivedIds.Add(P.RequestId);
		});

	const FGuid IdA = FGuid::NewGuid();
	const FGuid IdB = FGuid::NewGuid();

	FPGXBridgeGameFlowChanged PayloadA;
	PayloadA.RequestId = IdA;
	PayloadA.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, PayloadA);

	FPGXBridgeGameFlowChanged PayloadB;
	PayloadB.RequestId = IdB;
	PayloadB.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, PayloadB);

	// Default-RequestId broadcast — listener should observe IsValid()==false.
	FPGXBridgeGameFlowChanged PayloadDefault;
	PayloadDefault.Timestamp = FPlatformTime::Seconds();
	MsgSub->BroadcastMessage<FPGXBridgeGameFlowChanged>(TestChannel, PayloadDefault);

	Handle.Unregister();

	if (ReceivedIds.Num() != 3)
	{
		OutIssues.Add(FString::Printf(TEXT("FAIL: expected 3 receives, got %d"), ReceivedIds.Num()));
		return false;
	}
	if (ReceivedIds[0] != IdA)
	{
		OutIssues.Add(TEXT("FAIL: PayloadA RequestId not preserved"));
		return false;
	}
	if (ReceivedIds[1] != IdB)
	{
		OutIssues.Add(TEXT("FAIL: PayloadB RequestId not preserved"));
		return false;
	}
	if (IdA == IdB)
	{
		OutIssues.Add(TEXT("FAIL: distinct FGuid::NewGuid() must yield distinct values"));
		return false;
	}
	if (ReceivedIds[2].IsValid())
	{
		OutIssues.Add(TEXT("FAIL: default-construct RequestId must be invalid (zero FGuid)"));
		return false;
	}
	OutIssues.Add(TEXT("OK: caller-supplied RequestId preserved; default-construct yields invalid FGuid; two NewGuid() distinct"));
	return true;
}
