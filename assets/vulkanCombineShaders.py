import os
targetPath = os.path.dirname(__file__)
vertexShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(f"{targetPath}/shaders") for f in filenames if f.endswith("V.glsl")]
fragmentShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(f"{targetPath}/shaders") for f in filenames if f.endswith("F.glsl")]

mainFunctionTemplate = """
void main()
{
	switch (SHADER_ID)
	{
__CASES__	}
}
"""[1:-1]

def fixShaderInOuts(glslCode, outLinesSet, outs):
	for i in reversed(range(len(glslCode))):
		foundInOut = glslCode[i:i + 5]  == '\nout ' if outs else glslCode[i:i + 4]  == '\nin '
		if foundInOut:
			dts = glslCode.find(' ', i) + 1
			dte = glslCode.find(' ', dts)
			dt = glslCode[dts:dte]
			sci = glslCode.find(';', dts)
			vni = glslCode.rfind(' ', 0, sci) + 1
			outLinesSet.add(glslCode[i + 1:sci] + dt)
			glslCode = glslCode.replace(glslCode[vni:sci], glslCode[vni:sci] + dt)
			glslCode = glslCode[:i] + glslCode[glslCode.find(';', dts) + 1:]
	return glslCode, outLinesSet

with open(f"{targetPath}/shaders/z_vulkan.glsl", 'r') as vkheaderFile:
	shaderHeader = vkheaderFile.read()

# ---- VERTEX

vertexOutLines = set()
combinedVertexShaders = ""
for i, vsf in enumerate(vertexShaderFiles):
	with open(vsf, 'r') as vsfd:
		vsc = vsfd.read()
	vsc, vertexOutLines = fixShaderInOuts(vsc, vertexOutLines, True)
	combinedVertexShaders += '\n' + vsc.replace("void main()", f"void main{i}()")

with open(f"{targetPath}/vulkanV.glsl", 'w') as vsfd:
	vsfd.write("#version 450\n#define VERTEX\n")
	vsfd.write(shaderHeader)
	vsfd.write('\n')
	for line in vertexOutLines:
		vsfd.write(line + ';\n')
	vsfd.write(combinedVertexShaders + '\n')
	cases = ""
	for i in range(len(vertexShaderFiles)):
		cases += f"\t\tcase {i}: main{i}(); break; // {vertexShaderFiles[i]}\n"
	vsfd.write(mainFunctionTemplate.replace("__CASES__", cases) + '\n')

# ---- FRAGMENT

fragmentInLines = set()
fragmentOutLines = set()
combinedFragmentShaders = ""
for i, fsf in enumerate(fragmentShaderFiles):
	with open(fsf, 'r') as fsfd:
		fsc = fsfd.read()
	fsc, fragmentInLines = fixShaderInOuts(fsc, fragmentInLines, False)
	fsc, fragmentOutLines = fixShaderInOuts(fsc, fragmentOutLines, True)
	combinedFragmentShaders += '\n' + fsc.replace("void main()", f"void main{i}()")


with open(f"{targetPath}/vulkanF.glsl", 'w') as fsfd:
	fsfd.write("#version 450\n#define FRAGMENT\n")
	fsfd.write(shaderHeader)
	fsfd.write('\n')
	for line in fragmentInLines:
		fsfd.write(line + ';\n')
	for line in fragmentOutLines:
		fsfd.write(line + ';\n')
	fsfd.write(combinedFragmentShaders + '\n')
	cases = ""
	for i in range(len(fragmentShaderFiles)):
		cases += f"\t\tcase {i}: main{i}(); break; // {fragmentShaderFiles[i]}\n"
	fsfd.write(mainFunctionTemplate.replace("__CASES__", cases) + '\n')