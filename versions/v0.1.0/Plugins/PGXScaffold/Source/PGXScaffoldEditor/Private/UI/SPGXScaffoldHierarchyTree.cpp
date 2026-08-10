// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "UI/SPGXScaffoldHierarchyTree.h"
#include "Core/PGXHierarchyResolver.h"
#include "Style/PGXVisualTokens.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "PGXScaffoldHierarchyTree"

void SPGXScaffoldHierarchyTree::Construct(const FArguments& InArgs)
{
	RootNodes = InArgs._RootNodes;
	OnNodeToggled = InArgs._OnNodeToggled;

	TSharedRef<SVerticalBox> TreeContainer = SNew(SVerticalBox);
	BuildTreeRows(TreeContainer, RootNodes, 0);

	ChildSlot
	[
		TreeContainer
	];
}

void SPGXScaffoldHierarchyTree::BuildTreeRows(
	TSharedRef<SVerticalBox> Container,
	const TArray<TSharedPtr<FPGXScaffoldTreeNode>>& Nodes,
	int32 IndentLevel)
{
	for (const TSharedPtr<FPGXScaffoldTreeNode>& Node : Nodes)
	{
		if (!Node.IsValid()) { continue; }

		// EN: Type icon text / ES: Texto de icono por tipo
		FString TypeIcon;
		FLinearColor TypeColor;
		switch (Node->Item.ActionType)
		{
		case EPGXScaffoldActionType::CreateFolder:    TypeIcon = TEXT("[D]"); TypeColor = PGX::System::Scaffold; break;
		case EPGXScaffoldActionType::CreateDataAsset: TypeIcon = TEXT("[A]"); TypeColor = PGX::Semantic::Info; break;
		case EPGXScaffoldActionType::CreateBlueprint: TypeIcon = TEXT("[B]"); TypeColor = PGX::Semantic::Good; break;
		}

		TWeakPtr<FPGXScaffoldTreeNode> WeakNode = Node;

		Container->AddSlot().AutoHeight().Padding(0, 1.0f)
		[
			SNew(SHorizontalBox)

			// EN: Indent / ES: Indentacion
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox).WidthOverride(static_cast<float>(IndentLevel * 20))
				[
					SNullWidget::NullWidget
				]
			]

			// EN: Checkbox / ES: Checkbox
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
					.IsChecked_Lambda([WeakNode]() -> ECheckBoxState
					{
						TSharedPtr<FPGXScaffoldTreeNode> N = WeakNode.Pin();
						if (!N.IsValid()) { return ECheckBoxState::Unchecked; }

						if (!N->bChecked) { return ECheckBoxState::Unchecked; }
						if (N->Children.Num() > 0 && !N->AllChildrenChecked()) { return ECheckBoxState::Undetermined; }
						return ECheckBoxState::Checked;
					})
					.OnCheckStateChanged_Lambda([this, WeakNode](ECheckBoxState NewState)
					{
						TSharedPtr<FPGXScaffoldTreeNode> N = WeakNode.Pin();
						if (!N.IsValid()) { return; }

						bool bNewChecked = (NewState != ECheckBoxState::Unchecked);
						FPGXHierarchyResolver::ToggleNode(N, bNewChecked);

						if (OnNodeToggled.IsBound())
						{
							OnNodeToggled.Execute();
						}
					})
			]

			// EN: Type badge / ES: Badge de tipo
			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString(TypeIcon))
					.Font(PGX::Font::Badge())
					.ColorAndOpacity(TypeColor)
			]

			// EN: Display name / ES: Nombre de display
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(Node->Item.DisplayName)
					.Font(PGX::Font::Body())
			]

			// EN: Relative path hint / ES: Pista de ruta relativa
			+ SHorizontalBox::Slot().AutoWidth().Padding(PGX::Spacing::SM, 0.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString(Node->Item.RelativePath))
					.Font(PGX::Font::Caption())
					.ColorAndOpacity(PGX::Text::Muted)
			]
		];

		// EN: Recurse into children / ES: Recursar en hijos
		if (Node->Children.Num() > 0)
		{
			BuildTreeRows(Container, Node->Children, IndentLevel + 1);
		}
	}
}

#undef LOCTEXT_NAMESPACE
