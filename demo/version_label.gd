@tool
extends Label

const TEMPLATE := """Godot: %s
Godot-SQLite: %s"""
const CONFIG_PATH := "res://addons/godot-sqlite/plugin.cfg"

func _ready() -> void:
	text = TEMPLATE % [Engine.get_version_info()["string"], get_plugin_version()]

func get_plugin_version() -> String:
	var config := ConfigFile.new()

	var err := config.load(CONFIG_PATH)
	if err != OK:
		push_error("Failed to load plugin.cfg (Error code: %d)" % err)
		return ""

	return config.get_value("plugin", "version", "Unknown")
