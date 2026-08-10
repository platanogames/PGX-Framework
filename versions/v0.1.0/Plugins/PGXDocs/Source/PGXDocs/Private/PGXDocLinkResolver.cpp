// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXDocLinkResolver.h"
#include "PGXDocRegistry.h"
#include "Misc/Paths.h"
#include "Internationalization/Regex.h"

#if WITH_EDITOR
#include "SourceCodeNavigation.h"
#endif

FDocLinkResolver::FResolvedLink FDocLinkResolver::Resolve(const FString& CrossRef, const FDocRegistry& Registry) const
{
	FResolvedLink Result;
	Result.DisplayText = CrossRef;

	FString RefType, RefValue;

	if (CrossRef.Split(TEXT(":"), &RefType, &RefValue))
	{
		RefType = RefType.TrimStartAndEnd();
		RefValue = RefValue.TrimStartAndEnd();

		if (RefType.Equals(TEXT("Doc"), ESearchCase::IgnoreCase))
		{
			Result.Type = ELinkType::Doc;
			Result.Value = RefValue;
			Result.DisplayText = RefValue;
		}
		else if (RefType.Equals(TEXT("Class"), ESearchCase::IgnoreCase))
		{
			Result.Type = ELinkType::Class;
			Result.Value = RefValue;
			Result.DisplayText = RefValue;
		}
		else if (RefType.Equals(TEXT("BP"), ESearchCase::IgnoreCase))
		{
			Result.Type = ELinkType::Blueprint;
			Result.Value = RefValue;
			Result.DisplayText = RefValue;
		}
		else
		{
			// EN: Unknown type — treat as doc / ES: Tipo desconocido — tratar como doc
			Result.Type = ELinkType::Doc;
			Result.Value = CrossRef;
		}
	}
	else
	{
		// EN: No prefix — try Doc first, then Class / ES: Sin prefijo — intentar Doc primero, luego Class
		const FString DocPath = Registry.ResolveDocId(CrossRef);
		if (!DocPath.IsEmpty())
		{
			Result.Type = ELinkType::Doc;
			Result.Value = CrossRef;
		}
		else
		{
			// EN: Assume it's a class name / ES: Asumir que es un nombre de clase
			Result.Type = ELinkType::Class;
			Result.Value = CrossRef;
		}
	}

	return Result;
}

FString FDocLinkResolver::ProcessCrossRefsInHtml(const FString& Html, const FDocRegistry& Registry) const
{
	FString Result = Html;

	// EN: Find [[...]] patterns and replace with HTML links
	// ES: Encontrar patrones [[...]] y reemplazar con links HTML
	FRegexPattern Pattern(TEXT("\\[\\[([^\\]]+)\\]\\]"));
	FRegexMatcher Matcher(Pattern, Result);

	// EN: Collect replacements to avoid modifying string while iterating
	// ES: Recolectar reemplazos para evitar modificar el string mientras se itera
	TArray<TPair<FString, FString>> Replacements;

	while (Matcher.FindNext())
	{
		const FString FullMatch = Matcher.GetCaptureGroup(0);
		const FString CrossRef = Matcher.GetCaptureGroup(1);
		FResolvedLink Resolved = Resolve(CrossRef, Registry);

		FString LinkHtml;
		switch (Resolved.Type)
		{
		case ELinkType::Doc:
			LinkHtml = FString::Printf(
				TEXT("<a href=\"pgxdoc://%s\" class=\"pgx-crossref pgx-doc-link\">%s</a>"),
				*Resolved.Value, *Resolved.DisplayText);
			break;
		case ELinkType::Class:
			LinkHtml = FString::Printf(
				TEXT("<a href=\"pgxclass://%s\" class=\"pgx-crossref pgx-class-link\">%s</a>"),
				*Resolved.Value, *Resolved.DisplayText);
			break;
		case ELinkType::Blueprint:
			LinkHtml = FString::Printf(
				TEXT("<a href=\"pgxbp://%s\" class=\"pgx-crossref pgx-bp-link\">%s</a>"),
				*Resolved.Value, *Resolved.DisplayText);
			break;
		default:
			LinkHtml = FString::Printf(TEXT("<span class=\"pgx-crossref pgx-unresolved\">[[%s]]</span>"), *CrossRef);
			break;
		}

		Replacements.Add(TPair<FString, FString>(FullMatch, LinkHtml));
	}

	for (const auto& Replacement : Replacements)
	{
		Result = Result.Replace(*Replacement.Key, *Replacement.Value);
	}

	return Result;
}

void FDocLinkResolver::ExecuteLink(const FResolvedLink& Link) const
{
	switch (Link.Type)
	{
	case ELinkType::Doc:
		// EN: Navigation handled by the UI layer / ES: Navegacion manejada por la capa UI
		break;

	case ELinkType::Class:
		// EN: Open in source code editor via FSourceCodeNavigation
		// ES: Abrir en editor de codigo fuente via FSourceCodeNavigation
#if WITH_EDITOR
		{
			// EN: Find the UClass by name / ES: Encontrar la UClass por nombre
			UClass* FoundClass = FindObject<UClass>(nullptr, *FString::Printf(TEXT("/Script/PGXCoreRuntime.%s"), *Link.Value));
			if (!FoundClass)
			{
				// EN: Try broader search / ES: Intentar busqueda mas amplia
				FoundClass = FindFirstObject<UClass>(*Link.Value, EFindFirstObjectOptions::NativeFirst);
			}

			if (FoundClass)
			{
				FString ClassHeaderPath;
				if (FSourceCodeNavigation::FindClassHeaderPath(FoundClass, ClassHeaderPath))
				{
					FSourceCodeNavigation::OpenSourceFile(ClassHeaderPath);
				}
			}
		}
#endif
		break;

	case ELinkType::Blueprint:
		// EN: Open Blueprint asset in editor / ES: Abrir asset Blueprint en editor
		// EN: Handled by editor module with GEditor->GetEditorSubsystem / ES: Manejado por modulo editor
		break;

	case ELinkType::External:
		// EN: Open URL in default browser / ES: Abrir URL en navegador por defecto
		FPlatformProcess::LaunchURL(*Link.Value, nullptr, nullptr);
		break;

	default:
		break;
	}
}
