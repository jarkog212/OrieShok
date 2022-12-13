// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "HelloARManager.generated.h"

class UARSessionConfig;
class AARPlaneActor;
class UARPlaneGeometry;

//! @brief Class handling the AR framework and trackable objects
UCLASS()
class UE5_AR_API AHelloARManager : public AActor
{
	GENERATED_BODY()
	
public:

	//! Root Component
	UPROPERTY(Category = "SceneComp", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	// Sets default values for this actor's properties
	AHelloARManager();

protected:

	//! @brief Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	
	//! @brief Called every frame
	//! @param DeltaTime - time difference between frames.
	virtual void Tick(float DeltaTime) override;

	//! @brief Function that stops tracking most of the planes and turns off plane searching
	//! One plane, conncted to the pin is left in tracking mode
	//! @param excetpion - The plane whose tracking needs to continue for the AR Pin
	UFUNCTION(BlueprintCallable, Category = "ARSession")
		void StopTrackingPlanesExcept(UARPlaneGeometry* exception);

	//! @brief Function that re-enables the plane searching by restarting the AR session
	//! Cleans out all the old planes
	UFUNCTION(BlueprintCallable, Category = "ARSession")
		void ContinueTrackingAllPlanes();

protected:
	
	//! @brief Updates the plane actors on every frame as long as the AR Session is running
	void UpdatePlaneActors();

	//! @brief Function used to spawn a new AR plane actor representation
	//! @returns [value] - Pointer to the spawned plane actor
	AARPlaneActor* SpawnPlaneActor();

	//! @brief Returns a colour from a list
	//! @param Index - index of the colour within the list
	//! @returns [value] - Colour from the list
	FColor GetPlaneColor(int Index);

	//! @brief Function that resets all containers before starting a new AR session
	//! Removes/destroys planes
	void ResetARCoreSession();

	//! Configuration file for AR Session
	UARSessionConfig* Config;

	//! Base plane actor for geometry detection
	AARPlaneActor* PlaneActor;

	//! Map of geometry planes
	TMap<UARPlaneGeometry*, AARPlaneActor*> PlaneActors;

	//! Index for plane colours and array of colours
	int PlaneIndex = 0;

	//! Array of colours fo the planes
	TArray<FColor> PlaneColors;

};
