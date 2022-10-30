$RawMatrix = Get-Content -Raw -Path .github/workflows/build_var.json | ConvertFrom-Json

$Targets = $RawMatrix.targets
$Jobs = $RawMatrix.jobs

$Matrix = @()
foreach ($job in $Jobs)
{
  foreach ($target in $Targets) 
  {
    $MatrixJob = $job.PsObject.Copy()
    $MatrixJob | Add-Member -MemberType NoteProperty -Name 'target' -Value $target
    $Matrix += $MatrixJob
  }
}

Write-Host ($Matrix | ConvertTo-JSON)
Write-Host "::set-output name=matrix::$($Matrix | ConvertTo-JSON -Compress))"