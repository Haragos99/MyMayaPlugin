import maya.cmds as cmds

# Load the plugin by name
if not cmds.pluginInfo('MyMayaPlugin', query=True, loaded=True):
    cmds.loadPlugin(r"C:\Users\Geri\Documents\Projects\CG\MyMayaPlugin\out\build\x64-Debug\Debug\MyMayaPlugin.mll")
# Execute the custom command
start_time = cmds.timerX()
cmds.helloMaya()
elapsed_time = cmds.timerX(startTime=start_time)
print(f"Laplacian smoothing took {elapsed_time:.4f} seconds")