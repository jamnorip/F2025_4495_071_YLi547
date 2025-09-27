// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"

// Sets default values
APressurePlate::APressurePlate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Activated = false;
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);
	
	//Trigger
	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(RootComp);//TriggerMesh attach to the RootComp
	TriggerMesh->SetIsReplicated(true);
	
	auto TriggerMeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder.Shape_Cylinder"));
	if (TriggerMeshAsset.Succeeded())
	{
		TriggerMesh->SetStaticMesh(TriggerMeshAsset.Object);
		TriggerMesh->SetRelativeScale3D(FVector(3.3f, 3.3f, 0.3f));
		TriggerMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));
	}
	
	//Mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);//TriggerMesh attach to the RootComp
	Mesh->SetIsReplicated(true);

	auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/StarterContent/Shapes/Shape_Cylinder.Shape_Cylinder"));
	if (MeshAsset.Succeeded())
	{
		Mesh->SetStaticMesh(MeshAsset.Object);
		Mesh->SetRelativeScale3D(FVector(3.3f, 3.3f, 0.3f));
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -21.0f));
	}

	Transporter = CreateDefaultSubobject<UTransporter>(TEXT("Transporter"));
	Transporter->MoveTime = 0.25f;
	Transporter->OwnerIsTriggerActor = true;
}

// Called when the game starts or when spawned
void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerMesh->SetVisibility(false);
	TriggerMesh->SetCollisionProfileName(FName("OverlapAll"));

	FVector Point1 = GetActorLocation();
	FVector Point2 = Point1 + FVector(0.0f, 0.0f, -20.0f);
	Transporter->SetPoints(Point1, Point2);
}

// Called every frame
void APressurePlate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		TArray<AActor*> OverlappingActors;
		AActor* TriggerActor = 0;
		TriggerMesh->GetOverlappingActors(OverlappingActors);

		for (int ActorIdx = 0; ActorIdx < OverlappingActors.Num(); ++ActorIdx)
		{
			AActor* A = OverlappingActors[ActorIdx];

			//FString Msg = FString::Printf(TEXT("Overlap Actor: %s"), *A->GetName());
			//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White, Msg);
			if (A->ActorHasTag("TriggerActor"))
			{
				TriggerActor = A;
				break;
			}
		}

		if (TriggerActor)
		{
			if (!Activated)
			{
				Activated = true;
				//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White, TEXT("Activated"));
				OnActivated.Broadcast();
			}
		}
		else
		{
			if (Activated)
			{
				Activated = false;
				//GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White, TEXT("Deactivated"));
				OnDeactivated.Broadcast();
			}
		}
	}
	
}

