import bpy
import bmesh
import struct
from math import radians
from bpy_extras.io_utils import ExportHelper

class ExportSymmetry(bpy.types.Operator, ExportHelper):
    bl_idname       = "export_symmetry.symbres";
    bl_label        = "Symmetry Exporter";
    bl_options      = {'PRESET'};
    filename_ext    = ".symbres";
    
    def execute(self, context):
        scene = context.scene
        activeObject = scene.objects.active

        if not activeObject or str(activeObject.type) != 'MESH':
            raise NameError("Cannot export : Object %s is not a mesh" % activeObject)

        print("Exporting : " + activeObject.name)

        # Rotate -90 deg on x axis to compensate for blender's different orientation
        activeObject.rotation_euler[0] = radians(-90)
        bpy.ops.object.transform_apply(location = True, scale = True, rotation = True)
        
        mesh = activeObject.to_mesh(scene, True, 'PREVIEW')
        bm = bmesh.new()
        bm.from_mesh(mesh)
        bmesh.ops.triangulate(bm, faces = bm.faces)

        indices  = []
        vertices = []
        normals  = []
        uvs      = []

        if len(mesh.uv_layers) > 0:
            uv_layer = bm.loops.layers.uv.values()[0]
            index = 0;
            for face in bm.faces:
                for loop in face.loops:
                    uv = loop[uv_layer].uv
                    uv.y = 1.0 - uv.y
                    vert = loop.vert
                    vertices.append(vert.co.to_tuple())
                    normals.append(vert.normal.to_tuple())
                    uvs.append(uv.to_tuple())
                    indices.append(index)
                    index += 1
        else:
            raise NameError("No uv layers detected. Did you forget to unwrap?")

        bm.free()
        del bm

        # Reset object's previous rotation
        activeObject.rotation_euler[0] = radians(90)
        bpy.ops.object.transform_apply(location = True, scale = True, rotation = True)
        
        file = open(self.filepath, 'bw')

        # Header
        file.write(struct.pack('i', len(indices)))
        file.write(struct.pack('i', len(vertices)))
        file.write(struct.pack('i', len(normals)))
        file.write(struct.pack('i', len(uvs)))

        print ("Num Indices  : %d" % len(indices))
        print ("Indices : \n %s \n\n" % str(indices))
        
        print ("Num Vertices : %d" % len(vertices))
        print ("Vertices : \n %s \n\n" % str(vertices))

        print ("Num Normals : %d" % len(normals))
        print ("Normals : \n %s \n\n" % str(normals))

        print ("Num UVs : %d" % len(uvs))
        print ("UVs : \n %s \n\n" % str(uvs))

        # Body
        for index in indices:
            file.write(struct.pack('i', index))
            
        for vertex in vertices:
            file.write(struct.pack('fff', vertex[0], vertex[1], vertex[2]))

        for normal in normals:
            file.write(struct.pack('fff', normal[0], normal[1], normal[2]))

        for uv in uvs:
            file.write(struct.pack('ff', uv[0], uv[1]))

        file.close()

        print("Done!")
        return {'FINISHED'};
