#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Trooper.generated.h"

UCLASS()
class WORLDREBALANCE_API APC_Trooper : public APlayerController
{
	GENERATED_BODY()

public:
	APC_Trooper();

	// Replicated properties
	UPROPERTY(ReplicatedUsing=OnRep_DamageDealt, BlueprintReadOnly, Category="Stats")
	int32 DamageDealt;

	UPROPERTY(ReplicatedUsing=OnRep_KillCount, BlueprintReadOnly, Category="Stats")
	int32 KillCount;

	// Server-side setters (call on server)
	UFUNCTION()
	void AddDamageDealt(int32 Damage);

	UFUNCTION()
	void AddKillCount(int32 KillNumber);

	// Pure getters (no exec pin)
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetDamageDealt() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetKillCount() const;

protected:
	// RepNotify callbacks
	UFUNCTION()
	void OnRep_DamageDealt();

	UFUNCTION()
	void OnRep_KillCount();

	// Replication setup
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
