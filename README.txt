=================================================
			Grapigs Engine
-------------------------------------------------
		Jina Hyun (j.hyun@digipen.edu)
 	  Jinwoo Choi (jinwoo.choi@digipen.edu)
=================================================

1. Introduction
Graphics engine simulates object using PBR and IBL technique.
Library	: GLEW(3.3.8), GLEW(2.2.0), ImGui(1.88), FBXSDK 2020.0, ImGuizmo(1.83)

2. How to launch
	- Open 'GrapigsEngine.exe' in 400-GrapigsEngine folder.
	- There must be three directories in the folder: model, shader, and texture


3. How to control
    - Drag [Right Mouse Button]: Rotate around the origin
    - [Ctrl] + Drag [Right Mouse Button]: Move Forward/Backward along Y-axis
    - Drag [Middle Mouse Button]: Rotate around Camera
    - [Ctrl] + Drag [Middle Mouse Button]: Move Camera
    - [R]: Reset Camera
    - [Mouse Wheel Down/Up]: Zoom In/Out

Drag and Drop '*.fbx' for model and '*.png, *.jpg' for texture to import assets. The program support loading multiple files at once.

4. Windows
[Mesh] Window
	- It shows the hierarchy of the mesh.

[Material] Window
	- By dragging and dropping a mesh from [Mesh] window, you can see the detail of the mesh.

[Imported Asset] Window
	- There can be seen all loaded textures
	- You can directly apply a texture by dragging and dropping to [Material] window or by clicking button

[Transform] Window
	- Translate, rotate, or scale the object

[Scene] Window
	- There are four buttons for gizmo; cursor, translation, rotation, and scaling.

There is 'Windows' menu on the top-left corner of screen. You can open/close any window in the menu.


© 2022 DigiPen (USA) Corporation
