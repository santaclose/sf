import os
import re
import sys
import subprocess
targetPath = os.path.dirname(__file__)

vertShaderPath = sys.argv[1]
fragShaderPath = sys.argv[2]

print(f"[Python] Compiling vertex shader: {vertShaderPath}")
vertProc = subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=vert", vertShaderPath, "-o", vertShaderPath.replace(".glsl", ".spv")])
print(f"[Python] Compiling fragment shader: {fragShaderPath}")
fragProc = subprocess.run([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=frag", fragShaderPath, "-o", fragShaderPath.replace(".glsl", ".spv")])
if fragProc.returncode != 0 or vertProc.returncode != 0:
	print("[Python] Failed to compile, exiting")
	sys.exit(1)

VERTEX_ATTRIBUTE_PATTERN = r'^layout\(location *= *(\d+)\) *in (\w+) (\w+).*$'
UNIFORM_BUFFER_PATTERN = r'^layout\(binding *= *(\d+)\) *uniform.*$'
STORAGE_BUFFER_PATTERN = r'^layout\(binding *= *(\d+)\) *buffer.*$'

vertShaderSource = subprocess.check_output([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=vert", vertShaderPath, "-E"]).decode('utf-8')
vertexAttributes = []
uniformBuffers = []
storageBuffers = []
for line in vertShaderSource.split('\n'):
	vertexAttributeMatch = re.match(VERTEX_ATTRIBUTE_PATTERN, line)
	uniformBufferMatch = re.match(UNIFORM_BUFFER_PATTERN, line)
	storageBufferMatch = re.match(STORAGE_BUFFER_PATTERN, line)
	if vertexAttributeMatch is not None:
		vertexAttributes.append((vertexAttributeMatch.group(1), vertexAttributeMatch.group(2), vertexAttributeMatch.group(3)))
	elif uniformBufferMatch is not None:
		uniformBuffers.append(uniformBufferMatch.group(1))
	elif storageBufferMatch is not None:
		storageBuffers.append(storageBufferMatch.group(1))
with open(vertShaderPath.replace(".glsl", ".input"), 'w') as file:
	file.write("[vertex attributes]\n")
	for x in vertexAttributes:
		file.write(f"{x[0]} {x[1]} {x[2]}\n")
	if len(uniformBuffers) > 0:
		file.write("[uniform buffers]\n")
		for x in uniformBuffers:
			file.write(f"{x}\n")
	if len(storageBuffers) > 0:
		file.write("[storage buffers]\n")
		for x in storageBuffers:
			file.write(f"{x}\n")

fragShaderSource = subprocess.check_output([f"{os.environ['VULKAN_SDK']}/Bin/glslc", "-fshader-stage=frag", vertShaderPath, "-E"]).decode('utf-8')
uniformBuffers = []
storageBuffers = []
for line in fragShaderSource.split('\n'):
	uniformBufferMatch = re.match(UNIFORM_BUFFER_PATTERN, line)
	storageBufferMatch = re.match(STORAGE_BUFFER_PATTERN, line)
	if uniformBufferMatch is not None:
		uniformBuffers.append(uniformBufferMatch.group(1))
	elif storageBufferMatch is not None:
		storageBuffers.append(storageBufferMatch.group(1))
with open(fragShaderPath.replace(".glsl", ".input"), 'w') as file:
	if len(uniformBuffers) > 0:
		file.write("[uniform buffers]\n")
		for x in uniformBuffers:
			file.write(f"{x}\n")
	if len(storageBuffers) > 0:
		file.write("[storage buffers]\n")
		for x in storageBuffers:
			file.write(f"{x}\n")