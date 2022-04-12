// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RPG_RoguelikeCharacter.generated.h"

UCLASS(config=Game)
class ARPG_RoguelikeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ARPG_RoguelikeCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float currentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float maxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana")
	float maxMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana")
	float currentMana;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool isStunned;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool isAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	int attackType;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float damageAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamagePercent(float damagePercent);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void HealAmount(float healAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void HealPercent(float healPercent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainCharAnimMontages")
	UAnimMontage* M_lightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainCharAnimMontages")
	UAnimMontage* M_heavyAttack;
protected:
	

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	//Allows the chatracter to start sprinting
	void Sprint();

	//Stops the character from sprinting
	void StopSprinting();


	void AttackLight();

	void AttackHeavy();

	void AttackCombo();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void EndAttack();

	bool CanMove();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

