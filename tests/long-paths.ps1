param(
    [Parameter(Mandatory = $true)]
    [string] $Jq
)

$ErrorActionPreference = 'Stop'
$jqPath = (Resolve-Path -LiteralPath $Jq).Path
$policy = Get-ItemPropertyValue -LiteralPath 'HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem' -Name LongPathsEnabled
if ($policy -ne 1) {
    throw 'This test requires LongPathsEnabled=1.'
}

$tempPath = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
$root = Join-Path $tempPath ('jq-long-path-' + [Guid]::NewGuid().ToString('N'))
try {
    $directory = $root
    foreach ($i in 1..6) {
        $directory = Join-Path $directory ("segment$i" + ('x' * 50))
    }
    [IO.Directory]::CreateDirectory($directory) | Out-Null
    $inputFile = Join-Path $directory 'input.json'
    [IO.File]::WriteAllText($inputFile, '{"ok":true}')
    if ($inputFile.Length -le 260) {
        throw 'Test input path must exceed MAX_PATH.'
    }
    $output = & $jqPath -e -c '.ok' $inputFile
    if ($LASTEXITCODE -ne 0 -or $output -ne 'true') {
        throw "jq failed to read a $($inputFile.Length)-character path."
    }
}
finally {
    # Delete only the temporary directory created by this test.
    $resolvedRoot = [IO.Path]::GetFullPath($root)
    if ([IO.Path]::GetDirectoryName($resolvedRoot) -ne $tempPath.TrimEnd('\', '/')) {
        throw 'Unexpected temporary directory location.'
    }
    if (Test-Path -LiteralPath $resolvedRoot) {
        Remove-Item -LiteralPath $resolvedRoot -Recurse -Force
    }
}
