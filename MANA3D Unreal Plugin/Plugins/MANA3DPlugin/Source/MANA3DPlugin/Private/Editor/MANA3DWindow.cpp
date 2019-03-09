#include "MANA3DPluginPch.h"
#include "MANA3DWindow.h"
#include "Utilities/StringUtil.h"
#include "Utilities/UEUtil.h"

#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "Editor/DetailCustomizations/Private/DetailCustomizationsPrivatePCH.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "MANA3DPlugin"



MANA3DWindow::MANA3DWindow()
{
	// Create a new instance of MANA3DManager in the current world.
	CreateMANA3DManagerInstance();
}

MANA3DWindow::~MANA3DWindow()
{
}


void MANA3DWindow::CreateMANA3DManagerInstance()
{
	// Search for an instance of MANA3DManager.
	for ( TActorIterator<AMANA3DManager> It( UEUtil::GetWorld() ); It; ++It )
	{
		MANA3DManager = *It;
		return;
	}

	// Create a new MANA3DManager instance.
	MANA3DManager = UEUtil::GetWorld()->SpawnActor<AMANA3DManager>( AMANA3DManager::StaticClass() );
}

void MANA3DWindow::Construct( const FArguments& args )
{	
	TSharedRef<SVerticalBox> vertBox = SNew( SVerticalBox );

	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
	TSharedRef<IDetailsView> PropertyView = EditModule.CreateDetailView( FDetailsViewArgs( false, false, false, FDetailsViewArgs::ActorsUseNameArea, true ) );
	PropertyView->SetObject( MANA3DManager, true );

	ChildSlot
	[
		SNew( SVerticalBox )
		
		+ SVerticalBox::Slot()
		.Padding( 5 )
		[
			SNew( SBorder )
			.Padding( FMargin( 8.0f, 8.0f ) )
			[
				SNew( SVerticalBox )
			
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding( FMargin( 5.0f, 10.0f, 5.0f, 5.0f ) )
				[
					SNew( STextBlock )
					.Text( FText::FromString( "MANA3D Settings" ) )
				]
			
				+ SVerticalBox::Slot()
				.Padding( 5 )
				[
					PropertyView
				]
			]
		]


		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding( FMargin( 5.0f, 10.0f, 5.0f, 5.0f ) )
		[
			vertBox
		]
	];

#pragma region
	vertBox->AddSlot()
		.AutoHeight()
		.Padding( 5, 20 )
		.VAlign( VAlign_Center )
		.HAlign( HAlign_Right )
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign( HAlign_Right )
			.MaxWidth( 250 )
			[
				SNew( SButton )
				.HAlign( EHorizontalAlignment::HAlign_Center )
				.Text( FText::FromString( FString( TEXT( "Bake ( Bake Assets to MANA3D server )" ) ) ) )
				.OnClicked( this, &MANA3DWindow::OnClickBakeAssets )
			]
		];
#pragma endregion [Bake Assets Slot]
}


FReply MANA3DWindow::OnClickBakeAssets()
{
	if ( MANA3DManager != NULL )
		MANA3DManager->Bake( UEUtil::GetWorld() );
	else
		GLog->Log( ELogVerbosity::Error, "There is no instance of MANA3DManager in the scene: MANA3DManager is NULL" );
	return FReply::Handled();
}

FReply MANA3DWindow::OnClickViewActor()
{
	/// *********************************

	// Create a property view
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
	TSharedRef<IDetailsView> PropertyView = EditModule.CreateDetailView( FDetailsViewArgs( false, false, false, FDetailsViewArgs::ActorsUseNameArea, true ) );
	PropertyView->SetObject( MANA3DManager, true );

	// Create the window
	TSharedPtr< SWindow > PropertiesWindow = SNew( SWindow )
		.SupportsMaximize( false )
		.SupportsMinimize( false )
		.SizingRule( ESizingRule::FixedSize )
		.ClientSize( FVector2D( 700.0f, 575.0f ) )
		.Title( LOCTEXT( "PluginMetadata", "Plugin Properties" ) )
		.Content()
		[
			SNew(SBorder)
			.Padding(FMargin(8.0f, 8.0f))
			[
				SNew(SVerticalBox)

				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(5.0f, 10.0f, 5.0f, 5.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromString("Test!!"))
				]

				+ SVerticalBox::Slot()
				.Padding(5)
				[
					PropertyView
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
					SNew(SButton)
					.Text( FText::FromString( FString( TEXT( "Create MANA3DManager" ) ) ) )
					]

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
					SNew(SButton)
					.Text( FText::FromString( FString( TEXT( "View Actor" ) ) ) )
					]
				]
			]
		];
	
	TSharedPtr<SWindow> TopWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if ( TopWindow.IsValid() )
	{
		GLog->Log( "Add as Native" );
		FSlateApplication::Get().AddWindowAsNativeChild( PropertiesWindow.ToSharedRef(), TopWindow.ToSharedRef(), true );
	}
	else
	{
		GLog->Log( "Default in case no top window" );
		FSlateApplication::Get().AddWindow( PropertiesWindow.ToSharedRef() );
	}

	return FReply::Handled();
}


