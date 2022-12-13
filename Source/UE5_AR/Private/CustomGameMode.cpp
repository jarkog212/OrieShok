// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "CustomARPawn.h"
#include "CustomGameState.h"
#include "CustomActor.h"
#include "HelloARManager.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "PlaceableActor.h"
#include "Camera/CameraComponent.h"
#include "CustomUserWidget.h"
#include "FishingPond.h"
#include "Sound/SoundCue.h"

ACustomGameMode::ACustomGameMode() :
	SpawnedPlane(nullptr)
{
	// Add this line to your code if you wish to use the Tick() function
	PrimaryActorTick.bCanEverTick = true;

	// Set the default pawn and game state to be our custom pawn and gamestate programatically
	DefaultPawnClass = ACustomARPawn::StaticClass();
	GameStateClass = ACustomGameState::StaticClass();
}

void ACustomGameMode::AskForLineTrace(const FVector &ScreenPos, TArray<FARTraceResult>& TraceResults, FHitResult& TraceResultObj, FVector& DirectionResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(ScreenPos), WorldPos, DirectionResult);
	// Notice that this AskForLineTrace is in the ARBluePrintLibrary - this means that it's exclusive only for objects tracked by ARKit/ARCore
	TraceResults = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);
	// Line trace used for getting objects within the engine

	const FVector WorldEnd = WorldPos + (DirectionResult * ObjectLineTraceDistance);
	GWorld->LineTraceSingleByChannel(TraceResultObj, WorldPos, WorldEnd, ECollisionChannel::ECC_Pawn);
};

void ACustomGameMode::StartPlay() 
{
	// This is called before BeginPlay
	StartPlayEvent();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Current Money: %d"), GetMoney()));
	DisplayType = EDisplayMode::Intro;

	if (IsValid(UIScreens[DisplayType]))
		CurrentUI = CreateWidget<UCustomUserWidget, UWorld>(GetWorld(), UIScreens[DisplayType]);

	if (IsValid(CurrentUI))
		CurrentUI->AddToViewport();

	// This function will transcend to call BeginPlay on all the actors 
	Super::StartPlay();
}

// An implementation of the StartPlayEvent which can be triggered by calling StartPlayEvent() 
void ACustomGameMode::StartPlayEvent_Implementation() 
{
	// Start a timer which will call the SpawnCube Function every 4 seconds
	// GetWorldTimerManager().SetTimer(Ticker, this, &ACustomGameMode::SpawnCube, 4.0f, true, 0.0f);
}

int32 ACustomGameMode::GetMoney() const
{
	return GetGameState<ACustomGameState>()->Money;
}

void ACustomGameMode::SetMoney(const int32 NewMoney)
{
	GetGameState<ACustomGameState>()->Money = NewMoney;
}

void ACustomGameMode::AddMoney(const int32 AddMoney)
{
	GetGameState<ACustomGameState>()->Money += AddMoney;
}

void ACustomGameMode::SetDisplayType(const TEnumAsByte<EDisplayMode> NewMode)
{
	DisplayType = NewMode;
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaceableActor::StaticClass(), FoundActors);

	for (auto* It : FoundActors)
	{
		auto* FoundActor = IsValid(It) ? Cast<APlaceableActor>(It) : nullptr;

		if (IsValid(FoundActor))
			FoundActor->OnDisplayModeChanged(NewMode);
	}

	if (IsValid(ArManager))
		ArManager->ContinueTrackingAllPlanes();

	if (IsValid(CurrentUI))
	{
		CurrentUI->RemoveFromViewport();
		CurrentUI = nullptr;
	}

	if (!IsValid(UIScreens[DisplayType]))
		return;

	CurrentUI = CreateWidget<UCustomUserWidget, UWorld>(GetWorld(), UIScreens[DisplayType]);
	CurrentUI->AddToViewport();
}

void ACustomGameMode::SetSelectedActor(APlaceableActor* SelectedObj)
{
	// Second condition is needed in case we have deselected object manually beforehand...
	// ...and this call only serves as cleanup.
	if (IsValid(SelectedObject) && SelectedObject->GetIsSelected())
		SelectedObject->Deselect();

	// Can be assigned null
	SelectedObject = SelectedObj;
}

void ACustomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsValid(SpawnedPlane))
		bPlaneDetermined = false;
}

void ACustomGameMode::StartGame()
{
	ArManager = GetWorld()->SpawnActor<AHelloARManager>();
}

void ACustomGameMode::AskForLineTraceSpawnActor(const FARTraceResult &LineTraceHit, const FVector &Direction)
{
	if (!IsValid(GameplayPlanes[DisplayType]))
		return;

	// Get the first found object in the line trace - ignoring the rest of the array elements
	auto* HitPlane = IsValid(LineTraceHit.GetTrackedGeometry()) ? Cast<UARPlaneGeometry>(LineTraceHit.GetTrackedGeometry()) : nullptr;

	if (IsValid(HitPlane) && IsValid(ArManager))
	{
		ArManager->StopTrackingPlanesExcept(HitPlane);
		bPlaneDetermined = true;
	}

	auto TrackedTF = LineTraceHit.GetLocalToWorldTransform();

	if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), Direction) < 0)
	{

		auto* Player = IsValid(UGameplayStatics::GetPlayerPawn(this, 0)) ?
			Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0)) :
			nullptr;

		if(IsValid(Player))
		{
			auto CameraRotation =  Player->CameraComponent->GetComponentRotation();
			CameraRotation.Roll = 0;
			CameraRotation.Pitch = 0;
			TrackedTF.SetRotation(CameraRotation.Quaternion());
		}

		LineTraceHit.GetTrackedGeometry();

		// Spawn the actor pin and get the transform
		UARPin* ActorPin = UARBlueprintLibrary::PinComponent(nullptr, TrackedTF, LineTraceHit.GetTrackedGeometry());

		// Check if ARPins are available on your current device. ARPins are currently not supported locally by ARKit, so on iOS, this will always be "FALSE" 
		if (IsValid(ActorPin))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("ARPin is valid"));
			
			// If the pin is valid 
			auto PinTF = ActorPin->GetLocalToWorldTransform();

			// Spawn a new Actor at the location if not done yet
			if (!IsValid(SpawnedPlane))
			{
				const FActorSpawnParameters SpawnInfo;
				const FRotator MyRot(0, 0, 0);
				const FVector MyLoc(0, 0, 0);

				SpawnedPlane = GetWorld()->SpawnActor<AGameplayPlane>(GameplayPlanes[DisplayType], MyLoc, MyRot, SpawnInfo);
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Spawning BP"));
				
				if (!IsValid(SpawnedPlane))
					return;

				// Set the spawned actor location based on the Pin. Have a look at the code for Placeable Object to see how it handles the AR PIN passed on
				SpawnedPlane->SetActorTransform(PinTF);
				SpawnedPlane->PinComponent = ActorPin;
				ActorPin->AddToRoot();

				if (IsValid(PlaneSpawnSfx))
					UGameplayStatics::PlaySoundAtLocation(this, PlaneSpawnSfx, MyLoc);

				// DisplayType enum -> int32
				if (IsValid(Player))
					Player->SwitchBgm(DisplayType);
			}
		}
		// IF ARPins are Not supported locally (for iOS Devices) We will spawn the object in the location where the line trace has hit
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ARPin is invalid"));

			// Spawn a new Actor at the location if not done yet
			if (!IsValid(SpawnedPlane))
			{
				const FActorSpawnParameters SpawnInfo;
				const FRotator MyRot(0, 0, 0);
				const FVector MyLoc(0, 0, 0);
				SpawnedPlane = GetWorld()->SpawnActor<AGameplayPlane>(GameplayPlanes[DisplayType], MyLoc, MyRot, SpawnInfo);
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Spawning BP"));

				if (!IsValid(SpawnedPlane))
					return;

				SpawnedPlane->SetActorTransform(TrackedTF);
				SpawnedPlane->SetActorScale3D(FVector(0.2, 0.2, 0.2));
			}
		}
	}
}

void ACustomGameMode::StoreLayoutData(TArray<FLayoutData> &LayoutData)
{
	StoredLayoutData = LayoutData;
}

void ACustomGameMode::HandleGeneralLineTraceResult(const FARTraceResult& LineTraceHit, const FVector& Direction)
{
	if (bPlaneDetermined)
	{
		switch (DisplayType)
		{
			case Default:
				if (IsValid(GetSelectedActor()))
					GetSelectedActor()->Deselect();

				break;

			case Fishing:
			default:
				break;
		}
	}
	else
	{
		AskForLineTraceSpawnActor(LineTraceHit, Direction);
	}
}

