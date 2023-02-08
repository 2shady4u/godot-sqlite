extends Control

func _enter_tree():
    var _error : int = $Database.connect("output_received", self, "_on_output_received")
    _error = $Database.connect("texture_received", self, "_on_texture_received")

func _on_output_received(text : String) -> void:
    var label := Label.new()
    $MarginContainer/VBoxContainer/ScrollContainer/VBoxContainer.add_child(label)

    label.text = text
    label.set("custom_colors/font_color", Color.limegreen)
    label.autowrap = true

func _on_texture_received(texture : Texture) -> void:
    var texture_rect := TextureRect.new()
    $MarginContainer/VBoxContainer/ScrollContainer/VBoxContainer.add_child(texture_rect)

    texture_rect.texture = texture
