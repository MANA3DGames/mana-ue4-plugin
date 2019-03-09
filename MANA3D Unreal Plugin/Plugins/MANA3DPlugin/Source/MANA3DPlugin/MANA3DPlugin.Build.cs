using UnrealBuildTool;
using System;
using System.IO;

public class MANA3DPlugin : ModuleRules
{
    public MANA3DPlugin( TargetInfo target )
    {
        PrivateIncludePaths.AddRange( new[] { "MANA3DPlugin/Private" } );

        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "Engine",
                "UnrealEd",
                "InputCore",
                "Core",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "CoreUObject",
				"LevelEditor",
				"Json",
                "JsonUtilities",
                "Networking",
                "Sockets",
                "HTTP",
                "PropertyEditor",
                "AssetTools",
                "MeshUtilities"
            }
        );

		var publicPath = Path.GetFullPath( Path.Combine( ModuleDirectory, "Public" ) );

		PublicAdditionalLibraries.Add( Path.Combine( publicPath, @"lib\libcurl.Lib") );

		var include_path = Path.GetFullPath( Path.Combine( publicPath, "includes" ) );
        PublicIncludePaths.Add( include_path );
        PublicIncludePaths.Add( Path.Combine( include_path, "zlib" ) );
		
        PublicDependencyModuleNames.AddRange( new[] { "zlib", "libcurl" } );
    }
}