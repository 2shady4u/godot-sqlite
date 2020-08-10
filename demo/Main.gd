extends Control

func _enter_tree():
	var _error : int = $Database.connect("output_received", self, "_on_output_received")

func _on_output_received(text : String) -> void:
	var label := Label.new()
	$MarginContainer/VBoxContainer/ScrollContainer/VBoxContainer.add_child(label)

	label.text = text
	label.set("custom_colors/font_color", Color.limegreen)
	label.autowrap = true
