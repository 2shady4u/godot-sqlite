new-item -Name build -itemType directory
godot -s export_data.gd
godot -e --export-debug "Windows Desktop" build/godot-sqlite.exe
