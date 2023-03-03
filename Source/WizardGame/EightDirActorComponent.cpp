// Fill out your copyright notice in the Description page of Project Settings.


#include "EightDirActorComponent.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BillboardComponent.h"
#include "Engine/ObjectLibrary.h"

// Sets default values for this component's properties
UEightDirActorComponent::UEightDirActorComponent () {
  PrimaryComponentTick.bCanEverTick = false;

  BasicFlipbooks.SetNum (static_cast<uint8>(EEightDir::EightDirMax) * static_cast<uint8>(EEightDirFlipbookSpeeds::SpeedMax));
  
  // Set all flipbooks to the default
  static ConstructorHelpers::FObjectFinder<UPaperFlipbook> DefaultFlipbook (DEFAULT_FLIPBOOK_PATH);
  if (DefaultFlipbook.Succeeded ()) {
    for (int i = 0; i < BasicFlipbooks.Num (); i++) {
      BasicFlipbooks[i] = DefaultFlipbook.Object;
    }
  } else {
    UE_LOG (LogTemp, Warning, TEXT ("DefaultFlipbook not found"));
  }
}

void UEightDirActorComponent::LoadFlipbooksFromDirectory (
  const FString &Directory,
  bool Stationary,
  bool Slow,
  bool Fast,
  bool IsEightDir,
  bool TwoFlipbookRotation
) {

  bStationaryGlobal = Stationary;
  bSlowGlobal = Slow;
  bFastGlobal = Fast;
  bIsEightDirGlobal = IsEightDir;

  FString SearchTerm;
  int step = bIsEightDirGlobal ? 1 : 2;


  if (!TwoFlipbookRotation) {
    FString DirectoryWithNoTrailingSlash = Directory;

    if (DirectoryWithNoTrailingSlash.EndsWith ("/")) {
      DirectoryWithNoTrailingSlash.RemoveAt (DirectoryWithNoTrailingSlash.Len () - 1);
    }

    if (Slow) {
      for (int i = 0; i < static_cast<uint8>(EEightDir::EightDirMax); i += step) {
        SearchTerm = *FString::Printf (TEXT ("%s/slow_%s.slow_%s"), *DirectoryWithNoTrailingSlash, *DirectionStrings[i], *DirectionStrings[i]);
        ConstructorHelpers::FObjectFinder<UPaperFlipbook> Flipbook (*SearchTerm);
        if (Flipbook.Succeeded ()) {
          BasicFlipbooks[GET_FLIPBOOK_INDEX (i, static_cast<uint8>(EEightDirFlipbookSpeeds::Slow))] = Flipbook.Object;
        } else {
          UE_LOG (LogTemp, Warning, TEXT ("%s not found"), *SearchTerm);
        }
      }
    }

    if (Fast) {
      for (int i = 0; i < static_cast<uint8>(EEightDir::EightDirMax); i += step) {
        SearchTerm = *FString::Printf (TEXT ("%s/fast_%s.fast_%s"), *DirectoryWithNoTrailingSlash, *DirectionStrings[i], *DirectionStrings[i]);
        ConstructorHelpers::FObjectFinder<UPaperFlipbook> Flipbook (*SearchTerm);
        if (Flipbook.Succeeded ()) {
          BasicFlipbooks[GET_FLIPBOOK_INDEX (i, static_cast<uint8>(EEightDirFlipbookSpeeds::Fast))] = Flipbook.Object;
        } else {
          UE_LOG (LogTemp, Warning, TEXT ("%s not found"), *SearchTerm);
        }
      }
    }

    if (Stationary) {
      for (int i = 0; i < static_cast<uint8>(EEightDir::EightDirMax); i += step) {
        SearchTerm = *FString::Printf (TEXT ("%s/stationary_%s.stationary_%s"), *DirectoryWithNoTrailingSlash, *DirectionStrings[i], *DirectionStrings[i]);
        ConstructorHelpers::FObjectFinder<UPaperFlipbook> Flipbook (*SearchTerm);
        if (Flipbook.Succeeded ()) {
          BasicFlipbooks[GET_FLIPBOOK_INDEX (i, static_cast<uint8>(EEightDirFlipbookSpeeds::Stationary))] = Flipbook.Object;
        } else {
          UE_LOG (LogTemp, Warning, TEXT ("%s not found"), *SearchTerm);
        }
      }
    }
  } else {

    UObjectLibrary *FlipbookLibrary = UObjectLibrary::CreateLibrary (UPaperFlipbook::StaticClass (), false, GIsEditor);

    FlipbookLibrary->AddToRoot ();
    FlipbookLibrary->LoadAssetDataFromPath (Directory);
    FlipbookLibrary->LoadAssetsFromAssetData ();

    TArray<FAssetData> AssetDataList;
    FlipbookLibrary->GetAssetDataList (AssetDataList);

    // We expect there to be only two flipbooks in the input directory
    if (AssetDataList.Num () != 2) {
      UE_LOG (LogTemp, Warning, TEXT ("Directory %s can only have two flipbooks present"), *Directory);
      return;
    }

    UPaperFlipbook *Flipbook1 = LoadObject<UPaperFlipbook> (nullptr, *AssetDataList[0].PackageName.ToString ());
    UPaperFlipbook *Flipbook2 = LoadObject<UPaperFlipbook> (nullptr, *AssetDataList[1].PackageName.ToString ());

    if (!Flipbook1) {
      UE_LOG (LogTemp, Warning, TEXT ("File %s is not valid!"), *AssetDataList[0].PackageName.ToString ());
    }

    if (!Flipbook2) {
      UE_LOG (LogTemp, Warning, TEXT ("File %s is not valid!"), *AssetDataList[1].PackageName.ToString ());
    }

    if (!(Flipbook1 && Flipbook2)) {
      return;
    }

    BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::North, EEightDirFlipbookSpeeds::Stationary)] = Flipbook1;
    BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::East, EEightDirFlipbookSpeeds::Stationary)] = bIsEightDirGlobal ? Flipbook1 : Flipbook2;
    BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::South, EEightDirFlipbookSpeeds::Stationary)] = Flipbook1;
    BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::West, EEightDirFlipbookSpeeds::Stationary)] = bIsEightDirGlobal ? Flipbook1 : Flipbook2;

    if (bIsEightDirGlobal) {
      BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::Northeast, EEightDirFlipbookSpeeds::Stationary)] = Flipbook2;
      BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::Southeast, EEightDirFlipbookSpeeds::Stationary)] = Flipbook2;
      BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::Southwest, EEightDirFlipbookSpeeds::Stationary)] = Flipbook2;
      BasicFlipbooks[GET_FLIPBOOK_INDEX (EEightDir::Northwest, EEightDirFlipbookSpeeds::Stationary)] = Flipbook2;
    }
  }
}

void UEightDirActorComponent::SetupAttachment (
  USceneComponent *ActorRootComponent,
  UPaperFlipbookComponent *DisplayFlipbookComponent,
  UPaperFlipbookComponent *ShadowFlipbookComponent,
  bool CastShadow,
  float SlowSpeed
) {
  DisplayFlipbook = DisplayFlipbookComponent;

  if (CastShadow && ShadowFlipbookComponent) {
    ShadowFlipbook = ShadowFlipbookComponent;
    ShadowFlipbook->CastShadow = true;
    ShadowFlipbook->bHiddenInGame = true;
    ShadowFlipbook->bCastHiddenShadow = true;
  }
  bCastShadowGlobal = CastShadow;
  RootComponentGlobal = ActorRootComponent;
  SlowSpeedGlobal = SlowSpeed < 0.0f ? DEFAULT_FLIPBOOK_SLOW_SPEED : SlowSpeed;
  PrimitiveRootComponentGlobal = Cast<UPrimitiveComponent> (ActorRootComponent);
}

void UEightDirActorComponent::UpdateFlipbook (
  EEightDir DisplayFlipboookDirection,
  EEightDir ShadowFlipboookDirection,
  float Speed
) {
  UPaperFlipbook *FlipbookToUse[2] = {nullptr, nullptr};
  EEightDir Direction = DisplayFlipboookDirection;

  for (int i = (DisplayFlipboookDirection < EEightDir::EightDirMax ? 0 : 1);
       i < (bCastShadowGlobal && ShadowFlipboookDirection < EEightDir::EightDirMax ? 2 : 1);
       i++
       ) {

    if (i == SHADOW_ITERATION) {
      Direction = ShadowFlipboookDirection;
    }

    if (Speed >= SlowSpeedGlobal + SLOW_SPEED_BUFFER && bFastGlobal) {
      FlipbookToUse[i] = BasicFlipbooks[GET_FLIPBOOK_INDEX (Direction, EEightDirFlipbookSpeeds::Fast)];
    } else if (Speed >= MIN_SLOW_SPEED && bSlowGlobal) {
      FlipbookToUse[i] = BasicFlipbooks[GET_FLIPBOOK_INDEX (Direction, EEightDirFlipbookSpeeds::Slow)];
    } else {
      FlipbookToUse[i] = BasicFlipbooks[GET_FLIPBOOK_INDEX (Direction, EEightDirFlipbookSpeeds::Stationary)];
    }
  }

  if (DisplayFlipbook && DisplayFlipboookDirection < EEightDir::EightDirMax) {
    DisplayFlipbook->SetFlipbook (FlipbookToUse[DISPLAY_ITERATION]);
  }

  if (bCastShadowGlobal && ShadowFlipbook && ShadowFlipboookDirection < EEightDir::EightDirMax) {
    ShadowFlipbook->SetFlipbook (FlipbookToUse[SHADOW_ITERATION]);
  }
}

void UEightDirActorComponent::BeginPlay () {
  Super::BeginPlay ();

  // Populate the camera reference global
  CameraManagerGlobal = GetWorld ()->GetFirstPlayerController ()->PlayerCameraManager;
  if (!CameraManagerGlobal) {
    UE_LOG (LogTemp, Warning, TEXT ("Unable to initialize CameraManagerGlobal"));
  }

  // Populate the "sun" directional light global
  SunGlobal = Cast<ADirectionalLight> (UGameplayStatics::GetActorOfClass (GetWorld (), ADirectionalLight::StaticClass ()));
  if (!SunGlobal) {
    UE_LOG (LogTemp, Warning, TEXT ("Unable to initialize SunGlobal"))
  }
}

EEightDir UEightDirActorComponent::GetDirection (float Yaw) {
  if (bIsEightDirGlobal) {
    if EIGHT_DIR_IS_NORTH (Yaw) {
      return EEightDir::North;
    } else if EIGHT_DIR_IS_NORTHEAST (Yaw) {
      return EEightDir::Northeast;
    } else if EIGHT_DIR_IS_EAST (Yaw) {
      return EEightDir::East;
    } else if EIGHT_DIR_IS_SOUTHEAST (Yaw) {
      return EEightDir::Southeast;
    } else if EIGHT_DIR_IS_SOUTH (Yaw) {
      return EEightDir::South;
    } else if EIGHT_DIR_IS_SOUTHWEST (Yaw) {
      return EEightDir::Southwest;
    } else if EIGHT_DIR_IS_WEST (Yaw) {
      return EEightDir::West;
    } else {
      return EEightDir::Northwest;
    }
  } else {
    if FOUR_DIR_IS_NORTH (Yaw) {
      return EEightDir::North;
    } else if FOUR_DIR_IS_EAST (Yaw) {
      return EEightDir::East;
    } else if FOUR_DIR_IS_SOUTH (Yaw) {
      return EEightDir::South;
    } else {
      return EEightDir::West;
    }
  }
}

EEightDir UEightDirActorComponent::GetFlipbookDirection (FRotator ControlRotation, FRotator ComponentRotation) {
  return GetDirection (UKismetMathLibrary::NormalizedDeltaRotator (ComponentRotation, ControlRotation).Yaw);
}

void UEightDirActorComponent::UpdateDisplayFlipbook (bool ForceUpdate, float SpeedOverride) {

  if (DisplayFlipbook && (ForceUpdate || DisplayFlipbook->WasRecentlyRendered (0.1f))) {
    FRotator ControlRotation;
    EEightDir DisplayFlipbookDirection = EEightDir::North;

    if (!CameraManagerGlobal) {
      CameraManagerGlobal = GetWorld ()->GetFirstPlayerController ()->PlayerCameraManager;
    }

    // BEEBE TODO: Remove?
    if (CameraManagerGlobal) {
      // Rotate the display flipbook
      ControlRotation = CameraManagerGlobal->GetCameraRotation ();
      DisplayFlipbook->SetWorldRotation (FRotator (0.0f, ControlRotation.Yaw + FLIPBOOK_ROTATIONAL_OFFSET, 0.0f));
      UpdateFlipbook (
        GetFlipbookDirection (ControlRotation, RootComponentGlobal->GetComponentRotation ()),
        EEightDir::EightDirMax,
        SpeedOverride >= 0.0f ? SpeedOverride : PrimitiveRootComponentGlobal->GetPhysicsLinearVelocity ().Size ()
      );

    }
  }
}

void UEightDirActorComponent::UpdateShadowFlipbook (bool ForceUpdate, float SpeedOverride) {

  if (ShadowFlipbook && (ForceUpdate || ShadowFlipbook->WasRecentlyRendered (0.1f))) {
    FRotator ControlRotation;
    EEightDir ShadowFlipbookDirection = EEightDir::North;

    if (bCastShadowGlobal) {

      // BEEBE TODO: Remove?
      if (!SunGlobal) {
        SunGlobal = Cast<ADirectionalLight> (UGameplayStatics::GetActorOfClass (GetWorld (), ADirectionalLight::StaticClass ()));
      }

      if (SunGlobal) {
        // Rotate the shadow flipbook
        ControlRotation = SunGlobal->GetActorRotation ();
        ShadowFlipbook->SetWorldRotation (FRotator (0.0f, ControlRotation.Yaw + FLIPBOOK_ROTATIONAL_OFFSET, 0.0f));
        UpdateFlipbook (
          EEightDir::EightDirMax,
          GetFlipbookDirection (ControlRotation, RootComponentGlobal->GetComponentRotation ()),
          SpeedOverride >= 0.0f ? SpeedOverride : PrimitiveRootComponentGlobal->GetPhysicsLinearVelocity ().Size ()
        );
      }

      // Test rotating the sun
      //SunGlobal->SetActorRotation(FRotator(ControlRotation.Pitch, ControlRotation.Yaw + 1.0f, ControlRotation.Roll));
    }
  }
}

void UEightDirActorComponent::UpdateDisplayAndShadowFlipbooks (bool ForceUpdate, float SpeedOverride) {
  if (DisplayFlipbook && (ForceUpdate || DisplayFlipbook->WasRecentlyRendered (0.1f))) {
    EEightDir ShadowFlipbookDirection = EEightDir::North;
    EEightDir DisplayFlipbookDirection = EEightDir::North;
    FRotator ControlRotation;

    // BEEBE TODO: Remove?
    if (!CameraManagerGlobal) {
      CameraManagerGlobal = GetWorld ()->GetFirstPlayerController ()->PlayerCameraManager;
    }

    if (CameraManagerGlobal) {
      // Rotate the display flipbook
      ControlRotation = CameraManagerGlobal->GetCameraRotation ();
      DisplayFlipbook->SetWorldRotation (FRotator (0.0f, ControlRotation.Yaw + FLIPBOOK_ROTATIONAL_OFFSET, 0.0f));
      DisplayFlipbookDirection = GetFlipbookDirection (ControlRotation, RootComponentGlobal->GetComponentRotation ());
    }

    if (ShadowFlipbook && bCastShadowGlobal) {
      // BEEBE TODO: Remove?
      if (!SunGlobal) {
        SunGlobal = Cast<ADirectionalLight> (UGameplayStatics::GetActorOfClass (GetWorld (), ADirectionalLight::StaticClass ()));
      }

      if (SunGlobal) {
        // Rotate the shadow flipbook
        ControlRotation = SunGlobal->GetActorRotation ();
        ShadowFlipbook->SetWorldRotation (FRotator (0.0f, ControlRotation.Yaw + FLIPBOOK_ROTATIONAL_OFFSET, 0.0f));
        ShadowFlipbookDirection = GetFlipbookDirection (ControlRotation, RootComponentGlobal->GetComponentRotation ());
      }
    }

    // Only update the flipbook(s) if the component has been rendered recently
    UpdateFlipbook (
      DisplayFlipbookDirection,
      ShadowFlipbookDirection,
      SpeedOverride >= 0.0f ? SpeedOverride : PrimitiveRootComponentGlobal->GetPhysicsLinearVelocity ().Size ()
    );
  }
}
