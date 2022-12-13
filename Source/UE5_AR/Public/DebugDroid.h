// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlaceableActor.h"
#include "DebugDroid.generated.h"

//! @brief Class representing the debug android figure, mesh is assigned in blueprint child class
UCLASS()
class UE5_AR_API ADebugDroid : public APlaceableActor
{
	GENERATED_BODY()

public:

	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	//! Colour of the material when the object is close
	UPROPERTY(Category = "Debug Droid Colours", EditAnywhere)
		FVector4f CloseColour = FVector4f(1, 0, 0, 1);

	//! Colour of the material when the object is far
	UPROPERTY(Category = "Debug Droid Colours", EditAnywhere)
		FVector4f FarColour = FVector4f(1, 1, 1, 1);

	//! Distance in cm at which the colour changes
	UPROPERTY(Category = "Debug Droid Colours", EditAnywhere, BlueprintReadWrite)
		double MaxDistance = 100;
};
