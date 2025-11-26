// Copyright with YaolongLiu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestructionAsset.generated.h"

UCLASS()
class WORLDREBALANCE_API ADestructionAsset : public AActor
{
	GENERATED_BODY()

public:
	ADestructionAsset();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Meshes")
	TArray<UChildActorComponent*> MeshComponents;

	// 最大部件數，建議固定（比如 10）
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	int32 MaxParts = 10;

	// 實際要啟用多少個（BP 可調）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
	int32 PartNum = 3;
};
