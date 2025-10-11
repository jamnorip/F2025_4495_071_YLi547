// Fill out your copyright notice in the Description page of Project Settings.


#include "CollectableKeyHolder.h"

// Sets default values
ACollectableKeyHolder::ACollectableKeyHolder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(RootComp);
	KeyMesh->SetIsReplicated(true);
	KeyMesh->SetCollisionProfileName(FName("OverlapAllDynamic"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);
	Mesh->SetCollisionProfileName(FName("BlockAllDynamic"));

	KeyMeshRotationSpeed = 100.0f;
}

// Called when the game starts or when spawned
void ACollectableKeyHolder::BeginPlay()
{
	Super::BeginPlay();

	KeyMesh->SetVisibility(false);
}

// Called every frame
void ACollectableKeyHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	KeyMesh->AddRelativeRotation(FRotator(0, KeyMeshRotationSpeed* DeltaTime, 0));
}

void ACollectableKeyHolder::ActivateKeyMesh()
{
	KeyMesh->SetVisibility(true);
}

