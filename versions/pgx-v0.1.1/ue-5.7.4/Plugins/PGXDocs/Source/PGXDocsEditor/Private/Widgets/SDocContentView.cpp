// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "Widgets/SDocContentView.h"
#include "Render/SDocSlateRenderer.h"
#include "PGXDocsModule.h"
#include "Logging/PGXLogMacros.h"
#include "PGXDocSystem.h"
#include "PGXDocLinkResolver.h"
#include "PGXDocTypes.h"

#define LOCTEXT_NAMESPACE "PGXDocs"

void SDocContentView::Construct(const FArguments& /*InArgs*/)
{
	ChildSlot
	[
		SAssignNew(SlateRenderer, SDocSlateRenderer)
		.Config(nullptr)
	];

	SlateRenderer->OnLinkClicked.BindRaw(this, &SDocContentView::HandleLinkClicked);

	// EN: Build welcome page as FDocElement array / ES: Construir welcome page como array FDocElement
	TArray<FDocElement> Welcome;

	FDocElement H1;
	H1.Type = EDocElementType::Heading;
	H1.HeadingLevel = 1;
	FDocTextRun H1Run;
	H1Run.Text = TEXT("PGX Documentation");
	H1.Runs.Add(MoveTemp(H1Run));
	Welcome.Add(MoveTemp(H1));

	FDocElement P1;
	P1.Type = EDocElementType::Paragraph;
	FDocTextRun P1Run;
	P1Run.Text = TEXT("Select a document from the tree to begin.");
	P1.Runs.Add(MoveTemp(P1Run));
	Welcome.Add(MoveTemp(P1));

	FDocElement P2;
	P2.Type = EDocElementType::Paragraph;
	FDocTextRun P2Run;
	P2Run.Text = TEXT("PGX Core v0.4.0");
	P2Run.Style = EDocTextStyle::Italic;
	P2.Runs.Add(MoveTemp(P2Run));
	Welcome.Add(MoveTemp(P2));

	SlateRenderer->SetContent(Welcome, TEXT("PGX Documentation"));
}

void SDocContentView::LoadDocument(const FString& DocId)
{
	CurrentDocId = DocId;

	TSharedPtr<FDocDocument> Doc = FDocSystem::Get().LoadDocument(DocId);
	if (!Doc.IsValid())
	{
		PGX_LOG_WARNING(LogPGXDocs, TEXT("PGXDocs: Document NOT FOUND — DocId='%s'"), *DocId);

		TArray<FDocElement> ErrorElements;
		FDocElement ErrP;
		ErrP.Type = EDocElementType::Paragraph;
		FDocTextRun ErrRun;
		ErrRun.Text = FString::Printf(TEXT("Document not found: %s"), *DocId);
		ErrRun.Style = EDocTextStyle::Bold;
		ErrP.Runs.Add(MoveTemp(ErrRun));
		ErrorElements.Add(MoveTemp(ErrP));

		if (SlateRenderer.IsValid())
		{
			SlateRenderer->SetContent(ErrorElements, TEXT("Error"));
		}
		return;
	}

	// EN: Parse markdown to element tree / ES: Parsear markdown a arbol de elementos
	TArray<FDocElement> Elements = FDocSystem::ParseMarkdownToTree(Doc->RawMarkdown);

	if (SlateRenderer.IsValid())
	{
		SlateRenderer->SetContent(Elements, Doc->Title);
	}
}

void SDocContentView::Clear()
{
	CurrentDocId.Empty();
	if (SlateRenderer.IsValid())
	{
		SlateRenderer->Clear();
	}
}

void SDocContentView::HandleLinkClicked(const FString& Url)
{
	if (Url.StartsWith(TEXT("pgxdoc://")))
	{
		// EN: Internal doc navigation / ES: Navegacion interna de doc
		FString DocId = Url.Mid(9);
		LoadDocument(DocId);
		return;
	}

	if (Url.StartsWith(TEXT("pgxclass://")))
	{
		// EN: Open C++ class / ES: Abrir clase C++
		FString ClassName = Url.Mid(11);
		FDocLinkResolver::FResolvedLink Link;
		Link.Type = FDocLinkResolver::ELinkType::Class;
		Link.Value = ClassName;
		FDocSystem::Get().GetLinkResolver().ExecuteLink(Link);
		return;
	}

	if (Url.StartsWith(TEXT("pgxbp://")))
	{
		// EN: Open Blueprint / ES: Abrir Blueprint
		FString BPPath = Url.Mid(8);
		FDocLinkResolver::FResolvedLink Link;
		Link.Type = FDocLinkResolver::ELinkType::Blueprint;
		Link.Value = BPPath;
		FDocSystem::Get().GetLinkResolver().ExecuteLink(Link);
		return;
	}

	// EN: External URLs — block by default / ES: URLs externas — bloquear por defecto
	PGX_LOG_INFO(LogPGXDocs, TEXT("PGXDocs: External link blocked: %s"), *Url);
}

#undef LOCTEXT_NAMESPACE
