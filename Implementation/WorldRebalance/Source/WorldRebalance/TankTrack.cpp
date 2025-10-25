// Fill out your copyright notice in the Description page of Project Settings.


#include "TankTrack.h"

UTankTrack::UTankTrack()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 确保本组件会通知刚体碰撞（等同于在编辑器勾选 Simulation Generates Hit Events）
	//SetNotifyRigidBodyCollision(true);

	// 确保碰撞启用为 QueryAndPhysics（如果你在蓝图里已有设置，可以不重复）
	//SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UTankTrack::BeginPlay()
{
	Super::BeginPlay();
	//OnComponentHit.AddDynamic(this, &UTankTrack::OnHit);
}

/*
void UTankTrack::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("Hit"));
}
*/

void UTankTrack::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//DriveTrack();
	
	// 1) 确保 root 是物理模拟组件
	UPrimitiveComponent* TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if (!TankRoot || !TankRoot->IsSimulatingPhysics())
	{
		return;
	}

	// 2) 用 root 的向量与 root 的速度来计算侧向滑动速度（避免使用 track 自己的向量/速度）
	FVector RootRight = TankRoot->GetRightVector();
	FVector RootVel = TankRoot->GetComponentVelocity();
	float SlippageSpeed = FVector::DotProduct(RootRight, RootVel);

	// 3) 需要的加速度来修正侧滑（保留符号）
	FVector CorrectionAcceleration = (-SlippageSpeed / DeltaTime) * RootRight;

	// 4) 计算力（两个履带分摊）
	FVector CorrectionForce = (TankRoot->GetMass() * CorrectionAcceleration) / 2.0f;

	// 5) 为避免过大瞬时力导致旋转，做一个上限（按经验值或根据质量调）
	const float MaxCorrectionForce = 1e6f; // 例值，根据你的坦克质量/规模调整
	if (CorrectionForce.Size() > MaxCorrectionForce)
	{
		CorrectionForce = CorrectionForce.GetClampedToMaxSize(MaxCorrectionForce);
	}

	// 6) 将力施加到 root（在质心处施加，尽量用 AddForce 而非 AddForceAtLocation）
	TankRoot->AddForce(CorrectionForce);
}

void UTankTrack::SetThrottle(float Throttle)
{
	//CurrentThrottle = FMath::Clamp<float>(CurrentThrottle + Throttle, -1, 1);
	//DriveTrack();
	auto ForceApplied = GetForwardVector() * Throttle * TrackMaxDrivingForce;
	auto ForceLocation = GetComponentLocation();
	auto TankRoot = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	TankRoot->AddForceAtLocation(ForceApplied, ForceLocation);
}
