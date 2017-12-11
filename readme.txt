Members:
	Ernest Gaddi - ernest.gaddi.491@my.csun.edu

Target OS:
	Windows
API's used:
	Visual Studio 2017
	freeGLUT
	GLEW
	GLM
	includes465 files by Professor Barnes
	
Models:
	Blender Raws (custom program used to add color (Phase 2) or by hand (Phase 1 & 3) )

Game Logic/Rendering
	objectMVP: model data that is needed to create model matrix
		view matrix created in display function
		projection matrix created in reshape function
	ship, site, projectile: overlying logic of moving parts
		collisions handled by outside functions using bounding radius with relevant objects as args
	starfield does NOT use MVP or normal matrices as it is static relative to camera
	inputStructure: structure used to handle inputs to store data frame-to-frame
	game logic handled in update function
	title changed in updateTitle and called every update
	display render order:
		starfield (IMPORTANT)
		astral objects
		warship
		missile sites
		missiles

Starfield Notes
	large plane drawn first to cover whole field of view then depth buffer cleared for other objects to be drawn over
	since it is stationary relative to camera all matrixes are identity
	fragment shader uses screen coordinates for texture coordinates

Lighting:
	two diffuse lights/ no toggle b/c why would you
	camera lighting uses unmodified directional lighting without accounting for camera space ( i.e. wholly in fragment shader )
		lighting uses dot product shifted towards one to allow more light on orthogonal normal surfaces
	ruber light is transformed into view space in vertex shader
		ruber light is simplified into binary shading by checking whether it meets the requirement to be shaded ( dot product > 0.25 )
			if it does it is fully lighted; otherwise it is dark
			*since skybox is fixed, use this strong lighting from ruber on ship for relative placement in space
		ambient light is weighted with diffuse weighted relative to it