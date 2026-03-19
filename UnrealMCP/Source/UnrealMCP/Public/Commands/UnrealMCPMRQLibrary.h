// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealMCPMRQLibrary.generated.h"

/**
 * MRQ Settings Data Structure
 * Contains resolution, frame rate, and anti-aliasing settings from Movie Render Queue
 */
USTRUCT(BlueprintType)
struct FMRQSettingsData
{
    GENERATED_BODY()

    /** Output resolution width in pixels */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    int32 ResolutionWidth = 1920;

    /** Output resolution height in pixels */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    int32 ResolutionHeight = 1080;

    /** Output frame rate as decimal (e.g., 29.97, 30.0, 24.0) */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    float FrameRate = 30.0f;

    /** Anti-aliasing method name (None, FXAA, TAA, MSAA, TSR) */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    FString AntiAliasingMethod = TEXT("None");

    /** Number of spatial samples for anti-aliasing */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    int32 SpatialSamples = 1;

    /** Number of temporal samples for anti-aliasing */
    UPROPERTY(BlueprintReadOnly, Category = "MRQ Settings")
    int32 TemporalSamples = 1;

    FMRQSettingsData()
        : ResolutionWidth(1920)
        , ResolutionHeight(1080)
        , FrameRate(30.0f)
        , AntiAliasingMethod(TEXT("None"))
        , SpatialSamples(1)
        , TemporalSamples(1)
    {
    }
};

/**
 * Time Dilation Data Structure
 * Contains time dilation (playback rate) information from Level Sequence
 */
USTRUCT(BlueprintType)
struct FTimeDilationData
{
    GENERATED_BODY()

    /** Whether time dilation is enabled (playback rate != 1.0) */
    UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
    bool bEnabled = false;

    /** The time dilation value (1.0 = normal, 0.5 = half speed, 2.0 = double speed) */
    UPROPERTY(BlueprintReadOnly, Category = "Time Dilation")
    float Value = 1.0f;

    FTimeDilationData()
        : bEnabled(false)
        , Value(1.0f)
    {
    }
};

/**
 * Level Sequence Data Structure
 * Contains information about the current Level Sequence being rendered
 */
USTRUCT(BlueprintType)
struct FLevelSequenceData
{
    GENERATED_BODY()

    /** Name of the Level Sequence asset */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    FString Name;

    /** Full path of the Level Sequence asset */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    FString Path;

    /** Playback start frame */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    int32 PlaybackStart = 0;

    /** Playback end frame */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    int32 PlaybackEnd = 0;

    /** Display rate (frames per second) */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    float DisplayRate = 30.0f;

    /** Whether time dilation is enabled for this sequence */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    bool bTimeDilationEnabled = false;

    /** Time dilation value for this sequence */
    UPROPERTY(BlueprintReadOnly, Category = "Level Sequence")
    float TimeDilationValue = 1.0f;

    FLevelSequenceData()
        : PlaybackStart(0)
        , PlaybackEnd(0)
        , DisplayRate(30.0f)
        , bTimeDilationEnabled(false)
        , TimeDilationValue(1.0f)
    {
    }
};

/**
 * UnrealMCP MRQ Library
 *
 * A Blueprint Function Library that provides access to Movie Render Queue settings
 * and Level Sequence information. This library is designed to work independently
 * without requiring MCP connection, making it suitable for offline use and
 * non-C++ projects.
 *
 * Usage in Blueprints:
 * 1. Call GetMRQSettings() to get current render settings
 * 2. Call GetTimeDilationInfo() to check for time dilation
 * 3. Call GetLevelSequenceInfo() to get sequence details
 * 4. Call GenerateMRQExportJson() to get a complete JSON string for export
 */
UCLASS()
class UNREALMCP_API UUnrealMCPMRQLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Get current MRQ (Movie Render Queue) render settings.
     * This function can be called during MRQ rendering to get the current settings.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @param OutSettings - Output structure containing MRQ settings
     * @return True if settings were successfully retrieved, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool GetMRQSettings(UObject* WorldContextObject, FMRQSettingsData& OutSettings);

    /**
     * Get time dilation (playback rate) information from the active Level Sequence.
     * Time dilation affects animation speed during rendering.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @param OutInfo - Output structure containing time dilation info
     * @return True if time dilation info was successfully retrieved, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool GetTimeDilationInfo(UObject* WorldContextObject, FTimeDilationData& OutInfo);

    /**
     * Get detailed information about the current Level Sequence being rendered.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @param OutInfo - Output structure containing level sequence info
     * @return True if level sequence info was successfully retrieved, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool GetLevelSequenceInfo(UObject* WorldContextObject, FLevelSequenceData& OutInfo);

    /**
     * Generate a complete JSON string containing all MRQ settings, time dilation,
     * and level sequence information. This is useful for exporting to external tools.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @param OutJsonString - Output JSON string
     * @return True if JSON was successfully generated, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool GenerateMRQExportJson(UObject* WorldContextObject, FString& OutJsonString);

    /**
     * Check if there is currently an active MRQ rendering in progress.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @return True if MRQ rendering is active, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool IsMRQRenderingActive(UObject* WorldContextObject);

    /**
     * Generate a complete tracker export JSON with all MRQ settings.
     * This combines tracker data with MRQ settings for a complete export.
     *
     * @param WorldContextObject - Any object to get world context (use 'self' in blueprints)
     * @param TrackerNames - Array of tracker point names
     * @param TrackerPositions - Array of tracker positions (one per frame, per tracker)
     * @param OutJsonString - Output JSON string
     * @return True if JSON was successfully generated, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "UnrealMCP|MRQ", meta = (WorldContext = "WorldContextObject"))
    static bool GenerateTrackerExportJson(
        UObject* WorldContextObject,
        const TArray<FString>& TrackerNames,
        const TArray<FVector2D>& TrackerPositions,
        int32 TotalFrames,
        FString& OutJsonString
    );

private:
    /**
     * Internal helper to get the active MoviePipeline if one exists.
     */
    static class UMoviePipeline* GetActiveMoviePipeline(UObject* WorldContextObject);

    /**
     * Internal helper to get the first LevelSequenceActor in the world.
     */
    static class ALevelSequenceActor* GetLevelSequenceActor(UObject* WorldContextObject);
};
