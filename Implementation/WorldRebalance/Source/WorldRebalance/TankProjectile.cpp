// Fill out your copyright notice in the Description page of Project Settings.

#include "TankProjectile.h"

// 在 .cpp 裡 include 真正需要的 header（完整定義）
#include "PC_Trooper.h"
#include "Tank.h"
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
	// 在 constructor
	bReplicates = true;
	SetReplicateMovement(true);
	// 假设你有一个 UProjectileMovementComponent* ProjectileMovement;
	if (ProjectileMovement)
	{
		ProjectileMovement->SetIsReplicated(true); // 让 Movement Component 在网络上复制（如果存在）
	}

	CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("CollisionMesh"));
	SetRootComponent(CollisionMesh);
	CollisionMesh->SetNotifyRigidBodyCollision(true);
	CollisionMesh->SetCollisionProfileName(TEXT("BlockAllDynamic")); // 或使用專門的 Projectile profile
	CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionMesh->SetVisibility(false);
	CollisionMesh->SetIsReplicated(true);

	LaunchBlast = CreateDefaultSubobject<UParticleSystemComponent>(FName("LaunchBlast"));
	LaunchBlast->SetupAttachment(RootComponent);
	LaunchBlast->SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("Projectile Movement"));
	ProjectileMovement->bAutoActivate = false;
	ProjectileMovement->SetIsReplicated(true);

	ImpactBlast = CreateDefaultSubobject<UParticleSystemComponent>(FName("ImpactBlast"));
	ImpactBlast->SetupAttachment(RootComponent);
	ImpactBlast->bAutoActivate = false;
	ImpactBlast->SetIsReplicated(true);

	ExplosionForce = CreateDefaultSubobject<URadialForceComponent>(FName("ExplosionForce"));
	ExplosionForce->SetupAttachment(RootComponent);
	ExplosionForce->SetIsReplicated(true);
	// URadialForceComponent 不需要 bAutoActivate，FireImpulse 直接呼叫即可

	// 设置网络更新频率
	NetUpdateFrequency = 100.0f; // 较高的更新频率用于快速移动的物体
	MinNetUpdateFrequency = 20.0f;
}

// Called when the game starts or when spawned
void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();
	// 只在服务器上绑定碰撞事件
	if (HasAuthority())
	{
		CollisionMesh->OnComponentHit.AddDynamic(this, &ATankProjectile::OnHit);
	}
}

void ATankProjectile::LaunchProjectile(float Speed)
{
	if (!ensure(ProjectileMovement)) return;
	ProjectileMovement->Activate();
	ProjectileMovement->SetVelocityInLocalSpace(FVector::ForwardVector * Speed);
}



void ATankProjectile::ProcessDamageAndStats(AActor* HitTankActor)
{
	if (!HasAuthority()) return; // server only

	ATank* HitTank = Cast<ATank>(HitTankActor);
	if (!HitTank) return;

	float CurrentHealth = HitTank->GetCurrentHealth();
	if (CurrentHealth <= 0.f) return;

	ATank* FiringTank = GetFiringTank();
	if (!FiringTank) return;

	AController* OwnerController = FiringTank->GetController();
	APC_Trooper* PlayerController = OwnerController ? Cast<APC_Trooper>(OwnerController) : nullptr;
	if (PlayerController)
	{
		int32 DamageToAdd = FMath::TruncToInt(ProjectileDamage);
		PlayerController->AddDamageDealt(DamageToAdd);
	}

	if ((CurrentHealth - ProjectileDamage) <= 0.f)
	{
		if (PlayerController)
		{
			PlayerController->AddKillCount(1);
		}
	}
}


bool ATankProjectile::WillTankBeKilled(float CurrentHealth) const
{
	// 对应蓝图中的 Subtract 和 LessEqual 节点
	// 计算：CurrentHealth - ProjectileDamage <= 0
	return (CurrentHealth - ProjectileDamage) <= 0.0f;
}

ATank* ATankProjectile::GetFiringTank() const
{
	// 获取发射者（对应蓝图中的 GetOwner 和 Cast To BP_TankBase）
	return Cast<ATank>(GetOwner());
}

APC_Trooper* ATankProjectile::GetPlayerControllerFromTank(ATank* Tank) const
{
	if (!Tank) return nullptr;
    
	// 这里需要根据您的实际实现来获取玩家控制器
	// 假设坦克有一个方法可以获取其玩家控制器
	return Tank->GetTankPlayerController();
    
	// 或者使用另一种方法：
	//return Cast<APlayerController>(Tank->GetController());
}


void ATankProjectile::Multicast_OnHit_Implementation()
{
	// 在所有客户端上播放视觉效果
	LaunchBlast->Deactivate();
	ImpactBlast->Activate();
	if (ExplosionForce) 
	{
		ExplosionForce->FireImpulse();
	}
    
	// 更换根组件（在所有客户端执行）
	SetRootComponent(ImpactBlast);
	if (CollisionMesh)
	{
		CollisionMesh->DestroyComponent();
	}
}


void ATankProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
                            FVector NormalImpulse, const FHitResult& Hit)
{
	// 服务器处理伤害逻辑
	if (HasAuthority())
	{
		// damage - 只在服务器执行
		UGameplayStatics::ApplyRadialDamage(
			this, 
			ProjectileDamage, 
			GetActorLocation(), 
			ExplosionForce ? ExplosionForce->Radius : 0.f, 
			UDamageType::StaticClass(), 
			TArray<AActor*>()
		);
		
		// 处理伤害计算和统计数据
		if (OtherActor)
		{
			ProcessDamageAndStats(OtherActor);
		}
		
		// 通知所有客户端播放视觉效果
		Multicast_OnHit();
        
		// 只在服务器设置销毁计时器
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, this, &ATankProjectile::OnTimerExpire, DestroyDelay, false);
	}
}


void ATankProjectile::OnTimerExpire()
{
	Destroy();
}
