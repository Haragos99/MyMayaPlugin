import maya.cmds as cmds

WINDOW_NAME = "AnimationImporter"

def bare_name(full_path):
    # Take last segment of path, then strip namespace prefix
    short = full_path.split("|")[-1]   # e.g. "mixamorig:Spine"
    return short.split(":")[-1]  


def import_fbx_animation():

    # --- Get selected root joint ---
    selection = cmds.ls(selection=True, type="joint")
    if not selection:
        cmds.confirmDialog(title="Error", message="Select the ROOT joint of your scene skeleton.", button=["OK"])
        return

    # --- Snapshot BEFORE import ---
    scene_joints_before    = set(cmds.ls(type="joint", long=True))
    scene_assemblies_before = set(cmds.ls(assemblies=True, long=True))  # <-- all top-level nodes

    # --- Pick FBX file ---
    file_path = cmds.fileDialog2(fileMode=1, caption="Select FBX Animation")
    if not file_path:
        return
    file_path = file_path[0]

    # --- Import FBX ---
    cmds.file(
        file_path,
        i=True,
        type="FBX",
        ignoreVersion=True,
        mergeNamespacesOnClash=False,
        options="fbx",
        preserveReferences=True
    )

    # --- Separate imported vs scene joints ---
    all_joints_after = set(cmds.ls(type="joint", long=True))
    imported_joints  = list(all_joints_after - scene_joints_before)
    scene_joints     = list(scene_joints_before)

    # --- All new top-level nodes brought in by the import ---
    imported_assemblies = list(set(cmds.ls(assemblies=True, long=True)) - scene_assemblies_before)

    if not imported_joints:
        cmds.confirmDialog(title="Error", message="No joints found in imported FBX.", button=["OK"])
        if imported_assemblies:
            cmds.delete(imported_assemblies)
        return

    scene_lookup = {bare_name(j): j for j in scene_joints}

    # --- Match and constrain ---
    constraints     = []
    matched_targets = []

    for imported_joint in imported_joints:
        key = bare_name(imported_joint)
        if key not in scene_lookup:
            print(f"No match for: {key}")
            continue
        target_joint = scene_lookup[key]
        try:
            constraint = cmds.parentConstraint(imported_joint, target_joint, maintainOffset=False)
            constraints.extend(constraint)
            matched_targets.append(target_joint)
        except Exception as e:
            print(f"Could not constrain {key}: {e}")

    if not matched_targets:
        cmds.confirmDialog(title="Error", message="No matching joints found between FBX and scene skeleton.", button=["OK"])
        if imported_assemblies:
            cmds.delete(imported_assemblies)
        return

    print(f"Matched and constrained {len(matched_targets)} joints.")

    # --- Bake onto scene skeleton only ---
    start = cmds.playbackOptions(q=True, min=True)
    end   = cmds.playbackOptions(q=True, max=True)

    cmds.bakeResults(
        matched_targets,
        time=(start, end),
        simulation=True,
        sampleBy=1,
        minimizeRotation=True,
        disableImplicitControl=True,
        preserveOutsideKeys=False
    )

    # --- Cleanup: delete constraints then ALL imported nodes (skeleton + meshes + groups) ---
    cmds.delete(constraints)

    if imported_assemblies:
        cmds.delete(imported_assemblies)

    cmds.confirmDialog(
        title="Success",
        message=f"Animation baked onto {len(matched_targets)} joints successfully!",
        button=["OK"]
    )



# UI
def create_ui():

    if cmds.window(WINDOW_NAME, exists=True):
        cmds.deleteUI(WINDOW_NAME)

    cmds.window(
        WINDOW_NAME,
        title="FBX Animation Importer",
        widthHeight=(400, 180),
        sizeable=False
    )

    cmds.columnLayout(
        adjustableColumn=True,
        rowSpacing=10
    )

    cmds.separator(height=10)

    cmds.text(
        label="1. Select ROOT joint of your scene skeleton"
    )

    cmds.text(
        label="2. Click button and choose FBX animation"
    )

    cmds.separator(height=10)

    cmds.button(
        label="Import FBX Animation",
        height=50,
        command=lambda x: import_fbx_animation()
    )

    cmds.separator(height=10)

    cmds.showWindow(WINDOW_NAME)

# Run UI
create_ui()