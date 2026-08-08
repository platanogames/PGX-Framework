// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games
#include "Logging/PGXLogEntryBuilder.h"
#include "Logging/PGXLogCategories.h"
#include "Logging/PGXLogTagHelper.h"
#include "Subsystems/PGXLogSubsystem.h"

// EN: RAII log entry builder implementation
// ES: Implementacion del builder RAII de entradas de log

FPGXLogEntryBuilder::FPGXLogEntryBuilder(FName InCategory, EPGXLogVerbosity InVerbosity, FString&& InMessage)
	: Category(InCategory)
	, Verbosity(InVerbosity)
	, Message(MoveTemp(InMessage))
{
}

FPGXLogEntryBuilder& FPGXLogEntryBuilder::Tag(const FGameplayTag& InTag)
{
	CustomTags.AddTag(InTag);
	return *this;
}

FPGXLogEntryBuilder& FPGXLogEntryBuilder::Tags(const FGameplayTagContainer& InTags)
{
	CustomTags.AppendTags(InTags);
	return *this;
}

FPGXLogEntryBuilder& FPGXLogEntryBuilder::Domain(const FGameplayTag& InDomainTag)
{
	DomainTag = InDomainTag;
	return *this;
}

FPGXLogEntryBuilder::FPGXLogEntryBuilder(FPGXLogEntryBuilder&& Other) noexcept
	: Category(Other.Category)
	, Verbosity(Other.Verbosity)
	, Message(MoveTemp(Other.Message))
	, Params(MoveTemp(Other.Params))
	, CustomTags(MoveTemp(Other.CustomTags))
	, DomainTag(Other.DomainTag)
	, bSubmitted(Other.bSubmitted)
{
	// EN: Prevent the moved-from object from submitting
	// ES: Evitar que el objeto movido envie
	Other.bSubmitted = true;
}

FPGXLogEntryBuilder::~FPGXLogEntryBuilder()
{
	if (!bSubmitted)
	{
		Submit();
	}
}

void FPGXLogEntryBuilder::Submit()
{
	bSubmitted = true;

	// EN: Build the structured entry
	// ES: Construir la entrada estructurada
	FPGXLogEntry Entry;
	Entry.Message = BuildFormattedMessage();
	Entry.Category = Category;
	Entry.Verbosity = Verbosity;
	Entry.Timestamp = FDateTime::UtcNow();
	Entry.FrameNumber = GFrameCounter;
	Entry.Params = MoveTemp(Params);
	Entry.DomainTag = DomainTag;

	// EN: Auto-populate tags (verbosity + context) then append developer custom tags
	// ES: Auto-popular tags (verbosity + contexto) luego agregar tags custom del developer
	FPGXLogTagHelper::AutoPopulateTags(Entry);
	Entry.Tags.AppendTags(CustomTags);

	// EN: Bridge to UE_LOG with appropriate verbosity
	// ES: Puente a UE_LOG con la verbosity apropiada
	const FString& LogMsg = Entry.Message;
	switch (Verbosity)
	{
	case EPGXLogVerbosity::Verbose:
		UE_LOG(LogPGX, Verbose, TEXT("[PGX] %s"), *LogMsg);
		break;
	case EPGXLogVerbosity::Debug:
		UE_LOG(LogPGX, Log, TEXT("[DBG] %s"), *LogMsg);
		break;
	case EPGXLogVerbosity::Info:
		UE_LOG(LogPGX, Log, TEXT("[INF] %s"), *LogMsg);
		break;
	case EPGXLogVerbosity::Warning:
		UE_LOG(LogPGX, Warning, TEXT("[WRN] %s"), *LogMsg);
		break;
	case EPGXLogVerbosity::Error:
		UE_LOG(LogPGX, Error, TEXT("[ERR] %s"), *LogMsg);
		break;
	case EPGXLogVerbosity::Fatal:
		UE_LOG(LogPGX, Error, TEXT("[FTL] %s"), *LogMsg);
		break;
	}

	// EN: Submit to subsystem if available (may not be during early startup)
	// ES: Enviar al subsistema si esta disponible (puede no estar durante startup temprano)
	if (UPGXLogSubsystem* LogSub = UPGXLogSubsystem::GetCached())
	{
		LogSub->AddEntry(MoveTemp(Entry));
	}
}

FString FPGXLogEntryBuilder::BuildFormattedMessage() const
{
	if (Params.Num() == 0)
	{
		return Message;
	}

	// EN: Format: "message | key=value | key2=value2"
	// ES: Formato: "message | key=value | key2=value2"
	FString Result = Message;
	for (const FPGXLogParam& Param : Params)
	{
		Result += FString::Printf(TEXT(" | %s=%s"), *Param.Key, *Param.Value);
	}
	return Result;
}
