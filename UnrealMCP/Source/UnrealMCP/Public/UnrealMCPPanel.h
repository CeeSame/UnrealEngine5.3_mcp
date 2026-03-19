#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * Slate panel widget for UnrealMCP status display and control.
 * Shows connection status, server address, and provides control buttons.
 */
class SUnrealMCPPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUnrealMCPPanel) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** Tick to update status */
	virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

private:
	/** Get status text (Running/Stopped) */
	FText GetStatusText() const;

	/** Get status color (green/red) */
	FSlateColor GetStatusColor() const;

	/** Get server address text */
	FText GetAddressText() const;

	/** Get port number text */
	FText GetPortText() const;

	/** Handle start server button click */
	FReply OnStartServerClicked();

	/** Handle stop server button click */
	FReply OnStopServerClicked();

	/** Check if server is running (for button enable state) */
	bool IsServerRunning() const;

	/** Time accumulator for periodic updates */
	float UpdateTimeAccumulator = 0.0f;

	/** Cached status for display (updated periodically) */
	bool bCachedIsRunning = false;
	FString CachedAddress;
	int32 CachedPort = 55557;
};
