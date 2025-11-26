#include "DestructionAsset.h"

ADestructionAsset::ADestructionAsset()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 固定建立 MaxParts 個 ChildActorComponents（全部會出現在 BP Editor 裡）
	for (int32 i = 0; i < MaxParts; i++)
	{
		FString Name = FString::Printf(TEXT("ChildActor_%d"), i);
		UChildActorComponent* Comp = CreateDefaultSubobject<UChildActorComponent>(*Name);
		Comp->SetupAttachment(RootComponent);

		MeshComponents.Add(Comp);
	}
}