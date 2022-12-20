// Fill out your copyright notice in the Description page of Project Settings.

#include "FishingLure.h"

#include "CustomARPawn.h"
#include "GameplayPlane.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AFishingLure::AFishingLure()
{
	RealLureMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RealLureMeshComponent"));
	RealLureMeshComponent->SetupAttachment(RootComponent);
	RealLureMeshComponent->SetVisibility(false);
	RealLureMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void AFishingLure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Reset catching for the frame
	bIsCatchingAFish = false;

	switch(State)
	{
		case Visualisation:
			VisualisationUpdate();
			break;

		case Casting:
			if (MockCoro_FallingAnimation(DeltaTime))
				State = Floating;

			break;

		case Floating:
			MockCoro_FloatingAnimation(DeltaTime);
			FloatingUpdate(DeltaTime);
			break;
	}
}

void AFishingLure::OnSuddenPlayerMove(const FVector& MovementDelta)
{
	ReelIn();
}

void AFishingLure::StartFalling()
{
	State = Casting;
}

void AFishingLure::ReelIn()
{
	if (State != Floating)
		return;

	if (IsValid(SplashSystem))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SplashSystem, GetActorLocation());

	if (IsValid(SplashSfx))
		UGameplayStatics::PlaySoundAtLocation(this, SplashSfx, GetActorLocation());

	RealLureMeshComponent->SetVisibility(false);
	StaticMeshComponent->SetVisibility(true);
	State = Visualisation;
}

void AFishingLure::CastLure()
{
	if (State != Visualisation || !bIsCastable)
		return;

	bIsCastable = false;
	StaticMeshComponent->SetVisibility(false);
	RealLureMeshComponent->SetVisibility(true);
	State = Casting;
}

bool AFishingLure::MockCoro_FallingAnimation(const float DeltaTime)
{
	auto RelativeLocation = RelativeTransform.GetLocation();

	switch(MockCoro_FallingAnimation_AnimationStep)
	{
		case 0:
			MockCoro_FallingAnimation_FinalRelativePosition = RelativeLocation;
			RelativeLocation.Z += InitialRelativeHeightOffset;
			RelativeTransform.SetLocation(RelativeLocation);
			MockCoro_FallingAnimation_AnimationStep++;
		case 1:
			MockCoro_FallingAnimation_Speed += MockCoro_FallingAnimation_Acceleration * DeltaTime;
			RelativeLocation.Z -= MockCoro_FallingAnimation_Speed * DeltaTime;
			RelativeTransform.SetLocation(RelativeLocation);
			if (RelativeLocation.Z < MockCoro_FallingAnimation_FinalRelativePosition.Z - 10)
			{
				if (IsValid(SplashSystem))
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SplashSystem, GetActorLocation());

				if (IsValid(SplashSfx))
					UGameplayStatics::PlaySoundAtLocation(this, SplashSfx, GetActorLocation());

				MockCoro_FallingAnimation_AnimationStep++;
			}

			break;
		
		case 2:
		{
			auto Delta = MockCoro_FallingAnimation_FinalRelativePosition - RelativeLocation;
			RelativeLocation += Delta * DeltaTime * 20;
			RelativeTransform.SetLocation(RelativeLocation);
			if (fabs(RelativeLocation.Z) - fabs(MockCoro_FallingAnimation_FinalRelativePosition.Z) < 0.1)
			{
				MockCoro_FallingAnimation_Speed = 150;
				MockCoro_FallingAnimation_AnimationStep = 0;
				return true;
			}
		}
	}

	return false;
}

void AFishingLure::MockCoro_FloatingAnimation(const float DeltaTime)
{
	if (!IsValid(StaticMeshComponent))
		return;

	MockCoro_FloatingAnimation_SineInput += DeltaTime * 2;
	float ZOffset = -sinf(MockCoro_FloatingAnimation_SineInput) * FloatingHeightOffset;
	RealLureMeshComponent->SetRelativeLocation(FVector(0, 0, ZOffset));
}

void AFishingLure::VisualisationUpdate()
{
	TArray<FHitResult> TraceResultObj;
	auto Player = Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (!IsValid(Player))
		return;

	const auto StartTracePosition = Player->CameraComponent->GetComponentLocation();
	const auto EndTracePosition = StartTracePosition + Player->CameraComponent->GetForwardVector() * 1000;
	GWorld->LineTraceMultiByChannel(
		TraceResultObj,
		StartTracePosition,
		EndTracePosition,
		ECollisionChannel::ECC_Pawn
		);

	AGameplayPlane* FoundPlane = nullptr;
	FVector HitPosition;

	for(auto It : TraceResultObj)
	{
		if(auto PotentialPlane = Cast<AGameplayPlane>(It.GetActor()))
		{
			FoundPlane = PotentialPlane;
			HitPosition = It.ImpactPoint;
			FloatTowardsPoint = It.ImpactPoint;
			break;
		}
	}

	if (!IsValid(FoundPlane))
	{
		bIsCastable = false;
		StaticMeshComponent->SetVisibility(false);
		return;
	}

	StaticMeshComponent->SetVisibility(true);
	bIsCastable = true;
	SetARPosition(HitPosition);
	auto RelativeLocation = RelativeTransform.GetLocation();
	RelativeLocation.Z -= 8;
	RelativeTransform.SetLocation(RelativeLocation);
}

void AFishingLure::FloatingUpdate(const float DeltaTime)
{
	TArray<FHitResult> TraceResultObj;
	auto Player = Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (!IsValid(Player))
		return;

	const auto StartTracePosition = Player->CameraComponent->GetComponentLocation();
	const auto EndTracePosition = StartTracePosition + Player->CameraComponent->GetForwardVector() * 1000;
	GWorld->LineTraceMultiByChannel(
		TraceResultObj,
		StartTracePosition,
		EndTracePosition,
		ECollisionChannel::ECC_Pawn
	);

	AGameplayPlane* FoundPlane = nullptr;

	for (auto It : TraceResultObj)
	{
		if (auto PotentialPlane = Cast<AGameplayPlane>(It.GetActor()))
		{
			FoundPlane = PotentialPlane;
			FloatTowardsPoint = It.ImpactPoint;
			break;
		}
	}

	auto FloatDirection = FloatTowardsPoint - GetActorLocation();
	float Distance = FloatDirection.Length();
	FloatDirection.Normalize();
	Distance = FMath::Min(Distance, FloatMovingSpeed) * DeltaTime;
	SetARPosition(GetActorLocation() + FloatDirection * Distance);
}
