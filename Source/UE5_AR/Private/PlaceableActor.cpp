// Fill out your copyright notice in the Description page of Project Settings.

#include "PlaceableActor.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "CustomARPawn.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"

// Sets default values
APlaceableActor::APlaceableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(SceneComponent);
}

APlaceableActor::~APlaceableActor()
{
	if (!bIsSelected)
		return;

	bIsSelected = false;

	auto* GM = IsValid(GetWorld()) ?
		Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode()) :
		nullptr;

	if (IsValid(GM))
		GM->SetSelectedActor(nullptr);
}

// Called when the game starts or when spawned
void APlaceableActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(SpawnPuff))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SpawnPuff, GetActorLocation());

	if (IsValid(Mesh))
		StaticMeshComponent->SetStaticMesh(Mesh);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("No Mesh (APlaceableActor::BeginPlay())"));

	if (IsValid(Material))
		ActualMaterial = UMaterialInstanceDynamic::Create(Material, NULL);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("No Material (APlaceableActor::BeginPlay())"));

	if (IsValid(ActualMaterial))
		StaticMeshComponent->SetMaterial(0, ActualMaterial);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("No Dynamic material (APlaceableActor::BeginPlay())"));

	if (IsValid(SpawnPuff))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, 
			SpawnPuff, 
			GetActorLocation(),
			FRotator(0, 0, 0),
			StaticMeshComponent->GetComponentScale()
		);
}

// Called every frame
void APlaceableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Making sure the actor remains on the ARPin that has been found.
	if (IsValid(PinComponent))
	{
		auto TrackingState = PinComponent->GetTrackingState();
		switch (TrackingState)
		{
			case EARTrackingState::Tracking:
			{
				SceneComponent->SetVisibility(true);
				SetActorTransform(RelativeTransform * PinComponent->GetLocalToWorldTransform());
				// Scale down default cube mesh - Change this for your applications.
				//SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
				break;
			}
			case EARTrackingState::NotTracking:
				PinComponent = nullptr;
				break;
		}
	}

	if (bIsUIMember)
		UIMemberUpdate(DeltaTime);
}

void APlaceableActor::OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode)
{
	GWorld->DestroyActor(this);
}

void APlaceableActor::Select()
{
	if (bIsSelected)
		return;

	if (IsValid(ActualMaterial))
		ActualMaterial->SetScalarParameterValue("IsSelected", 1.0f);

	bIsSelected = true;

	auto* GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	if (IsValid(GM))
		GM->SetSelectedActor(this);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("APlaceableActor::Select(): No GM "));
}

void APlaceableActor::Deselect()
{
	if (!bIsSelected)
		return;

	if (IsValid(ActualMaterial))
		ActualMaterial->SetScalarParameterValue("IsSelected", 0.0f);

	bIsSelected = false;

	auto* GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());

	//!! Potential for infinite loop / recursion here as SetSelectedActor can call this function
	//It is needed for cleanup purposes therefore the bIsSelected flag acts as the break of the loop, recursion
	if (IsValid(GM))
		GM->SetSelectedActor(nullptr);
	else
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("APlaceableActor::Deselect(): No GM "));
}

void APlaceableActor::OnTouched(const FVector &TouchPositionWorld)
{
	if (!bIsSelected)
		Select();
	else
		Deselect();
}

void APlaceableActor::SetARPosition(const FVector& WorldPosition)
{
	if (IsValid(PinComponent))
	{
		FTransform AbsoluteTransform;
		AbsoluteTransform.SetLocation(WorldPosition);
		RelativeTransform = AbsoluteTransform * PinComponent->GetLocalToWorldTransform().Inverse();
	}
	else
	{
		SetActorLocation(WorldPosition);
	}
}

void APlaceableActor::SetAsUIMember(const bool State, ACustomARPawn* CurrentUIPlayer, const FVector RelativeCameraOffset)
{
	if (State == bIsUIMember || (State && !IsValid(CurrentUIPlayer)) || (!State && !IsValid(UIPlayer)))
		return;

	bIsUIMember = State;
	if (bIsUIMember)
	{
		UIObjectCameraOffset = RelativeCameraOffset;
		CurrentUIPlayer->UIMembers.AddUnique(this);
		UIPlayer = CurrentUIPlayer;
	}
	else
	{
		UIPlayer->UIMembers.RemoveSingle(this);
		UIPlayer = nullptr;
		RelativeTransform.SetRotation(FRotator().Quaternion());
	}
}

void APlaceableActor::UIMemberUpdate(const float DeltaTime)
{
	if (!UIPlayer)
		return;

	// Rotate around in camera relative space
	auto PlacingActorRotation = RelativeTransform.GetRotation().Rotator();
	PlacingActorRotation.Yaw += DeltaTime * PlacingActorAngularVelocity;
	RelativeTransform.SetRotation(PlacingActorRotation.Quaternion());
	RelativeTransform.SetLocation(UIObjectCameraOffset);

	// Apply relative to the camera
	const auto& CameraTransform = UIPlayer->CameraComponent->GetComponentTransform();
	SetActorTransform(RelativeTransform * CameraTransform);
	SetActorScale3D(FVector(0.25, 0.25, 0.25));
}
