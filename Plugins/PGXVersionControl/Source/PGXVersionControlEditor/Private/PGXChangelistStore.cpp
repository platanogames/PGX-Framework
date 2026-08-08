// SPDX-License-Identifier: Apache-2.0
// Copyright 2024-2026 Platano Games

#include "PGXChangelistStore.h"
#include "Logging/PGXLogMacros.h"
#include "PGXVersionControlEditor.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FPGXChangelistStore::FPGXChangelistStore()
{
}

FPGXChangelistStore::FPGXChangelistStore(const FString& InSavePathOverride)
	: SavePathOverride(InSavePathOverride)
{
}

FString FPGXChangelistStore::GetSavePath() const
{
	if (!SavePathOverride.IsEmpty())
	{
		return SavePathOverride;
	}
	return FPaths::ProjectSavedDir() / TEXT("PGX") / TEXT("changelists.json");
}

void FPGXChangelistStore::SetLastOperationResult(
	EPGXVersionControlOperationStatus InStatus,
	const FString& InMessage,
	const FString& InContextPath)
{
	LastOperationResult.Status = InStatus;
	LastOperationResult.Message = InMessage;
	LastOperationResult.ContextPath = InContextPath;
}

void FPGXChangelistStore::Load()
{
	FScopeLock Lock(&CriticalSection);

	const FString FilePath = GetSavePath();
	FString JsonString;

	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		PGX_LOG_INFO(LogPGXVersionControl, TEXT("ChangelistStore: No saved file found at %s — creating default."), *FilePath);
		EnsureDefaultChangelist();
		return;
	}

	TSharedPtr<FJsonObject> RootObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		PGX_LOG_WARNING(LogPGXVersionControl, TEXT("ChangelistStore: Failed to parse JSON — resetting to default."));
		Changelists.Empty();
		EnsureDefaultChangelist();
		return;
	}

	const int32 FileVersion = RootObj->GetIntegerField(TEXT("version"));
	if (FileVersion != JsonVersion)
	{
		PGX_LOG_WARNING(LogPGXVersionControl, TEXT("ChangelistStore: Version mismatch (file=%d, expected=%d) — resetting."), FileVersion, JsonVersion);
		Changelists.Empty();
		EnsureDefaultChangelist();
		return;
	}

	Changelists.Empty();
	const TArray<TSharedPtr<FJsonValue>>* CLArray = nullptr;
	if (RootObj->TryGetArrayField(TEXT("changelists"), CLArray))
	{
		for (const TSharedPtr<FJsonValue>& Val : *CLArray)
		{
			const TSharedPtr<FJsonObject>& CLObj = Val->AsObject();
			if (!CLObj.IsValid()) continue;

			FPGXChangelist CL;
			FGuid::Parse(CLObj->GetStringField(TEXT("guid")), CL.Guid);
			CL.DisplayName = CLObj->GetStringField(TEXT("display_name"));
			CL.Description = CLObj->GetStringField(TEXT("description"));
			CL.bIsDefault = CLObj->GetBoolField(TEXT("is_default"));

			FDateTime::ParseIso8601(*CLObj->GetStringField(TEXT("created_at")), CL.CreatedAt);
			FDateTime::ParseIso8601(*CLObj->GetStringField(TEXT("modified_at")), CL.ModifiedAt);

			const TArray<TSharedPtr<FJsonValue>>* FilesArray = nullptr;
			if (CLObj->TryGetArrayField(TEXT("files"), FilesArray))
			{
				for (const TSharedPtr<FJsonValue>& FileVal : *FilesArray)
				{
					CL.FilePaths.Add(FileVal->AsString());
				}
			}

			Changelists.Add(MoveTemp(CL));
		}
	}

	EnsureDefaultChangelist();
	PGX_LOG_INFO(LogPGXVersionControl, TEXT("ChangelistStore: Loaded %d changelists from disk."), Changelists.Num());
}

void FPGXChangelistStore::Save()
{
	SaveWithResult();
}

FPGXVersionControlOperationResult FPGXChangelistStore::SaveWithResult()
{
	FScopeLock Lock(&CriticalSection);

	TSharedRef<FJsonObject> RootObj = MakeShared<FJsonObject>();
	RootObj->SetNumberField(TEXT("version"), JsonVersion);

	TArray<TSharedPtr<FJsonValue>> CLArray;
	for (const FPGXChangelist& CL : Changelists)
	{
		TSharedRef<FJsonObject> CLObj = MakeShared<FJsonObject>();
		CLObj->SetStringField(TEXT("guid"), CL.Guid.ToString());
		CLObj->SetStringField(TEXT("display_name"), CL.DisplayName);
		CLObj->SetStringField(TEXT("description"), CL.Description);
		CLObj->SetBoolField(TEXT("is_default"), CL.bIsDefault);
		CLObj->SetStringField(TEXT("created_at"), CL.CreatedAt.ToIso8601());
		CLObj->SetStringField(TEXT("modified_at"), CL.ModifiedAt.ToIso8601());

		TArray<TSharedPtr<FJsonValue>> FilesArray;
		for (const FString& Path : CL.FilePaths)
		{
			FilesArray.Add(MakeShared<FJsonValueString>(Path));
		}
		CLObj->SetArrayField(TEXT("files"), FilesArray);

		CLArray.Add(MakeShared<FJsonValueObject>(CLObj));
	}
	RootObj->SetArrayField(TEXT("changelists"), CLArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObj, Writer);

	// EN: Atomic write — write to .tmp then rename / ES: Escritura atomica — escribir a .tmp luego renombrar
	const FString FinalPath = GetSavePath();
	const FString TmpPath = FinalPath + TEXT(".tmp");
	const FString BackupPath = FinalPath + TEXT(".bak");

	// EN: Ensure directory exists / ES: Asegurar que el directorio existe
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*FPaths::GetPath(FinalPath));

	if (!FFileHelper::SaveStringToFile(JsonString, *TmpPath))
	{
		PGX_LOG_ERROR(LogPGXVersionControl, TEXT("ChangelistStore: Failed to write temporary file %s."), *TmpPath);
		SetLastOperationResult(EPGXVersionControlOperationStatus::TempWriteFailed, TEXT("Failed to write temporary changelist file"), TmpPath);
		return LastOperationResult;
	}

	bool bBackupCreated = false;
	if (PlatformFile.FileExists(*FinalPath))
	{
		PlatformFile.DeleteFile(*BackupPath);
		if (!PlatformFile.MoveFile(*BackupPath, *FinalPath))
		{
			PGX_LOG_ERROR(LogPGXVersionControl, TEXT("ChangelistStore: Failed to create backup before atomic replace: %s"), *BackupPath);
			PlatformFile.DeleteFile(*TmpPath);
			SetLastOperationResult(EPGXVersionControlOperationStatus::BackupCreateFailed, TEXT("Failed to create backup before atomic replace"), BackupPath);
			return LastOperationResult;
		}
		bBackupCreated = true;
	}

	if (!PlatformFile.MoveFile(*FinalPath, *TmpPath))
	{
		PGX_LOG_ERROR(LogPGXVersionControl, TEXT("ChangelistStore: Atomic replace failed for %s."), *FinalPath);
		if (bBackupCreated)
		{
			PlatformFile.MoveFile(*FinalPath, *BackupPath);
		}
		PlatformFile.DeleteFile(*TmpPath);
		SetLastOperationResult(EPGXVersionControlOperationStatus::AtomicReplaceFailed, TEXT("Atomic replace failed; previous file restored when backup existed"), FinalPath);
		return LastOperationResult;
	}

	if (bBackupCreated)
	{
		PlatformFile.DeleteFile(*BackupPath);
	}

	PGX_LOG_INFO(LogPGXVersionControl, TEXT("ChangelistStore: Saved %d changelists to disk."), Changelists.Num());
	SetLastOperationResult(EPGXVersionControlOperationStatus::Success, TEXT("Changelists saved"), FinalPath);
	return LastOperationResult;
}

const FPGXChangelist* FPGXChangelistStore::FindChangelist(const FGuid& InGuid) const
{
	return Changelists.FindByPredicate([&InGuid](const FPGXChangelist& CL) { return CL.Guid == InGuid; });
}

FPGXChangelist* FPGXChangelistStore::FindChangelistMutable(const FGuid& InGuid)
{
	return Changelists.FindByPredicate([&InGuid](const FPGXChangelist& CL) { return CL.Guid == InGuid; });
}

FGuid FPGXChangelistStore::CreateChangelist(const FString& InName, const FString& InDescription)
{
	FScopeLock Lock(&CriticalSection);

	if (InName.TrimStartAndEnd().IsEmpty())
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::EmptyName, TEXT("Changelist name is empty"));
		return FGuid();
	}

	FPGXChangelist CL;
	CL.Guid = FGuid::NewGuid();
	CL.DisplayName = InName;
	CL.Description = InDescription;
	CL.bIsDefault = false;
	CL.CreatedAt = FDateTime::UtcNow();
	CL.ModifiedAt = CL.CreatedAt;

	Changelists.Add(MoveTemp(CL));
	OnChangelistsChanged.Broadcast();
	SetLastOperationResult(EPGXVersionControlOperationStatus::Success, TEXT("Changelist created"));
	return Changelists.Last().Guid;
}

bool FPGXChangelistStore::DeleteChangelist(const FGuid& InGuid)
{
	FScopeLock Lock(&CriticalSection);

	const int32 Idx = Changelists.IndexOfByPredicate([&InGuid](const FPGXChangelist& CL) { return CL.Guid == InGuid; });
	if (Idx == INDEX_NONE)
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::InvalidChangelist, TEXT("Changelist not found"));
		return false;
	}
	if (Changelists[Idx].bIsDefault)
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::DefaultChangelistProtected, TEXT("Default changelist cannot be deleted"));
		return false;
	}

	// EN: Move orphaned files to default CL / ES: Mover archivos huerfanos al CL por defecto
	FPGXChangelist* DefaultCL = Changelists.FindByPredicate([](const FPGXChangelist& CL) { return CL.bIsDefault; });
	if (DefaultCL)
	{
		for (const FString& Path : Changelists[Idx].FilePaths)
		{
			DefaultCL->FilePaths.AddUnique(Path);
		}
		DefaultCL->ModifiedAt = FDateTime::UtcNow();
	}

	Changelists.RemoveAt(Idx);
	OnChangelistsChanged.Broadcast();
	SetLastOperationResult(EPGXVersionControlOperationStatus::Success, TEXT("Changelist deleted"));
	return true;
}

bool FPGXChangelistStore::RenameChangelist(const FGuid& InGuid, const FString& InNewName)
{
	FScopeLock Lock(&CriticalSection);

	FPGXChangelist* CL = FindChangelistMutable(InGuid);
	if (!CL)
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::InvalidChangelist, TEXT("Changelist not found"));
		return false;
	}
	if (InNewName.TrimStartAndEnd().IsEmpty())
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::EmptyName, TEXT("Changelist name is empty"));
		return false;
	}

	CL->DisplayName = InNewName;
	CL->ModifiedAt = FDateTime::UtcNow();
	OnChangelistsChanged.Broadcast();
	SetLastOperationResult(EPGXVersionControlOperationStatus::Success, TEXT("Changelist renamed"));
	return true;
}

bool FPGXChangelistStore::MoveFileToChangelist(const FString& InFilePath, const FGuid& InTargetGuid)
{
	FScopeLock Lock(&CriticalSection);

	FPGXChangelist* TargetCL = FindChangelistMutable(InTargetGuid);
	if (!TargetCL)
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::InvalidChangelist, TEXT("Target changelist not found"));
		return false;
	}
	if (InFilePath.TrimStartAndEnd().IsEmpty())
	{
		SetLastOperationResult(EPGXVersionControlOperationStatus::EmptyFilePath, TEXT("File path is empty"));
		return false;
	}

	// EN: Remove from all other changelists / ES: Remover de todas las otras changelists
	for (FPGXChangelist& CL : Changelists)
	{
		if (CL.Guid != InTargetGuid)
		{
			CL.FilePaths.Remove(InFilePath);
		}
	}

	TargetCL->FilePaths.AddUnique(InFilePath);
	TargetCL->ModifiedAt = FDateTime::UtcNow();
	OnChangelistsChanged.Broadcast();
	SetLastOperationResult(EPGXVersionControlOperationStatus::Success, TEXT("File moved to changelist"), InFilePath);
	return true;
}

FGuid FPGXChangelistStore::GetDefaultChangelistGuid() const
{
	const FPGXChangelist* DefaultCL = Changelists.FindByPredicate([](const FPGXChangelist& CL) { return CL.bIsDefault; });
	return DefaultCL ? DefaultCL->Guid : FGuid();
}

void FPGXChangelistStore::EnsureDefaultChangelist()
{
	const bool bHasDefault = Changelists.ContainsByPredicate([](const FPGXChangelist& CL) { return CL.bIsDefault; });
	if (!bHasDefault)
	{
		FPGXChangelist DefaultCL;
		DefaultCL.Guid = FGuid::NewGuid();
		DefaultCL.DisplayName = TEXT("Default");
		DefaultCL.Description = TEXT("");
		DefaultCL.bIsDefault = true;
		DefaultCL.CreatedAt = FDateTime::UtcNow();
		DefaultCL.ModifiedAt = DefaultCL.CreatedAt;
		Changelists.Insert(MoveTemp(DefaultCL), 0);
	}
}
