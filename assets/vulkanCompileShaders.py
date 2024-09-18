import os, subprocess, sys
targetPath = os.path.dirname(__file__)

# vertexShaderFiles = [f"{targetPath}/vulkanV.glsl", f"{targetPath}/vulkan/testV.glsl"]
# fragmentShaderFiles = [f"{targetPath}/vulkanF.glsl", f"{targetPath}/vulkan/testF.glsl"]
# vertexShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(os.path.dirname(__file__)) for f in filenames if "vulkan" in dp and f.endswith("V.glsl")]
# fragmentShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(os.path.dirname(__file__)) for f in filenames if "vulkan" in dp and f.endswith("F.glsl")]

vertexShaderFiles = [sys.argv[1]]
fragmentShaderFiles = [sys.argv[2]]

try:
	print(f"[Python] Compiling vertex shaders: {vertexShaderFiles}")
	for vsf in vertexShaderFiles:
		subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=vert", vsf, "-o", vsf.replace(".glsl", ".spv")])
	print(f"[Python] Compiling fragment shaders: {fragmentShaderFiles}")
	for fsf in fragmentShaderFiles:
		subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=frag", fsf, "-o", fsf.replace(".glsl", ".spv")])
except:
	pass