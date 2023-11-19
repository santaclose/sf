import os
targetPath = os.path.dirname(__file__)
vertexShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(f"{targetPath}/shaders") for f in filenames if f.endswith("V.glsl")]
fragmentShaderFiles = [os.path.join(dp, f).replace('\\', '/') for dp, dn, filenames in os.walk(f"{targetPath}/shaders") for f in filenames if f.endswith("F.glsl")]

mainFunctionTemplate = """
void main()
{
#ifdef VERTEX
	switch (VERTEX_SHADER_ID)
#else
	switch (FRAGMENT_SHADER_ID)
#endif
	{
__CASES__	}
}
"""[1:-1]

uniformBlockTemplate = """
layout(set = 0, binding = 0) uniform UniformBlock {
__UNIFORM_VARIABLES__};
"""[1:-1]

def extractShaderDeclaration(glslCode, targetSet, varType):
	for i in reversed(range(len(glslCode))):
		foundDeclaration = glslCode[i:i + len(varType) + 2] == f'\n{varType} '
		if foundDeclaration:
			dts = glslCode.find(' ', i) + 1
			dte = glslCode.find(' ', dts)
			dt = glslCode[dts:dte]
			sci = glslCode.find(';', dts)
			vni = glslCode.rfind(' ', 0, sci) + 1

			if varType == "in" or varType == "out":
				targetSet.add(glslCode[i + 1:sci] + dt)
				glslCode = glslCode.replace(glslCode[vni:sci], glslCode[vni:sci] + dt)
			else:
				targetSet.add(glslCode[i + 1:sci])

			glslCode = glslCode[:i] + glslCode[glslCode.find(';', dts) + 1:]
	return glslCode, targetSet

with open(f"{targetPath}/shaders/z_vulkan.glsl", 'r') as vkheaderFile:
	shaderHeader = vkheaderFile.read()


vertexOutFragmentInLines = set()


vertexUniformLines = set()
vertexOutLines = set()
combinedVertexShaders = ""
for i, vsf in enumerate(vertexShaderFiles):
	with open(vsf, 'r') as vsfd:
		vsc = vsfd.read()
	vsc, vertexUniformLines = extractShaderDeclaration(vsc, vertexUniformLines, 'uniform')
	vsc, vertexOutLines = extractShaderDeclaration(vsc, vertexOutLines, 'out')
	vertexOutFragmentInLines = vertexOutFragmentInLines.union(set([x.replace('out ', '') for x in vertexOutLines]))
	combinedVertexShaders += '\n' + vsc.replace("void main()", f"void main{i}()")

fragmentUniformLines = set()
fragmentInLines = set()
fragmentOutLines = set()
combinedFragmentShaders = ""
for i, fsf in enumerate(fragmentShaderFiles):
	with open(fsf, 'r') as fsfd:
		fsc = fsfd.read()
	fsc, fragmentUniformLines = extractShaderDeclaration(fsc, fragmentUniformLines, 'uniform')
	fsc, fragmentInLines = extractShaderDeclaration(fsc, fragmentInLines, 'in')
	fsc, fragmentOutLines = extractShaderDeclaration(fsc, fragmentOutLines, 'out')
	assert(all(x.replace('in ', '') in vertexOutFragmentInLines for x in fragmentInLines))
	combinedFragmentShaders += '\n' + fsc.replace("void main()", f"void main{i}()")


with open(f"{targetPath}/vulkanV.glsl", 'w') as vsfd:
	vsfd.write("#version 450\n#define VERTEX\n")
	vsfd.write(shaderHeader)
	vsfd.write('\n')
	vsfd.write(uniformBlockTemplate.replace("__UNIFORM_VARIABLES__", "".join([f"\t{line.replace('uniform ', '').replace(' = ', '; //= ')};\n" for line in vertexUniformLines if "sampler" not in line])))
	vsfd.write('\n')
	for i, line in enumerate([line for line in vertexUniformLines if "sampler" in line]):
		vsfd.write(f"layout(binding = {i+1}) {line};\n")
	vsfd.write("layout(location = 0) out VertexOut {\n")
	for i, line in enumerate(vertexOutFragmentInLines):
		vsfd.write(f"\t{line};\n")
	vsfd.write("};\n")
	vsfd.write(combinedVertexShaders + '\n')
	cases = ""
	for i in range(len(vertexShaderFiles)):
		cases += f"\t\tcase {i}: main{i}(); break; // {vertexShaderFiles[i]}\n"
	vsfd.write(mainFunctionTemplate.replace("__CASES__", cases) + '\n')


with open(f"{targetPath}/vulkanF.glsl", 'w') as fsfd:
	fsfd.write("#version 450\n#define FRAGMENT\n")
	fsfd.write(shaderHeader)
	fsfd.write('\n')
	fsfd.write(uniformBlockTemplate.replace("__UNIFORM_VARIABLES__", "".join([f"\t{line.replace('uniform ', '').replace(' = ', '; //= ')};\n" for line in fragmentUniformLines if "sampler" not in line])))
	fsfd.write('\n')
	for i, line in enumerate([line for line in fragmentUniformLines if "sampler" in line]):
		fsfd.write(f"layout(binding = {i+1}) {line};\n")
	fsfd.write("layout(location = 0) in VertexOut {\n")
	for line in vertexOutFragmentInLines:
		fsfd.write(f"\t{line};\n")
	fsfd.write("};\n")
	for i, line in enumerate(fragmentOutLines):
		fsfd.write(f'layout(location = {i}) {line};\n')
	fsfd.write(combinedFragmentShaders + '\n')
	cases = ""
	for i in range(len(fragmentShaderFiles)):
		cases += f"\t\tcase {i}: main{i}(); break; // {fragmentShaderFiles[i]}\n"
	fsfd.write(mainFunctionTemplate.replace("__CASES__", cases) + '\n')