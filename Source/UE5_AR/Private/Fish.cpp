// Fill out your copyright notice in the Description page of Project Settings.

#include "Fish.h"
#include "ARPin.h"
#include "CustomARPawn.h"
#include "FishingPond.h"
#include "FishingLure.h"
#include "Camera/CameraComponent.h"
#include "CustomUserWidget.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AFish::AFish()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RealFishMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FishMeshComponent"));
	RealFishMeshComponent->SetupAttachment(RootComponent);
	RealFishMeshComponent->SetVisibility(false);
	RealFishMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	SwimmingCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SwimmingCollider"));
	SwimmingCollider->SetupAttachment(RootComponent);
	SwimmingCollider->OnComponentBeginOverlap.AddDynamic(this, &AFish::OnColliderEnter);
	SwimmingCollider->OnComponentEndOverlap.AddDynamic(this, &AFish::OnColliderLeave);

	RelativeTransform.SetScale3D(FVector(ScaleWidth,  ScaleHeight,  1));
}

void AFish::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(FishMesh) && IsValid(FishMesh->GetMaterial(0)))
		FishActualMaterial = UMaterialInstanceDynamic::Create(FishMesh->GetMaterial(0), NULL);

	if (IsValid(StaticMeshComponent))
		StaticMeshComponent->SetWorldScale3D(FVector(ScaleHeight, ScaleWidth, 1.f));
}

void AFish::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(LureInVicinity))
	{
		auto* Player = UGameplayStatics::GetPlayerController(this, 0);
		if (IsValid(Player))
		{
			UGameplayStatics::GetPlayerController(this, 0)->PlayDynamicForceFeedback(
				0.3,
				0.1,
				true,
				true,
				true,
				true,
				EDynamicForceFeedbackAction::Start
			);
		}
	}
	
	switch(State)
	{
		case Entering:
			if (MockCoro_FadeInAnimation(DeltaTime))
			{
				State = Swimming;
			}
		case Swimming:
			MoveTowardsInterest(DeltaTime);
			MockCoro_ConsiderChangingTarget(DeltaTime);
			if (Iterations > IterationLifespan)
				State = Leaving;

			break;

		case Leaving:
			MoveTowardsInterest(DeltaTime);
		case Escaped:
			if (MockCoro_FadeOutAnimation(DeltaTime))
			{
				const auto* GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
				auto* Pond = IsValid(GM) ? Cast<AFishingPond>(GM->GetGameplayPlane()) : nullptr;

				if (IsValid(Pond))
					Pond->RemoveFish(this);

				GWorld->DestroyActor(this);
			}

			return;

		case ReelingIn:
			if (ShouldEscape())
				State = Escaped;
			else if (MockCoro_ReelInAnimation(DeltaTime))
			{
				State = Caught;
				const auto* GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
				auto* Pond = IsValid(GM) ? Cast<AFishingPond>(GM->GetGameplayPlane()) : nullptr;

				if (IsValid(Pond))
						Pond->RemoveFish(this);

				if (IsValid(LureInVicinity))
				{
					LureInVicinity = nullptr;
				}
			}

			break;
	}
}

void AFish::OnTouched(const FVector& TouchPositionWorld)
{
	if (State == Caught)
		Super::OnTouched(TouchPositionWorld);
}

void AFish::OnDisplayModeChanged(const TEnumAsByte<EDisplayMode> NewMode)
{
	switch (State)
	{
		case ReelingIn:
			State = Escaped;
			break;

		case Caught:
			GWorld->DestroyActor(this);
			return;

		default:
			State = Leaving;
	}
}

void AFish::OnSuddenPlayerMove(const FVector& MovementDelta)
{
	const float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);

	if (State < Interactive)
	{
		const auto Player = Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0));

		if (!IsValid(Player))
			return;

		if (IsValid(LureInVicinity) 
			&& LureInVicinity->IsDesirable()
			&& !LureInVicinity->IsCatchingFish()
			&& MovementDelta.Length() < (CatchingMovementRangeTop) * DeltaTime)
		{
			LureInVicinity->SetIsCatchingFish(true);
			Catch();
			return;
		}

		// UI for leaving or too strong or etc.
		SpookFish(Player->CameraComponent->GetComponentLocation());
	}
}

void AFish::OnSuddenPlayerRotate(const FRotator& RotationDelta)
{
	if (State == ReelingIn)
		GuaranteeEscape = true;
}

void AFish::OnColliderEnter(
	UPrimitiveComponent* OverlappedComp, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	auto* PotentialLure = Cast<AFishingLure>(OtherActor);

	if (IsValid(PotentialLure))
	{
		if (PotentialLure->IsSpooky())
		{
			SpookFish(PotentialLure->GetActorLocation());
			return;
		}

		LureInVicinity = PotentialLure;
	}
}

void AFish::OnColliderLeave(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	const auto* PotentialLure = Cast<AFishingLure>(OtherActor);

	//Reeling in requires lure for the movement strength value
	if (IsValid(PotentialLure) 
		&& LureInVicinity == PotentialLure
		&& State != ReelingIn)
		LureInVicinity = nullptr;
}

void AFish::LeavePond()
{
	if (State < Interactive)
		State = Leaving;
}

void AFish::Catch()
{
	if (State != Leaving && State < Interactive)
		State = ReelingIn;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Caught!")));

	if (IsValid(SplashSfx))
		UGameplayStatics::PlaySoundAtLocation(this, SplashSfx, GetActorLocation(), GetActorRotation());
}

void AFish::SpookFish(const FVector& WorldSpookSource)
{
	if (State >= Interactive || State == Leaving)
		return;

	State = Leaving;
	Speed *= 10;
	MaxAngularVelocity = 180;

	auto RelativeSpookSource = PinComponent->GetLocalToWorldTransform().GetLocation() - WorldSpookSource;
	RelativeSpookSource.Z = 0;

	const auto RelativeDirectionToSpookSource = RelativeSpookSource - RelativeTransform.GetLocation();
	RelativePointOfInterest = RelativeTransform.GetLocation() + (RelativeDirectionToSpookSource * -1) * 150;

	if (IsValid(SplashSfx))
		UGameplayStatics::PlaySoundAtLocation(this, SplashSfx, GetActorLocation(), GetActorRotation());
}

bool AFish::ShouldNotRemoveFromWorld() const
{
	return State == Escaped || State == Leaving || State == Caught;
}

void AFish::MoveTowardsInterest(const float DeltaTime)
{
	if (!IsValid(PinComponent))
		return;

	auto RelativeForward = RelativeTransform.GetRotation().GetForwardVector();
	RelativeForward.Normalize();

	auto RelativePosition = RelativeTransform.GetLocation();
	RelativePosition += RelativeForward * Speed * DeltaTime;
	RelativeTransform.SetLocation(RelativePosition);
	RelativePosition.Z = 0.0f;
	ActualAngularVelocity = MaxAngularVelocity * FMath::RandRange(0.5f, 1.f);

	auto RelativeDirectionToInterest = RelativePointOfInterest - RelativePosition;

	if (State == Swimming)
		Speed = FMath::Min(RelativeDirectionToInterest.Length(), MaxSpeed);

	RelativeDirectionToInterest.Normalize();

	float RelativeAngleToInterest = acosf(RelativeForward.Dot(RelativeDirectionToInterest));
	RelativeAngleToInterest *= 180 / PI;

	int ClockwiseDirection = 0;

	if (RelativeForward.Y > 0)
	{
		if (RelativeDirectionToInterest.Y > 0)
			ClockwiseDirection = RelativeForward.X > RelativeDirectionToInterest.X ? 1 : -1;
		else
			ClockwiseDirection = RelativeForward.X > RelativeDirectionToInterest.X * -1 ? -1 : 1;
	}
	else
	{
		if (RelativeDirectionToInterest.Y <= 0)
			ClockwiseDirection = RelativeForward.X > RelativeDirectionToInterest.X ? -1 : 1;
		else
			ClockwiseDirection = RelativeForward.X > RelativeDirectionToInterest.X * -1 ? 1 : -1;
	}

	RelativeAngleToInterest = floor(RelativeAngleToInterest);

	if (abs(RelativeAngleToInterest) < 2)
		return;

	RelativeTransform.SetRotation(
		FRotator(
			0, 
			RelativeTransform.GetRotation().Rotator().Yaw + fminf(abs(RelativeAngleToInterest), ActualAngularVelocity) * DeltaTime * ClockwiseDirection,
			0).Quaternion());

	MockCoro_WiggleAnimation(DeltaTime);
}

bool AFish::ShouldEscape()
{
	if (GuaranteeEscape)
		return true;

	return false;
}

void AFish::MockCoro_ConsiderChangingTarget(const float DeltaTime)
{
	MockCoro_ConsiderChangingTarget_Timer += DeltaTime;

	if (MockCoro_ConsiderChangingTarget_Timer > TargetChangeTime 
		|| FVector(RelativePointOfInterest - RelativeTransform.GetLocation()).Length() < 1.f)
	{
		int RandomValue = FMath::RandRange(1, 100);
		Iterations++;
		MockCoro_ConsiderChangingTarget_Timer = 0;

		FVector RelativeLureLocation;
		const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
		const auto Pond = (IsValid(GM) && IsValid(GM->GetGameplayPlane())) ?
			Cast<AFishingPond>(GM->GetGameplayPlane()) :
			nullptr;

		if (IsValid(Pond) 
			&& Pond->GetValidLureRelativeLocation(RelativeLureLocation)
			&& FVector(RelativeTransform.GetLocation() - RelativeLureLocation).Length() < LureDetectionRadius
			&& RandomValue <= ChoosingLureProbability)
		{
			RelativePointOfInterest = RelativeLureLocation;
			return;
		}

		RelativePointOfInterest = FVector(FMath::RandRange(-100, 100), FMath::RandRange(-100, 100), 0);
	}
}

bool AFish::MockCoro_FadeInAnimation(const float DeltaTime)
{
	if (MockCoro_FadeInAnimation_CurrentOpacity < 1.f)
	{
		MockCoro_FadeInAnimation_CurrentOpacity += DeltaTime * FadeInOutSpeed;

		if (MockCoro_FadeInAnimation_CurrentOpacity > 0.9f)
			MockCoro_FadeInAnimation_CurrentOpacity = 0.9f;

		if (IsValid(ActualMaterial))
			ActualMaterial->SetScalarParameterValue("OpacityFactor", MockCoro_FadeInAnimation_CurrentOpacity);

		return false;
	}

	return true;
}

bool AFish::MockCoro_FadeOutAnimation(const float DeltaTime)
{
	if (MockCoro_FadeOutAnimation_CurrentOpacity > 0.f)
	{
		MockCoro_FadeOutAnimation_CurrentOpacity -= DeltaTime * FadeInOutSpeed * 2;

		if (MockCoro_FadeOutAnimation_CurrentOpacity < 0.f)
			MockCoro_FadeOutAnimation_CurrentOpacity = 0.f;

		if (IsValid(ActualMaterial))
			ActualMaterial->SetScalarParameterValue("OpacityFactor", MockCoro_FadeOutAnimation_CurrentOpacity);

		return false;
	}

	return true;
}

bool AFish::MockCoro_ReelInAnimation(const float DeltaTime)
{
	auto* Player = Cast<ACustomARPawn>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (!IsValid(Player))
		return false;

	const auto CameraPosition = Player->CameraComponent->GetComponentLocation();

	if (MockCoro_ReelInAnimation_FirstRun)
	{
		PinComponent = nullptr;
		RealFishMeshComponent->SetVisibility(true);
		StaticMeshComponent->SetVisibility(false);
		ActualMaterial = FishActualMaterial;
		MockCoro_ReelInAnimation_FirstRun = false;
	}
	
	auto WorldActorLocation = GetActorLocation();
	auto ReelInDirection = CameraPosition - WorldActorLocation;
	ReelInDirection.Normalize();

	constexpr float ReelInSpeed = 400.f;
	WorldActorLocation += ReelInDirection * DeltaTime * ReelInSpeed;

	const auto SnappingDistance = UIObjectCameraOffset.Length();

	if (FVector(CameraPosition - WorldActorLocation).Length() <= SnappingDistance)
	{
		const auto GM = Cast<ACustomGameMode>(GetWorld()->GetAuthGameMode());
		const auto Pond = (IsValid(GM) && IsValid(GM->GetGameplayPlane())) ?
			Cast<AFishingPond>(GM->GetGameplayPlane()) :
			nullptr;

		if (IsValid(Pond) && Player->QuantityInTempInventory() < Pond->MaxCaughtFishCapacity)
		{
			WorldActorLocation.Z = 5000;
			SetARPosition(WorldActorLocation);
			Player->AddNewToTempInventory(this);

			if (IsValid(CaughtSfx))
				UGameplayStatics::PlaySoundAtLocation(this, CaughtSfx, GetActorLocation());

			return true;
		}

		GuaranteeEscape = true;
	}
	else
	{
		SetARPosition(WorldActorLocation);
	}

	return false;
}

void AFish::MockCoro_WiggleAnimation(const float DeltaTime)
{
	MockCoro_WiggleAnimation_SineInput += DeltaTime * WiggleSpeedFactor;

	float YawOffset = sinf(MockCoro_WiggleAnimation_SineInput) * MaxWiggleAngle;
	StaticMeshComponent->SetRelativeRotation(FRotator(0, YawOffset, 0));
}
