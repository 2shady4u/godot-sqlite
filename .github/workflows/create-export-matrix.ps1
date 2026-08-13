param (
    # Expects a string array directly
    [string[]]$JobNames = @()
)

$RawMatrix = Get-Content -Raw -Path .github/workflows/export_var.json | ConvertFrom-Json
$Jobs = $RawMatrix.jobs

$Matrix = @()
foreach ($job in $Jobs) {
    if ($JobNames -notcontains $job.name) {
        continue
    }
    $Matrix += $job
}

Write-Host (ConvertTo-JSON -InputObject $Matrix)
Write-Output "matrix=$(ConvertTo-JSON -InputObject $Matrix -Compress)" >> $env:GITHUB_OUTPUT
