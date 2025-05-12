$rcFile = "enzotechdevicelogin.rc"

# Read the line with the FileVersion string
$versionLine = Get-Content $rcFile | Where-Object { $_ -match 'VALUE\s+"FileVersion",\s+"(\d+\.\d+\.\d+\.\d+)"' }

if ($versionLine -match '"(\d+\.\d+\.\d+\.\d+)"') {
    $fullVersion = $matches[1]  # e.g., "0.1.1.8"
    $parts = $fullVersion.Split('.')

    if ($parts.Length -eq 4) {
        # Drop the first part (e.g., "0")
        $newVersion = "$($parts[1]).$($parts[2]).$($parts[3])"  # Result: "1.1.8"
        $output = '#define MyAppVersion "' + $newVersion + '"'
        Set-Content -Path "x64\Release\version.iss" -Value $output
        Write-Output "Extracted version: $newVersion"
    } else {
        Write-Error "Unexpected version format: $fullVersion"
    }
} else {
    Write-Error "Could not find version in RC file."
}