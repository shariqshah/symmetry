bl_info = {
    "name": "Export to Symmetry",
    "description": "Export to a format that can be read by Symmetry",
    "author": "Shariq Shah",
    "version": (0, 1),
    "blender": (2, 79, 0),
    "location": "File > Export > Export to Symmetry",
    "category": "Import-Export"
}

# Ensure that we reload our dependencies if we ourselves are reloaded by Blender
if "bpy" in locals():
    import imp;
    if "exporter" in locals():
        imp.reload(exporter);

import bpy
from . import exporter


def menu_func(self, context):
    self.layout.operator(ExportSymmetry.bl_idname, text="Symbres (.symbres)");

def register():
    bpy.utils.register_module(__name__);
    bpy.types.INFO_MT_file_export.append(menu_func);
    
def unregister():
    bpy.utils.unregister_module(__name__);
    bpy.types.INFO_MT_file_export.remove(menu_func);


if __name__ == "__main__":
    register()
