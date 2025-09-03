import maya.cmds as cmds

# For debug mode
path = r"C:\Users\Geri\Documents\Projects\CG\MyMayaPlugin\out\build\x64-Debug\Debug\MyMayaPlugin.mll"
# Load the plugin by name
if not cmds.pluginInfo(path, query=True, loaded=True):
    cmds.loadPlugin(path)
# Execute the custom command
start_time = cmds.timerX()
cmds.helloMaya()
node = cmds.createNode("myLocator")
elapsed_time = cmds.timerX(startTime=start_time)
print(f"Laplacian smoothing took {elapsed_time:.4f} seconds")