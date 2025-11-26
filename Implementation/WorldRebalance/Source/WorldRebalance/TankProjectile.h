// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PC_Trooper.h"
#include "Tank.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

// Forward declarations（把會在 header 中出現的 UClass 用 forward declare）
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;
class URadialForceComponent;
class UPrimitiveComponent;

UCLASS()
class WORLDREBALANCE_API ATankProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATankProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void LaunchProjectile(float Speed);

	UPROPERTY(EditDefaultsOnly, Category="Setup")
	float ProjectileDamage = 21.0f;

private:
	// 处理伤害计算和统计的核心函数
	void ProcessDamageAndStats(AActor* HitTankActor);
	bool WillTankBeKilled(float CurrentHealth) const;
	ATank* GetFiringTank() const;
	// 获取玩家控制器
	APC_Trooper* GetPlayerControllerFromTank(ATank* Tank) const;
	

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
			   FVector NormalImpulse, const FHitResult& Hit);
    
	// 添加网络复制函数
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnHit();
    
	void OnTimerExpire();


	
	UPROPERTY(EditDefaultsOnly, Category="Setup")
	float DestroyDelay = 2.5f;

	// forward-declared pointer members
	UProjectileMovementComponent* ProjectileMovement = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* CollisionMesh = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UParticleSystemComponent* LaunchBlast = nullptr;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UParticleSystemComponent* ImpactBlast = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* ExplosionForce = nullptr;
};
