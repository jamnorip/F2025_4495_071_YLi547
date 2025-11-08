// TextFileManager.cpp
#include "TextFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/FileManager.h"

bool UTextFileManager::SaveArrayText(const FString& SaveDirectory, const FString& FileName, const TArray<FString>& SaveText, bool AllowOverWriting)
{
	// 組合完整路徑（跨平台方式）
	FString FullPath = FPaths::Combine(SaveDirectory, FileName);

	// 確保目錄存在
	FString DirectoryOnly = FPaths::GetPath(FullPath);
	if (!IFileManager::Get().DirectoryExists(*DirectoryOnly))
	{
		IFileManager::Get().MakeDirectory(*DirectoryOnly, true);
	}

	// 若不允許覆寫，檢查檔案是否存在
	if (!AllowOverWriting)
	{
		if (IFileManager::Get().FileExists(*FullPath))
		{
			return false;
		}
	}

	// 合併陣列為一個字串（每行換行）
	FString FinalString;
	for (const FString& Each : SaveText)
	{
		FinalString += Each;
		FinalString += LINE_TERMINATOR;
	}

	// 儲存為文字檔（使用 SaveStringToFile）
	return FFileHelper::SaveStringToFile(FinalString, *FullPath);
}
