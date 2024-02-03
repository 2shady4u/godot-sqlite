$RawMatrix = Get-Content -Raw -Path .github/workflows/build_var.json | ConvertFrom-Json

$Targets = $RawMatrix.targets
$Jobs = $RawMatrix.jobs

$Matrix = @()
foreach ($job in $Jobs)
{
  if ($job.skip -eq $true) {
    continue
  }
  foreach ($target in $Targets) 
  {
    $MatrixJob = $job.PsObject.Copy()
    $MatrixJob | Add-Member -MemberType NoteProperty -Name 'target' -Value $target
    $Matrix += $MatrixJob
  }
}

Write-Host (ConvertTo-JSON -InputObject $Matrix)
Write-Output "matrix=$(ConvertTo-JSON -InputObject $Matrix -Compress)" >> $env:GITHUB_OUTPUT