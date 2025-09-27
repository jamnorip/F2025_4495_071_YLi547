#include "MovableActor.h"

// Sets default values
AMovableActor::AMovableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	SetRootComponent(RootComp);

	Point1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point1"));
	Point1->SetupAttachment(RootComp);
	Point1->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	
	Point2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Point2"));
	Point2->SetupAttachment(RootComp);
	Point2->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComp);
	Mesh->SetIsReplicated(true);

	Transporter = CreateDefaultSubobject<UTransporter>(TEXT("Transporter"));
}

// Called when the game starts or when spawned
void AMovableActor::BeginPlay()
{
	Super::BeginPlay();

	//FVector StartPoint = GetActorLocation() + Point1->GetRelativeLocation();
	//FVector EndPoint = GetActorLocation() + Point2->GetRelativeLocation();
	FVector StartPoint = Point1 ? Point1->GetComponentLocation() : GetActorLocation();
	FVector EndPoint   = Point2 ? Point2->GetComponentLocation() : GetActorLocation();
	Transporter->SetPoints(StartPoint, EndPoint);
}

// Called every frame
void AMovableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

