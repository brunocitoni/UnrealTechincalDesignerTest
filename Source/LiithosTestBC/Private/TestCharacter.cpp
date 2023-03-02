// Fill out your copyright notice in the Description page of Project Settings.


#include "TestCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Components/ShapeComponent.h"

// Sets default values
ATestCharacter::ATestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// could use a proper function here rather than creating 4 of them like this... but it works
	SphereComponent[0] = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponentRightHand"));
	SphereComponent[1] = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponentLeftHand"));
	SphereComponent[2] = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponentRightFoot"));
	SphereComponent[3] = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponentLeftFoot"));

	FName namesArray[4] = { FName(TEXT("Att_HandR")), FName(TEXT("Att_HandL")), FName(TEXT("Att_FootR")), FName(TEXT("Att_FootL")) };

	for (int i = 0; i < 4; i++)
	{
		SphereComponent[i]->SetSphereRadius(15.f);
		// attach to socket
		//SphereComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName(TEXT("Att_FootR")));
		SphereComponent[i]->SetupAttachment(GetMesh(), namesArray[i]);
		// activates collision and overlap events
		SphereComponent[i]->SetCollisionProfileName("OverlapAllDynamic");
		SphereComponent[i]->CanCharacterStepUpOn = ECB_No;
		SphereComponent[i]->SetGenerateOverlapEvents(true);
	}
}

// Called when the game starts or when spawned
void ATestCharacter::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// these seem like they need to be defined in BEGIN PLAY to work
	for (int i = 0; i < 4; i++)
	{
		SphereComponent[i]->OnComponentBeginOverlap.AddDynamic(this, &ATestCharacter::OnSphereComponentOverlapBegin);
		SphereComponent[i]->OnComponentEndOverlap.AddDynamic(this, &ATestCharacter::OnSphereComponentOverlapEnd);
	}
}

// overlap events
void ATestCharacter::OnSphereComponentOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (*OtherActor->GetName() == this->GetName()) // cheap way of not showing collisions with itself rather than preventing them in the first place
		return;
	FString DebugString = FString::Printf(TEXT("%s is colliding with %s"), *OtherActor->GetName(), *OverlappedComponent->GetName()); // GetName() is not the most human readable way
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
}

void ATestCharacter::OnSphereComponentOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (*OtherActor->GetName() == this->GetName()) // cheap way of not showing collisions with itself rather than preventing them in the first place
		return;
	FString DebugString = FString::Printf(TEXT("%s stopped colliding with %s"), *OtherActor->GetName(), *OverlappedComponent->GetName()); // GetName() is not the most human readable way
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugString);
}


// Called every frame
void ATestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//////////////////////////////////////////////////////////////////////////
// Input from 3d player class

void ATestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestCharacter::Look);
	}
}

void ATestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

