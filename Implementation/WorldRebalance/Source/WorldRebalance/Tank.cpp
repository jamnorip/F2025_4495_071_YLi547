#include "Tank.h"
#include "Math/UnrealMathUtility.h" // FMath (通常已包含 CoreMinimal 就行)

ATank::ATank()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentHealth = StartingHealth; // 確保執行時 CurrentHealth = StartingHealth
}

float ATank::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 轉為整數傷害
	int32 DamagePoints = FPlatformMath::RoundToInt(DamageAmount);
	// clamp 範圍用整數版本
	int32 DamageToApply = FMath::Clamp(DamagePoints, 0, CurrentHealth);

	CurrentHealth -= DamageToApply;
	if (CurrentHealth <= 0) // tank dead
	{
		CurrentHealth = 0;
		// 這裡可以加入死亡處理（Broadcast、播放音效、Disable 等）
	}
	
	return static_cast<float>(DamageToApply);
}

float ATank::GetHealthPercent() const
{
	// 強制轉為 float 才能做浮點除法
	return (StartingHealth > 0) ? (static_cast<float>(CurrentHealth) / static_cast<float>(StartingHealth)) : 0.0f;
}

float ATank::GetCurrentHealth() const
{
	return static_cast<float>(CurrentHealth);
}
