#! /usr/bin/python

import os
import sys
import subprocess


## Process inputs
iterations = sys.argv[1]
optLevel = sys.argv[2]

loopTracker = subprocess.Popen("./for_loop_tracker " + iterations + " " + optLevel,
								# stdout=subprocess.PIPE, #silence IR output
								stderr=subprocess.PIPE, 
								shell=True)
(output, err) = loopTracker.communicate()

## Parse llvm errs() output to find dag file
startPath = err.find(" '") + 2
endPath = err.find("'...")
filePath = err[startPath:endPath]

## Helper variables
splitIndex = filePath.find("dag.")
fileDir = filePath[:splitIndex]
fileName = filePath[splitIndex:]

## Produce DAG
imgDirPath = "images/"
imgDirExists = os.path.isdir("./" + imgDirPath)
if imgDirExists is not True:
	print "Creating image directory..."
	createImgDir = os.mkdir(imgDirPath)

print "Moving and converting .dot file..."
copyFile = subprocess.Popen("cp " + filePath + " " + imgDirPath, shell=True)
# copyFile.wait()
removeFile = subprocess.Popen("rm " + filePath, shell=True)
# removeFile.wait()
genDag = subprocess.Popen("dot -T png -O " + imgDirPath + fileName, shell=True)
genDag.wait()

print "Opening newly created DAG file..."
newFileName = fileName[:fileName.find("-")] #remove identifier
fileNamePng = newFileName + ".png"
renameFile = os.rename(imgDirPath + fileName + ".png", imgDirPath + fileNamePng)
openDag = subprocess.Popen("open " + imgDirPath + fileNamePng, shell=True)
removeDotFile = subprocess.Popen("rm " + imgDirPath + "*.dot", shell=True)