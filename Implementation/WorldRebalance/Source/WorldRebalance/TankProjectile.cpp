// Fill out your copyright notice in the Description page of Project Settings.

#include "TankProjectile.h"

// 在 .cpp 裡 include 真正需要的 header（完整定義）
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "TimerManager.h"

// Sets default values
ATankProjectile::ATankProjectile()
{
 	PrimaryActorTick.bCanEverTick = false;

	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("CollisionMesh"));
	SetRootComponent(CollisionMesh);
	CollisionMesh->SetNotifyRigidBodyCollision(true);
	CollisionMesh->SetVisibility(false);

	LaunchBlast = CreateDefaultSubobject<UParticleSystemComponent>(FName("LaunchBlast"));
	LaunchBlast->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile Movement"));
	ProjectileMovement->bAutoActivate = false;

	ImpactBlast = CreateDefaultSubobject<UParticleSystemComponent>(FName("ImpactBlast"));
	ImpactBlast->SetupAttachment(RootComponent);
	ImpactBlast->bAutoActivate = false;

	ExplosionForce = CreateDefaultSubobject<URadialForceComponent>(FName("ExplosionForce"));
	ExplosionForce->SetupAttachment(RootComponent);
	// URadialForceComponent 不需要 bAutoActivate，FireImpulse 直接呼叫即可
}

// Called when the game starts or when spawned
void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionMesh->OnComponentHit.AddDynamic(this, &ATankProjectile::OnHit);
}

void ATankProjectile::LaunchProjectile(float Speed)
{
	if (!ensure(ProjectileMovement)) return;
	ProjectileMovement->Activate();
	ProjectileMovement->SetVelocityInLocalSpace(FVector::ForwardVector * Speed);
}

void ATankProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
                           FVector NormalImpulse, const FHitResult& Hit)
{
	LaunchBlast->Deactivate();
	ImpactBlast->Activate();
	if (ExplosionForce) ExplosionForce->FireImpulse();
	
	// 換根 component（注意：確保 ImpactBlast 是能被當作 Root 的 component）
	SetRootComponent(ImpactBlast);
	if (CollisionMesh)
	{
		CollisionMesh->DestroyComponent();
	}

	// damage
	UGameplayStatics::ApplyRadialDamage(this, ProjectileDamage, GetActorLocation(), ExplosionForce ? ExplosionForce->Radius : 0.f, UDamageType::StaticClass(), TArray<AActor*>());

	// destroy this projectile actor after delay
	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, this, &ATankProjectile::OnTimerExpire, DestroyDelay, false);
}

void ATankProjectile::OnTimerExpire()
{
	Destroy();
}
