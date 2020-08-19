directionalLightCount = 0
pointLightCount = 0
pLights = []
dLights = []

def getFileContents(filePath):
	file = open(filePath, 'r')
	a = file.read()
	file.close()
	return a

def getLightData(filePath):
	global directionalLightCount
	global pointLightCount
	global pLights
	global dLights
	file = open(filePath, 'r')
	content = file.readlines()
	file.close()
	matrix = []
	for line in content:
		elements = line[:-1].split(" ") if line[len(line) - 1] == '\n' else line.split(" ")
		if elements[0] == 'p':
			pointLightCount += 1
			pLights.append(elements)
		else:
			directionalLightCount += 1
			dLights.append(elements)

firstPart = getFileContents("part1.txt")
secondPart = getFileContents("part2.txt")
thirdPart = getFileContents("part3.txt")

pLightCalc = getFileContents("pLightSpecific.txt")
dLightCalc = getFileContents("dLightSpecific.txt")
lightCalc = getFileContents("lightCalc.txt")

output = open("../../shaders/gpbrF.shader", 'w')

getLightData("../../lighting/lights.l")

output.write(firstPart)

#######################
# point lights uniform

if pointLightCount > 0:
	output.write("\n\nuniform vec3 pLightPos[{}] = vec3[](".format(pointLightCount))
	temp = "vec3("
	for pl in pLights:
		temp += pl[1] + ", "
		temp += pl[2] + ", "
		temp += pl[3] + "), vec3("
	output.write(temp[:-7] + ");")

	output.write("\nuniform vec3 pLightRad[{}] = vec3[](".format(pointLightCount))
	temp = "vec3("
	for pl in pLights:
		temp += pl[4] + ", "
		temp += pl[5] + ", "
		temp += pl[6] + "), vec3("
	output.write(temp[:-7] + ");")

	output.write("\nuniform float pLightRa[{}] = float[](".format(pointLightCount))
	temp = ""
	for pl in pLights:
		temp += pl[7] + ", "
	output.write(temp[:-2] + ");")

#######################
# directional lights uniform

if directionalLightCount > 0:
	output.write("\n\nuniform vec3 dLightDir[{}] = vec3[](".format(directionalLightCount))
	temp = "vec3("
	for dl in dLights:
		temp += dl[1] + ", "
		temp += dl[2] + ", "
		temp += dl[3] + "), vec3("
	output.write(temp[:-7] + ");")

	output.write("\nuniform vec3 dLightRad[{}] = vec3[](".format(directionalLightCount))
	temp = "vec3("
	for dl in dLights:
		temp += dl[4] + ", "
		temp += dl[5] + ", "
		temp += dl[6] + "), vec3("
	output.write(temp[:-7] + ");")

output.write(secondPart)

if pointLightCount > 0:
	output.write("\n\n\tfor (int i = 0; i < {}; i++) // for each point light\n\t".format(pointLightCount) + "{\n")
	output.write(pLightCalc);
	output.write(lightCalc + "\n\t}")

if directionalLightCount > 0:
	output.write("\n\n\tfor (int i = 0; i < {}; i++) // for each directional light\n\t".format(directionalLightCount) + "{\n")
	output.write(dLightCalc);
	output.write(lightCalc + "\n\t}")

output.write(thirdPart)

output.close()