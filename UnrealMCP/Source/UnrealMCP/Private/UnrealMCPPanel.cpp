#include "UnrealMCPPanel.h"
#include "UnrealMCPBridge.h"
#include "Editor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "UnrealMCPPanel"

void SUnrealMCPPanel::Construct(const FArguments& InArgs)
{
	// Get initial state
	if (GEditor)
	{
		UUnrealMCPBridge* Bridge = GEditor->GetEditorSubsystem<UUnrealMCPBridge>();
		if (Bridge)
		{
			bCachedIsRunning = Bridge->IsRunning();
			CachedAddress = Bridge->GetServerAddress();
			CachedPort = Bridge->GetServerPort();
		}
	}

	ChildSlot
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PanelTitle", "UnrealMCP Control Panel"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 0.0f)
		[
			SNew(SBox)
			.HeightOverride(1.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("WhiteBrush"))
				.ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
			]
		]

		// Status section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f, 10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("StatusLabel", "Status: "))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SUnrealMCPPanel::GetStatusText)
				.ColorAndOpacity(this, &SUnrealMCPPanel::GetStatusColor)
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]
		]

		// Address section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AddressLabel", "Address: "))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SUnrealMCPPanel::GetAddressText)
			]
		]

		// Port section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PortLabel", "Port: "))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(5.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &SUnrealMCPPanel::GetPortText)
			]
		]

		// Separator
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 10.0f)
		[
			SNew(SBox)
			.HeightOverride(1.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("WhiteBrush"))
				.ColorAndOpacity(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f))
			]
		]

		// Control buttons
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(10.0f, 5.0f, 10.0f, 10.0f)
		.HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 10.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("StartServer", "Start Server"))
				.IsEnabled(this, &SUnrealMCPPanel::IsServerRunning)
				.OnClicked(this, &SUnrealMCPPanel::OnStartServerClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("StopServer", "Stop Server"))
				.IsEnabled_Lambda([this]() { return !IsServerRunning(); })
				.OnClicked(this, &SUnrealMCPPanel::OnStopServerClicked)
			]
		]

		// Info section
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(10.0f, 10.0f)
		.VAlign(VAlign_Top)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("InfoTitle", "How to use:"))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 5.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Info1", "1. Server starts automatically on editor launch"))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Info2", "2. Connect from Claude Code using MCP"))
				.AutoWrapText(true)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Info3", "3. claude mcp add unreal -- py <path>"))
				.AutoWrapText(true)
			]
		]
	];
}

void SUnrealMCPPanel::Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Update cached status periodically (every 0.5 seconds)
	UpdateTimeAccumulator += InDeltaTime;
	if (UpdateTimeAccumulator >= 0.5f)
	{
		UpdateTimeAccumulator = 0.0f;

		if (GEditor)
		{
			UUnrealMCPBridge* Bridge = GEditor->GetEditorSubsystem<UUnrealMCPBridge>();
			if (Bridge)
			{
				bCachedIsRunning = Bridge->IsRunning();
				CachedAddress = Bridge->GetServerAddress();
				CachedPort = Bridge->GetServerPort();
			}
		}
	}
}

FText SUnrealMCPPanel::GetStatusText() const
{
	return bCachedIsRunning
		? LOCTEXT("StatusRunning", "Connected")
		: LOCTEXT("StatusStopped", "Disconnected");
}

FSlateColor SUnrealMCPPanel::GetStatusColor() const
{
	return bCachedIsRunning
		? FLinearColor(0.2f, 0.8f, 0.2f, 1.0f)  // Green
		: FLinearColor(0.8f, 0.2f, 0.2f, 1.0f);  // Red
}

FText SUnrealMCPPanel::GetAddressText() const
{
	return FText::FromString(CachedAddress.IsEmpty() ? FString("127.0.0.1") : CachedAddress);
}

FText SUnrealMCPPanel::GetPortText() const
{
	return FText::FromString(FString::FromInt(CachedPort));
}

FReply SUnrealMCPPanel::OnStartServerClicked()
{
	if (GEditor)
	{
		UUnrealMCPBridge* Bridge = GEditor->GetEditorSubsystem<UUnrealMCPBridge>();
		if (Bridge)
		{
			Bridge->StartServer();
			bCachedIsRunning = Bridge->IsRunning();
		}
	}
	return FReply::Handled();
}

FReply SUnrealMCPPanel::OnStopServerClicked()
{
	if (GEditor)
	{
		UUnrealMCPBridge* Bridge = GEditor->GetEditorSubsystem<UUnrealMCPBridge>();
		if (Bridge)
		{
			Bridge->StopServer();
			bCachedIsRunning = Bridge->IsRunning();
		}
	}
	return FReply::Handled();
}

bool SUnrealMCPPanel::IsServerRunning() const
{
	return !bCachedIsRunning;
}

#undef LOCTEXT_NAMESPACE
