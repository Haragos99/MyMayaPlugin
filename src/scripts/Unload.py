import socket

def unload_plugin(plugin_name, port=7001):
    try:
        with socket.create_connection(('localhost', port)) as s:
            cmd = f'import maya.cmds as cmds; cmds.unloadPlugin("{plugin_name}")\n'
            s.sendall(cmd.encode('utf-8'))
            print(f"Sent unload command for plugin: {plugin_name}")
    except Exception as ex:
        print(f"Failed to send command: {ex}")



# Load the Plugin 
unload_plugin('MyMayaPlugin')
