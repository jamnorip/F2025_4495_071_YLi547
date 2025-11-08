// TextFileManager.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TextFileManager.generated.h"

// 使用你專案的 API 宏（從你的編譯日誌看，專案名是 WorldRebalance）
UCLASS()
class WORLDREBALANCE_API UTextFileManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 預設參數放在這裡
	UFUNCTION(BlueprintCallable, Category = "File")
	static bool SaveArrayText(const FString& SaveDirectory, const FString& FileName, const TArray<FString>& SaveText, bool AllowOverWriting = false);
};
