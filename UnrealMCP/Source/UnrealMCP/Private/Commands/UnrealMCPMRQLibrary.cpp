// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/UnrealMCPMRQLibrary.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// MRQ includes
#include "MoviePipeline.h"
#include "MoviePipelineOutputSetting.h"
#include "MoviePipelineAntiAliasingSetting.h"
#include "MoviePipelinePrimaryConfig.h"
#include "MoviePipelineQueueEngineSubsystem.h"

// Level Sequence includes
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieScene.h"

bool UUnrealMCPMRQLibrary::GetMRQSettings(UObject* WorldContextObject, FMRQSettingsData& OutSettings)
{
    UMoviePipeline* Pipeline = GetActiveMoviePipeline(WorldContextObject);

    if (!Pipeline)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No active MoviePipeline found"));
        return false;
    }

    // Reset to defaults
    OutSettings = FMRQSettingsData();

    // Get the pipeline configuration
    UMoviePipelinePrimaryConfig* Config = Pipeline->GetPipelinePrimaryConfig();
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No PipelinePrimaryConfig found"));
        return false;
    }

    // Get all settings from the configuration
    TArray<UMoviePipelineSetting*> AllSettings = Config->GetAllSettings(true, true);

    for (UMoviePipelineSetting* Setting : AllSettings)
    {
        if (UMoviePipelineOutputSetting* OutputSetting = Cast<UMoviePipelineOutputSetting>(Setting))
        {
            // Get resolution
            FIntPoint Resolution = OutputSetting->OutputResolution;
            OutSettings.ResolutionWidth = Resolution.X;
            OutSettings.ResolutionHeight = Resolution.Y;

            // Get frame rate
            FFrameRate FrameRate = OutputSetting->OutputFrameRate;
            OutSettings.FrameRate = FrameRate.AsDecimal();

            UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: Resolution: %dx%d, FrameRate: %f"),
                OutSettings.ResolutionWidth, OutSettings.ResolutionHeight, OutSettings.FrameRate);
        }
        else if (UMoviePipelineAntiAliasingSetting* AASetting = Cast<UMoviePipelineAntiAliasingSetting>(Setting))
        {
            // Get anti-aliasing method
            OutSettings.AntiAliasingMethod = StaticEnum<EAntiAliasingMethod>()->GetNameStringByValue((int64)AASetting->AntiAliasingMethod);
            OutSettings.SpatialSamples = AASetting->SpatialSampleCount;
            OutSettings.TemporalSamples = AASetting->TemporalSampleCount;

            UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: AA Method: %s, Spatial: %d, Temporal: %d"),
                *OutSettings.AntiAliasingMethod, OutSettings.SpatialSamples, OutSettings.TemporalSamples);
        }
    }

    return true;
}

bool UUnrealMCPMRQLibrary::GetTimeDilationInfo(UObject* WorldContextObject, FTimeDilationData& OutInfo)
{
    ALevelSequenceActor* LSActor = GetLevelSequenceActor(WorldContextObject);

    if (!LSActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No LevelSequenceActor found"));
        return false;
    }

    ULevelSequencePlayer* Player = LSActor->GetSequencePlayer();
    if (!Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No SequencePlayer found"));
        return false;
    }

    // Get playback rate (time dilation)
    float PlaybackRate = Player->GetPlayRate();
    OutInfo.bEnabled = !FMath::IsNearlyEqual(PlaybackRate, 1.0f, KINDA_SMALL_NUMBER);
    OutInfo.Value = PlaybackRate;

    UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: TimeDilation enabled: %s, value: %f"),
        OutInfo.bEnabled ? TEXT("true") : TEXT("false"), OutInfo.Value);

    return true;
}

bool UUnrealMCPMRQLibrary::GetLevelSequenceInfo(UObject* WorldContextObject, FLevelSequenceData& OutInfo)
{
    ALevelSequenceActor* LSActor = GetLevelSequenceActor(WorldContextObject);

    if (!LSActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No LevelSequenceActor found"));
        return false;
    }

    ULevelSequence* Sequence = LSActor->GetSequence();
    if (!Sequence)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: No LevelSequence found"));
        return false;
    }

    // Reset to defaults
    OutInfo = FLevelSequenceData();

    // Get sequence name and path
    OutInfo.Name = Sequence->GetName();
    OutInfo.Path = Sequence->GetPathName();

    // Get MovieScene to access playback range and display rate
    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (MovieScene)
    {
        // Get playback range
        TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();
        OutInfo.PlaybackStart = PlaybackRange.GetLowerBoundValue().Value;
        OutInfo.PlaybackEnd = PlaybackRange.GetUpperBoundValue().Value;

        // Get display rate
        FFrameRate DisplayRate = MovieScene->GetDisplayRate();
        OutInfo.DisplayRate = DisplayRate.AsDecimal();
    }

    // Get time dilation from player
    ULevelSequencePlayer* Player = LSActor->GetSequencePlayer();
    if (Player)
    {
        float PlaybackRate = Player->GetPlayRate();
        OutInfo.bTimeDilationEnabled = !FMath::IsNearlyEqual(PlaybackRate, 1.0f, KINDA_SMALL_NUMBER);
        OutInfo.TimeDilationValue = PlaybackRate;
    }

    UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: Sequence: %s, Range: %d-%d, Rate: %f"),
        *OutInfo.Name, OutInfo.PlaybackStart, OutInfo.PlaybackEnd, OutInfo.DisplayRate);

    return true;
}

bool UUnrealMCPMRQLibrary::GenerateMRQExportJson(UObject* WorldContextObject, FString& OutJsonString)
{
    TSharedPtr<FJsonObject> RootJson = MakeShareable(new FJsonObject);

    // Add MRQ Settings
    FMRQSettingsData MRQSettings;
    if (GetMRQSettings(WorldContextObject, MRQSettings))
    {
        TSharedPtr<FJsonObject> MRQObj = MakeShareable(new FJsonObject);

        // Resolution
        TSharedPtr<FJsonObject> ResObj = MakeShareable(new FJsonObject);
        ResObj->SetNumberField(TEXT("width"), MRQSettings.ResolutionWidth);
        ResObj->SetNumberField(TEXT("height"), MRQSettings.ResolutionHeight);
        MRQObj->SetObjectField(TEXT("resolution"), ResObj);

        // Frame rate
        MRQObj->SetNumberField(TEXT("frame_rate"), MRQSettings.FrameRate);

        // Anti-aliasing
        TSharedPtr<FJsonObject> AAObj = MakeShareable(new FJsonObject);
        AAObj->SetStringField(TEXT("method"), MRQSettings.AntiAliasingMethod);
        AAObj->SetNumberField(TEXT("spatial_samples"), MRQSettings.SpatialSamples);
        AAObj->SetNumberField(TEXT("temporal_samples"), MRQSettings.TemporalSamples);
        MRQObj->SetObjectField(TEXT("anti_aliasing"), AAObj);

        RootJson->SetObjectField(TEXT("MRQSettings"), MRQObj);
    }

    // Add Time Dilation
    FTimeDilationData TimeDilation;
    if (GetTimeDilationInfo(WorldContextObject, TimeDilation))
    {
        TSharedPtr<FJsonObject> TDObj = MakeShareable(new FJsonObject);
        TDObj->SetBoolField(TEXT("enabled"), TimeDilation.bEnabled);
        TDObj->SetNumberField(TEXT("value"), TimeDilation.Value);
        RootJson->SetObjectField(TEXT("TimeDilation"), TDObj);
    }

    // Add Level Sequence Info
    FLevelSequenceData SeqInfo;
    if (GetLevelSequenceInfo(WorldContextObject, SeqInfo))
    {
        TSharedPtr<FJsonObject> SeqObj = MakeShareable(new FJsonObject);
        SeqObj->SetStringField(TEXT("name"), SeqInfo.Name);
        SeqObj->SetStringField(TEXT("path"), SeqInfo.Path);
        SeqObj->SetNumberField(TEXT("playback_start"), SeqInfo.PlaybackStart);
        SeqObj->SetNumberField(TEXT("playback_end"), SeqInfo.PlaybackEnd);
        SeqObj->SetNumberField(TEXT("display_rate"), SeqInfo.DisplayRate);
        SeqObj->SetBoolField(TEXT("time_dilation_enabled"), SeqInfo.bTimeDilationEnabled);
        SeqObj->SetNumberField(TEXT("time_dilation_value"), SeqInfo.TimeDilationValue);
        RootJson->SetObjectField(TEXT("LevelSequence"), SeqObj);
    }

    // Serialize to string
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJsonString);
    bool bSuccess = FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: Generated MRQ export JSON successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPMRQLibrary: Failed to serialize JSON"));
    }

    return bSuccess;
}

bool UUnrealMCPMRQLibrary::IsMRQRenderingActive(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return false;
    }

    // Use GEngine to get the engine subsystem
    UMoviePipelineQueueEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMoviePipelineQueueEngineSubsystem>();
    if (!Subsystem)
    {
        return false;
    }

    return Subsystem->IsRendering();
}

bool UUnrealMCPMRQLibrary::GenerateTrackerExportJson(
    UObject* WorldContextObject,
    const TArray<FString>& TrackerNames,
    const TArray<FVector2D>& TrackerPositions,
    int32 TotalFrames,
    FString& OutJsonString)
{
    TSharedPtr<FJsonObject> RootJson = MakeShareable(new FJsonObject);

    // Build Anim array
    // Expected format: Anim[frame][tracker] = {x, y}
    // But we receive positions as a flat array, so we need to reorganize
    TArray<TSharedPtr<FJsonValue>> AnimArray;

    if (TrackerNames.Num() > 0 && TotalFrames > 0)
    {
        // Calculate expected positions count
        int32 PositionsPerFrame = TrackerNames.Num();
        int32 ExpectedPositions = TotalFrames * PositionsPerFrame;

        if (TrackerPositions.Num() >= ExpectedPositions)
        {
            for (int32 FrameIndex = 0; FrameIndex < TotalFrames; FrameIndex++)
            {
                TSharedPtr<FJsonObject> FrameObj = MakeShareable(new FJsonObject);

                for (int32 TrackerIndex = 0; TrackerIndex < TrackerNames.Num(); TrackerIndex++)
                {
                    int32 PositionIndex = FrameIndex * PositionsPerFrame + TrackerIndex;
                    if (PositionIndex < TrackerPositions.Num())
                    {
                        const FVector2D& Pos = TrackerPositions[PositionIndex];

                        TSharedPtr<FJsonObject> PosObj = MakeShareable(new FJsonObject);
                        PosObj->SetNumberField(TEXT("x"), Pos.X);
                        PosObj->SetNumberField(TEXT("y"), Pos.Y);

                        FrameObj->SetObjectField(TrackerNames[TrackerIndex], PosObj);
                    }
                }

                AnimArray.Add(MakeShareable(new FJsonValueObject(FrameObj)));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UnrealMCPMRQLibrary: Not enough tracker positions. Expected %d, got %d"),
                ExpectedPositions, TrackerPositions.Num());
        }
    }

    RootJson->SetArrayField(TEXT("Anim"), AnimArray);

    // Build NullLayers array
    TArray<TSharedPtr<FJsonValue>> NullLayersArray;
    for (const FString& Name : TrackerNames)
    {
        NullLayersArray.Add(MakeShareable(new FJsonValueString(Name)));
    }
    RootJson->SetArrayField(TEXT("NullLayers"), NullLayersArray);

    // Add MRQ Settings
    FMRQSettingsData MRQSettings;
    if (GetMRQSettings(WorldContextObject, MRQSettings))
    {
        TSharedPtr<FJsonObject> MRQObj = MakeShareable(new FJsonObject);

        TSharedPtr<FJsonObject> ResObj = MakeShareable(new FJsonObject);
        ResObj->SetNumberField(TEXT("width"), MRQSettings.ResolutionWidth);
        ResObj->SetNumberField(TEXT("height"), MRQSettings.ResolutionHeight);
        MRQObj->SetObjectField(TEXT("resolution"), ResObj);

        MRQObj->SetNumberField(TEXT("frame_rate"), MRQSettings.FrameRate);

        TSharedPtr<FJsonObject> AAObj = MakeShareable(new FJsonObject);
        AAObj->SetStringField(TEXT("method"), MRQSettings.AntiAliasingMethod);
        AAObj->SetNumberField(TEXT("spatial_samples"), MRQSettings.SpatialSamples);
        AAObj->SetNumberField(TEXT("temporal_samples"), MRQSettings.TemporalSamples);
        MRQObj->SetObjectField(TEXT("anti_aliasing"), AAObj);

        RootJson->SetObjectField(TEXT("MRQSettings"), MRQObj);
    }

    // Add Time Dilation
    FTimeDilationData TimeDilation;
    if (GetTimeDilationInfo(WorldContextObject, TimeDilation))
    {
        TSharedPtr<FJsonObject> TDObj = MakeShareable(new FJsonObject);
        TDObj->SetBoolField(TEXT("enabled"), TimeDilation.bEnabled);
        TDObj->SetNumberField(TEXT("value"), TimeDilation.Value);
        RootJson->SetObjectField(TEXT("TimeDilation"), TDObj);
    }

    // Add Level Sequence Info
    FLevelSequenceData SeqInfo;
    if (GetLevelSequenceInfo(WorldContextObject, SeqInfo))
    {
        TSharedPtr<FJsonObject> SeqObj = MakeShareable(new FJsonObject);
        SeqObj->SetStringField(TEXT("name"), SeqInfo.Name);
        SeqObj->SetStringField(TEXT("path"), SeqInfo.Path);
        SeqObj->SetNumberField(TEXT("playback_start"), SeqInfo.PlaybackStart);
        SeqObj->SetNumberField(TEXT("playback_end"), SeqInfo.PlaybackEnd);
        SeqObj->SetNumberField(TEXT("display_rate"), SeqInfo.DisplayRate);
        SeqObj->SetBoolField(TEXT("time_dilation_enabled"), SeqInfo.bTimeDilationEnabled);
        SeqObj->SetNumberField(TEXT("time_dilation_value"), SeqInfo.TimeDilationValue);
        RootJson->SetObjectField(TEXT("LevelSequence"), SeqObj);
    }

    // Serialize to string
    TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJsonString);
    bool bSuccess = FJsonSerializer::Serialize(RootJson.ToSharedRef(), Writer);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("UnrealMCPMRQLibrary: Generated tracker export JSON successfully (%d frames, %d trackers)"),
            TotalFrames, TrackerNames.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPMRQLibrary: Failed to serialize JSON"));
    }

    return bSuccess;
}

UMoviePipeline* UUnrealMCPMRQLibrary::GetActiveMoviePipeline(UObject* WorldContextObject)
{
    // Use GEngine to get the engine subsystem
    UMoviePipelineQueueEngineSubsystem* Subsystem = GEngine->GetEngineSubsystem<UMoviePipelineQueueEngineSubsystem>();
    if (!Subsystem)
    {
        return nullptr;
    }

    // Check if rendering is active
    if (!Subsystem->IsRendering())
    {
        return nullptr;
    }

    // Get the active executor
    UMoviePipelineExecutorBase* Executor = Subsystem->GetActiveExecutor();
    if (!Executor)
    {
        return nullptr;
    }

    // Search for all MoviePipeline objects in the world
    TArray<UObject*> PipelineObjects;
    GetObjectsOfClass(UMoviePipeline::StaticClass(), PipelineObjects);

    for (UObject* Obj : PipelineObjects)
    {
        if (UMoviePipeline* Pipeline = Cast<UMoviePipeline>(Obj))
        {
            // Check if this pipeline is currently active/rendering
            if (Pipeline->GetPipelineState() != EMovieRenderPipelineState::Finished)
            {
                return Pipeline;
            }
        }
    }

    return nullptr;
}

ALevelSequenceActor* UUnrealMCPMRQLibrary::GetLevelSequenceActor(UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> LevelSequenceActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALevelSequenceActor::StaticClass(), LevelSequenceActors);

    if (LevelSequenceActors.Num() == 0)
    {
        return nullptr;
    }

    return Cast<ALevelSequenceActor>(LevelSequenceActors[0]);
}
