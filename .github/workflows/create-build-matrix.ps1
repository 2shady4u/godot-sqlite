$RawMatrix = Get-Content -Raw -Path .github/workflows/build_var.json | ConvertFrom-Json

$Targets = $RawMatrix.targets
$CommonFlags = $RawMatrix.common_flags
$Jobs = $RawMatrix.jobs
$DefaultSkip = if ($null -ne $RawMatrix.default_skip) { [bool]$RawMatrix.default_skip } else { $false }

$Matrix = @()
$JobNames = @()
foreach ($job in $Jobs) {
  $Skip = if ($null -ne $job.skip) { [bool]$job.skip } else { $DefaultSkip }
  if ($Skip -eq $true) {
    continue
  }
  foreach ($target in $Targets) {
    $MatrixJob = $job.PsObject.Copy()
    $MatrixJob | Add-Member -MemberType NoteProperty -Name 'target' -Value $target
    # Add the common flags to the job-specific flags
    if ($null -ne $CommonFlags) {
      if ($null -eq $MatrixJob.flags) {
        $MatrixJob | Add-Member -MemberType NoteProperty -Name 'flags' -Value $CommonFlags
      }
      else {
        $MatrixJob.flags += " " + $CommonFlags
      }
    }
    $Matrix += $MatrixJob
  }
  $JobNames += $MatrixJob.name
}

Write-Host (ConvertTo-JSON -InputObject $Matrix)
Write-Output "matrix=$(ConvertTo-JSON -InputObject $Matrix -Compress)" >> $env:GITHUB_OUTPUT

Write-Host (ConvertTo-JSON -InputObject $JobNames)
Write-Output "job_names=$(ConvertTo-JSON -InputObject $JobNames -Compress)" >> $env:GITHUB_OUTPUT
