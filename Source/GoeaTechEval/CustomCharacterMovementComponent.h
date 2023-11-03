// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class GOEATECHEVAL_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	bool IsClimbing() const;

	UFUNCTION(BlueprintPure)
	FVector GetClimbSurfaceNormal() const;

	UFUNCTION(BlueprintPure)
	FVector GetClimbSurfacePosition() const;

	void TryClimbing();

	void CancelClimbing();

private:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	
	/* shape sweep to detect object */
	void SweepAndStoreWallHits();

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleRadius = 50;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere)
	int CollisionCapsuleHalfHeight = 72;

	/* conditions to enable climbing */
	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "1.0", ClampMax = "75.0"));
	float MaxHorizontalDegreesToStartClimbing = 30.f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "80.0"))
	float ClimbingCollisionShrinkAmount = 30.f;


	

	/* line-trace forward at eye height to confirm climbable surface */
	bool EyeHeightTrace(const float TraceDistance) const;

	bool IsFacingSurface(const float) const;

	bool bWantsToClimb = false;

	TArray<FHitResult> CurrentWallHits;

	FCollisionQueryParams ClimbQueryParams;



	/** Members for calculating climbing physics **/
	FVector CurrentClimbingNormal;
	FVector CurrentClimbingPosition;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float MaxClimbingSpeed = 120.f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "2000.0"))
	float MaxClimbingAcceleration = 380.f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "3000.0"))
	float BrakingDecelerationClimbing = 550.f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "12.0"))
	int ClimbingRotationSpeed = 6;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "60.0"))
	float ClimbingSnapSpeed = 4.f;

	UPROPERTY(Category = "Character Movement: Climbing", EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "80.0"))
	float DistanceFromSurface = 45.f;

	void PhysClimbing(float deltaTime, int32 Iterations);

	/* calc current climbing normal and climbing position */
	void ComputeSurfaceInfo();

	void ComputeClimbingVelocity(float deltaTime);

	bool ShouldStopClimbing();

	void StopClimbing(float deltaTime, int32 Iterations);

	/* move along climbing surface considering velocity and rotation */
	void MoveAlongClimbingSurface(float deltaTime);

	/* move component torwards the climbing surface */
	void SnapToClimbingSurface(float deltaTime) const;

	FQuat GetClimbingRotation(float deltaTime) const;

	virtual float GetMaxSpeed() const override;

	virtual float GetMaxAcceleration() const override;

public:
		bool CanStartClimbing();

};
