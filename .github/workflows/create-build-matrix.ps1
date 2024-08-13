$RawMatrix = Get-Content -Raw -Path .github/workflows/build_var.json | ConvertFrom-Json

$Targets = $RawMatrix.targets
$CommonFlags = $RawMatrix.common_flags
$Jobs = $RawMatrix.jobs

$Matrix = @()
foreach ($job in $Jobs) {
  if ($job.skip -eq $true) {
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
}

Write-Host (ConvertTo-JSON -InputObject $Matrix)
Write-Output "matrix=$(ConvertTo-JSON -InputObject $Matrix -Compress)" >> $env:GITHUB_OUTPUT