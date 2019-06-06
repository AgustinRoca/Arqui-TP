param(
    [Array] $m = @(),
    [Switch] $h = $false,
    [Switch] $help = $false
)

Clear-Host

if ($h -eq $true -or $help -eq $true) {
    Write-Host "This utility makes a git pull, add ., commit and push to origin master."
    Write-Host "Usage: uploadToGit.ps1 -m `"Title`" [`"Description`"]`n"
    exit
}

if ($m.Count -le 0) {
    throw "-m is required"
}

Write-Host "[*] Pulling commits from repo..." -ForegroundColor Yellow
git pull
if ($? -eq $false) {
    Write-Error "`nError pulling commits from repo."
    exit
}
Write-Host "[*] Done pulling commits from repo." -ForegroundColor Green

Write-Host "`n[*] Adding files in directory..." -ForegroundColor Yellow
git add .
if ($? -eq $false) {
    Write-Error "`nError adding files in directory."
    exit
}
Write-Host "[*] Done adding files." -ForegroundColor Green

Write-Host "`n[*] Committing..." -ForegroundColor Yellow
if ($m.Length -eq 2) {
    git commit -m $m[0] -m $m[1]
} else {
    git commit -m $m[0]
}
if ($? -eq $false) {
    Write-Error "`nError committing."
    exit
}
Write-Host "[*] Done committing." -ForegroundColor Green

Write-Host "`n[*] Pushing to origin master..." -ForegroundColor Yellow
git push origin master
if ($? -eq $false) {
    Write-Error "`nError pushing to origin master."
    exit
}
Write-Host "[*] Done pushing." -ForegroundColor Green
