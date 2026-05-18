$ErrorActionPreference = "Stop"

$root = "C:\GITHUB\L2Modder_V2"

$tools = [ordered]@{
    Root = $root
    Clients = [ordered]@{
        H5 = Join-Path $root "Kliensek\Lineage II H5 Custom"
        Fafurion = Join-Path $root "Kliensek\FULL CLIENT LINEAGE2 FAFURION REV 166 EU"
        Homonkulus = Join-Path $root "Kliensek\L2NAP286D20201216G269"
    }
    PackageDecoders = [ordered]@{
        L2EncDec = Join-Path $root "L2FileEdit\data\l2encdec\l2encdec.exe"
        L2EncDecOld = Join-Path $root "L2FileEdit\data\l2encdec\l2encdec_old.exe"
        Loader = Join-Path $root "L2FileEdit\data\l2encdec\loader.exe"
        LoaderCT1Plus = Join-Path $root "L2FileEdit\data\l2encdec\loaderCT1++.exe"
    }
    DatTools = [ordered]@{
        L2Asm = Join-Path $root "L2FileEdit\data\l2asm-disasm\l2asm.exe"
        L2Disasm = Join-Path $root "L2FileEdit\data\l2asm-disasm\l2disasm.exe"
        L2ClientDat = Join-Path $root "L2ClientDat"
        XDatEditor = Join-Path $root "xdat_editor"
    }
    UnrealAssetTools = [ordered]@{
        L2AssetViewer = Join-Path $root "L2AssetViewer"
        UnrealPackageDump = Join-Path $root "L2AssetViewer\tools\UnrealPackageDump.java"
        UnrealPackageIndexJson = Join-Path $root "L2AssetViewer\tools\UnrealPackageIndexJson.java"
        TextureProbe = Join-Path $root "L2AssetViewer\tools\TextureProbe.java"
        TextureExtract = Join-Path $root "L2AssetViewer\tools\TextureExtract.java"
        UnrealEffectMeshRefs = Join-Path $root "L2AssetViewer\tools\UnrealEffectMeshRefs.java"
        ConvertPskToGlb = Join-Path $root "L2AssetViewer\tools\convert_psk_to_glb.py"
        ConvertUnreal3dToGlb = Join-Path $root "L2AssetViewer\tools\convert_unreal_3d_to_glb.py"
        UModel64 = Join-Path $root "tools\umodel\umodel_64.exe"
    }
    GeodataReferences = [ordered]@{
        GeoEngine = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\GeoEngine.java"
        Cell = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\Cell.java"
        IBlock = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\IBlock.java"
        IRegion = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\IRegion.java"
        Region = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\regions\Region.java"
        FlatBlock = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\blocks\FlatBlock.java"
        ComplexBlock = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\blocks\ComplexBlock.java"
        MultilayerBlock = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive\java\org\l2jmobius\gameserver\geoengine\geodata\blocks\MultilayerBlock.java"
    }
    EditorReferences = [ordered]@{
        L2Editor = Join-Path $root "L2Editor"
        L2wTool = Join-Path $root "l2w_tool"
        Ue2ReadWrite = Join-Path $root "l2w_tool\l2_rw\src\ue2_rw.rs"
        SpawnBackend = Join-Path $root "l2w_tool\spawn_editor\src\backend.rs"
        SpawnFrontend = Join-Path $root "l2w_tool\spawn_editor\src\frontend.rs"
    }
    ServerReferences = [ordered]@{
        HighFiveBuild = Join-Path $root "L2J_Mobius_CT_2.6_HighFive_build"
        HighFiveSource = Join-Path $root "source\L2J_Mobius_CT_2.6_HighFive"
        FafurionSchemas = Join-Path $root "L2jSedonaStudio\resources\xsd-catalog\L2J_Mobius_06.0_Fafurion"
        HomunculusSchemas = Join-Path $root "L2jSedonaStudio\resources\xsd-catalog\L2J_Mobius_08.2_Homunculus"
    }
}

function Convert-Node {
    param($Node)

    if ($Node -is [System.Collections.IDictionary]) {
        $result = [ordered]@{}
        foreach ($key in $Node.Keys) {
            $result[$key] = Convert-Node $Node[$key]
        }
        return $result
    }

    [pscustomobject]@{
        Path = $Node
        Exists = Test-Path -LiteralPath $Node
    }
}

Convert-Node $tools | ConvertTo-Json -Depth 8
