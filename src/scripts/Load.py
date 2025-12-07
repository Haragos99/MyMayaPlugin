import maya.cmds as cmds

# For debug mode
path = r"C:\Users\Geri\Documents\Projects\CG\MyMayaPlugin\out\build\x64-Debug\Debug\MyMayaPlugin.mll"
path2 = r"C:\Users\Geri\Documents\Projects\CG\MyMayaPlugin\out\build\x64-Release\MyMayaPlugin.mll"
# Load the plugin by name
if not cmds.pluginInfo(path2, query=True, loaded=True):
    cmds.loadPlugin(path2)
# Execute the custom command
start_time = cmds.timerX()
cmds.creataCustomDeltaMush()
elapsed_time = cmds.timerX(startTime=start_time)
print(f"Laplacian smoothing took {elapsed_time:.4f} seconds")