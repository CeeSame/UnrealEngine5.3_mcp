#include "UnrealMCPModule.h"
#include "UnrealMCPBridge.h"
#include "UnrealMCPPanel.h"
#include "Modules/ModuleManager.h"
#include "EditorSubsystem.h"
#include "Editor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FUnrealMCPModule"

static const FName UnrealMCPTabName("UnrealMCP");

void FUnrealMCPModule::StartupModule()
{
	UE_LOG(LogTemp, Display, TEXT("Unreal MCP Module has started"));

	// Register the Nomad tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		UnrealMCPTabName,
		FOnSpawnTab::CreateRaw(this, &FUnrealMCPModule::SpawnTab)
	)
	.SetDisplayName(LOCTEXT("TabTitle", "UnrealMCP"))
	.SetMenuType(ETabSpawnerMenuType::Enabled);
}

void FUnrealMCPModule::ShutdownModule()
{
	UE_LOG(LogTemp, Display, TEXT("Unreal MCP Module has shut down"));

	// Unregister the tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UnrealMCPTabName);
}

TSharedRef<SDockTab> FUnrealMCPModule::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUnrealMCPPanel)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealMCPModule, UnrealMCP)
