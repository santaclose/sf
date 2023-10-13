import os, subprocess
vertexShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(os.path.dirname(__file__)) for f in filenames if "vulkan" in dp and f.endswith("V.glsl")]
fragmentShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(os.path.dirname(__file__)) for f in filenames if "vulkan" in dp and f.endswith("F.glsl")]
print(f"[Python] Compiling vertex shaders: {vertexShaderFiles}")
print(f"[Python] Compiling fragment shaders: {fragmentShaderFiles}")
try:
	for vsf in vertexShaderFiles:
		subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=vert", vsf, "-o", vsf.replace(".glsl", ".spv")])
	for fsf in fragmentShaderFiles:
		subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=frag", fsf, "-o", fsf.replace(".glsl", ".spv")])
except:
	pass