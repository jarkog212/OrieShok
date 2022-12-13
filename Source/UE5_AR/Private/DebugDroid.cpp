// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugDroid.h"
#include "ARPin.h"
#include "ARTypes.h"
#include "CustomARPawn.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"

void ADebugDroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(PinComponent) && PinComponent->GetTrackingState() == EARTrackingState::Tracking)
	{
		// Camera Distance gathering
		double DistanceFromCamera = 0.0f;
		if (const auto* Player = dynamic_cast<ACustomARPawn*>(GWorld->GetFirstPlayerController()->GetPawn()))
			DistanceFromCamera = UKismetMathLibrary::Vector4_Size(GetActorLocation() - Player->CameraComponent->GetComponentLocation());
		else
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Failed Cast to Player (APlaceableActor::Tick())"));

		// Changing colour based on the distance
		if (DistanceFromCamera > MaxDistance)
			ActualMaterial->SetVectorParameterValue("BaseColor", FarColour);
		else
			ActualMaterial->SetVectorParameterValue("BaseColor", CloseColour);
	}
}
