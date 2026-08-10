// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocSearchIndex.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocsModule.h"
#include "Misc/FileHelper.h"
#include "Internationalization/Regex.h"
#include "Hash/CityHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void FDocSearchIndex::IndexDocument(const FDocDocument& Document)
{
	FIndexEntry Entry;
	Entry.DocId = Document.DocId;
	Entry.Title = Document.Title;
	Entry.ModuleId = Document.ModuleId;
	Entry.Tags = Document.Tags;
	Entry.ModifiedTimeUtc = Document.ModifiedTimeUtc;

	// EN: Strip markdown to plain text for searching / ES: Quitar markdown a texto plano para busqueda
	Entry.PlainText = StripMarkdown(Document.RawMarkdown);
	Entry.Headings = ExtractHeadings(Document.RawMarkdown);
	Entry.ContentHash = ComputeHash(Document.RawMarkdown);

	IndexEntries.Add(Document.DocId, MoveTemp(Entry));
}

void FDocSearchIndex::RemoveDocument(const FString& DocId)
{
	IndexEntries.Remove(DocId);
}

void FDocSearchIndex::Clear()
{
	IndexEntries.Empty();
}

TArray<FDocSearchResult> FDocSearchIndex::Search(const FString& Query, const FDocSearchFilters& Filters, int32 MaxResults) const
{
	TArray<FDocSearchResult> Results;

	if (Query.IsEmpty())
	{
		return Results;
	}

	const FString QueryLower = Query.ToLower();
	TArray<FString> QueryTerms;
	QueryLower.ParseIntoArrayWS(QueryTerms);

	for (const auto& Pair : IndexEntries)
	{
		const FIndexEntry& Entry = Pair.Value;

		// EN: Apply module filter / ES: Aplicar filtro de modulo
		if (!Filters.ModuleFilter.IsEmpty() && Entry.ModuleId != Filters.ModuleFilter)
		{
			continue;
		}

		// EN: Apply tag filters / ES: Aplicar filtros de tags
		if (Filters.TagFilters.Num() > 0)
		{
			bool bHasTag = false;
			for (const FString& TagFilter : Filters.TagFilters)
			{
				if (Entry.Tags.Contains(TagFilter))
				{
					bHasTag = true;
					break;
				}
			}
			if (!bHasTag)
			{
				continue;
			}
		}

		int32 Score = 0;
		FString MatchContext;

		const FString TitleLower = Entry.Title.ToLower();
		const FString PlainLower = Entry.PlainText.ToLower();

		for (const FString& Term : QueryTerms)
		{
			// EN: Title match (highest weight) / ES: Match en titulo (peso mas alto)
			if (TitleLower.Contains(Term))
			{
				Score += 100;
			}

			// EN: Heading match / ES: Match en heading
			for (const FString& Heading : Entry.Headings)
			{
				if (Heading.ToLower().Contains(Term))
				{
					Score += 50;
					break;
				}
			}

			// EN: Body match / ES: Match en body
			int32 BodyPos = PlainLower.Find(Term);
			if (BodyPos != INDEX_NONE)
			{
				Score += 10;

				// EN: Extract snippet around match / ES: Extraer extracto alrededor del match
				if (MatchContext.IsEmpty())
				{
					const int32 SnippetStart = FMath::Max(0, BodyPos - 40);
					const int32 SnippetLen = FMath::Min(120, Entry.PlainText.Len() - SnippetStart);
					MatchContext = Entry.PlainText.Mid(SnippetStart, SnippetLen);
					if (SnippetStart > 0)
					{
						MatchContext = TEXT("...") + MatchContext;
					}
					if (SnippetStart + SnippetLen < Entry.PlainText.Len())
					{
						MatchContext += TEXT("...");
					}
				}
			}
		}

		if (Score > 0)
		{
			FDocSearchResult Result;
			Result.DocId = Entry.DocId;
			Result.Title = Entry.Title;
			Result.ModuleId = Entry.ModuleId;
			Result.MatchContext = MatchContext;
			Result.Score = Score;
			Results.Add(MoveTemp(Result));
		}
	}

	// EN: Sort by score descending / ES: Ordenar por puntuacion descendente
	Results.Sort([](const FDocSearchResult& A, const FDocSearchResult& B)
	{
		return A.Score > B.Score;
	});

	// EN: Truncate to max results / ES: Truncar a maximo de resultados
	if (Results.Num() > MaxResults)
	{
		Results.SetNum(MaxResults);
	}

	return Results;
}

FString FDocSearchIndex::StripMarkdown(const FString& Markdown)
{
	FString Result = Markdown;

	// EN: Remove code blocks / ES: Remover bloques de codigo
	while (true)
	{
		int32 Start = Result.Find(TEXT("```"));
		if (Start == INDEX_NONE) break;
		int32 End = Result.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Start + 3);
		if (End == INDEX_NONE) break;
		Result = Result.Left(Start) + TEXT(" ") + Result.Mid(End + 3);
	}

	// EN: Remove inline code / ES: Remover codigo inline
	Result.ReplaceInline(TEXT("`"), TEXT(""));

	// EN: Remove markdown formatting characters / ES: Remover caracteres de formato markdown
	Result.ReplaceInline(TEXT("**"), TEXT(""));
	Result.ReplaceInline(TEXT("__"), TEXT(""));
	Result.ReplaceInline(TEXT("~~"), TEXT(""));
	Result.ReplaceInline(TEXT("*"), TEXT(""));
	Result.ReplaceInline(TEXT("_"), TEXT(" "));

	// EN: Remove headings markers / ES: Remover marcadores de headings
	Result.ReplaceInline(TEXT("######"), TEXT(""));
	Result.ReplaceInline(TEXT("#####"), TEXT(""));
	Result.ReplaceInline(TEXT("####"), TEXT(""));
	Result.ReplaceInline(TEXT("###"), TEXT(""));
	Result.ReplaceInline(TEXT("##"), TEXT(""));
	Result.ReplaceInline(TEXT("# "), TEXT(""));

	// EN: Remove link syntax / ES: Remover sintaxis de links
	// Basic: [text](url) → text
	FRegexPattern LinkPattern(TEXT("\\[([^\\]]+)\\]\\([^)]+\\)"));
	FRegexMatcher LinkMatcher(LinkPattern, Result);
	while (LinkMatcher.FindNext())
	{
		Result = Result.Replace(*LinkMatcher.GetCaptureGroup(0), *LinkMatcher.GetCaptureGroup(1));
		LinkMatcher = FRegexMatcher(LinkPattern, Result);
	}

	// EN: Remove image syntax / ES: Remover sintaxis de imagenes
	FRegexPattern ImgPattern(TEXT("!\\[([^\\]]*)\\]\\([^)]+\\)"));
	FRegexMatcher ImgMatcher(ImgPattern, Result);
	while (ImgMatcher.FindNext())
	{
		Result = Result.Replace(*ImgMatcher.GetCaptureGroup(0), *ImgMatcher.GetCaptureGroup(1));
		ImgMatcher = FRegexMatcher(ImgPattern, Result);
	}

	// EN: Remove HTML tags / ES: Remover tags HTML
	FRegexPattern HtmlPattern(TEXT("<[^>]+>"));
	FRegexMatcher HtmlMatcher(HtmlPattern, Result);
	while (HtmlMatcher.FindNext())
	{
		Result = Result.Replace(*HtmlMatcher.GetCaptureGroup(0), TEXT(""));
		HtmlMatcher = FRegexMatcher(HtmlPattern, Result);
	}

	// EN: Remove table pipes / ES: Remover pipes de tablas
	Result.ReplaceInline(TEXT("|"), TEXT(" "));
	Result.ReplaceInline(TEXT("---"), TEXT(""));

	// EN: Collapse whitespace / ES: Colapsar espacios en blanco
	while (Result.Contains(TEXT("  ")))
	{
		Result.ReplaceInline(TEXT("  "), TEXT(" "));
	}

	return Result.TrimStartAndEnd();
}

TArray<FString> FDocSearchIndex::ExtractHeadings(const FString& Markdown)
{
	TArray<FString> Headings;

	TArray<FString> Lines;
	Markdown.ParseIntoArrayLines(Lines);

	for (const FString& Line : Lines)
	{
		FString Trimmed = Line.TrimStart();
		if (Trimmed.StartsWith(TEXT("#")))
		{
			// EN: Strip # characters and whitespace / ES: Quitar caracteres # y espacios
			int32 HashCount = 0;
			while (HashCount < Trimmed.Len() && Trimmed[HashCount] == TEXT('#'))
			{
				HashCount++;
			}
			FString HeadingText = Trimmed.Mid(HashCount).TrimStart();
			if (!HeadingText.IsEmpty())
			{
				Headings.Add(HeadingText);
			}
		}
	}

	return Headings;
}

uint64 FDocSearchIndex::ComputeHash(const FString& Content)
{
	return CityHash64(reinterpret_cast<const char*>(*Content), Content.Len() * sizeof(TCHAR));
}

bool FDocSearchIndex::SaveCache(const FString& CachePath) const
{
	TSharedRef<FJsonObject> RootObj = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> EntriesArray;

	for (const auto& Pair : IndexEntries)
	{
		const FIndexEntry& Entry = Pair.Value;
		TSharedPtr<FJsonObject> EntryObj = MakeShared<FJsonObject>();
		EntryObj->SetStringField(TEXT("docId"), Entry.DocId);
		EntryObj->SetStringField(TEXT("title"), Entry.Title);
		EntryObj->SetStringField(TEXT("moduleId"), Entry.ModuleId);
		EntryObj->SetNumberField(TEXT("contentHash"), static_cast<double>(Entry.ContentHash));
		EntryObj->SetNumberField(TEXT("modifiedTime"), static_cast<double>(Entry.ModifiedTimeUtc));
		EntryObj->SetStringField(TEXT("plainText"), Entry.PlainText);

		TArray<TSharedPtr<FJsonValue>> HeadingsArr;
		for (const FString& H : Entry.Headings)
		{
			HeadingsArr.Add(MakeShared<FJsonValueString>(H));
		}
		EntryObj->SetArrayField(TEXT("headings"), HeadingsArr);

		TArray<TSharedPtr<FJsonValue>> TagsArr;
		for (const FString& T : Entry.Tags)
		{
			TagsArr.Add(MakeShared<FJsonValueString>(T));
		}
		EntryObj->SetArrayField(TEXT("tags"), TagsArr);

		EntriesArray.Add(MakeShared<FJsonValueObject>(EntryObj));
	}

	RootObj->SetArrayField(TEXT("entries"), EntriesArray);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(RootObj, Writer);

	return FFileHelper::SaveStringToFile(OutputString, *CachePath);
}

bool FDocSearchIndex::LoadCache(const FString& CachePath)
{
	FString JsonStr;
	if (!FFileHelper::LoadFileToString(JsonStr, *CachePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> RootObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		return false;
	}

	const TArray<TSharedPtr<FJsonValue>>* EntriesArray = nullptr;
	if (!RootObj->TryGetArrayField(TEXT("entries"), EntriesArray))
	{
		return false;
	}

	IndexEntries.Empty();
	for (const TSharedPtr<FJsonValue>& EntryVal : *EntriesArray)
	{
		const TSharedPtr<FJsonObject>& EntryObj = EntryVal->AsObject();
		if (!EntryObj.IsValid()) continue;

		FIndexEntry Entry;
		Entry.DocId = EntryObj->GetStringField(TEXT("docId"));
		Entry.Title = EntryObj->GetStringField(TEXT("title"));
		Entry.ModuleId = EntryObj->GetStringField(TEXT("moduleId"));
		Entry.ContentHash = static_cast<uint64>(EntryObj->GetNumberField(TEXT("contentHash")));
		Entry.ModifiedTimeUtc = static_cast<int64>(EntryObj->GetNumberField(TEXT("modifiedTime")));
		Entry.PlainText = EntryObj->GetStringField(TEXT("plainText"));

		const TArray<TSharedPtr<FJsonValue>>* HeadingsArr;
		if (EntryObj->TryGetArrayField(TEXT("headings"), HeadingsArr))
		{
			for (const auto& H : *HeadingsArr)
			{
				Entry.Headings.Add(H->AsString());
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* TagsArr;
		if (EntryObj->TryGetArrayField(TEXT("tags"), TagsArr))
		{
			for (const auto& T : *TagsArr)
			{
				Entry.Tags.Add(T->AsString());
			}
		}

		IndexEntries.Add(Entry.DocId, MoveTemp(Entry));
	}

	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: Loaded search index cache with %d entries"), IndexEntries.Num());
	return true;
}
