#include "PC_Trooper.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

APC_Trooper::APC_Trooper()
{
	DamageDealt = 0;
	KillCount = 0;

	// PlayerController 通常由引擎管理複製，但確保需要的行為在 server 上改變
	bReplicates = true;
}

void APC_Trooper::AddDamageDealt(int32 Damage)
{
	if (!HasAuthority()) return; // 只有 server 修改
	DamageDealt += Damage;
	// (可選) 直接在 server log
	UE_LOG(LogTemp, Log, TEXT("Server AddDamageDealt: %d -> %d"), Damage, DamageDealt);
}

void APC_Trooper::AddKillCount(int32 KillNumber)
{
	if (!HasAuthority()) return;
	KillCount += KillNumber;
	UE_LOG(LogTemp, Log, TEXT("Server AddKillCount: %d -> %d"), KillNumber, KillCount);
}

int32 APC_Trooper::GetDamageDealt() const
{
	return DamageDealt;
}

int32 APC_Trooper::GetKillCount() const
{
	return KillCount;
}

// RepNotify callbacks: 在 client 上被呼叫，適合更新 HUD 或顯示 debug
void APC_Trooper::OnRep_DamageDealt()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("DamageDealt (replicated): %d"), DamageDealt));
	}
}

void APC_Trooper::OnRep_KillCount()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("KillCount (replicated): %d"), KillCount));
	}
}

void APC_Trooper::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APC_Trooper, DamageDealt);
	DOREPLIFETIME(APC_Trooper, KillCount);
}
