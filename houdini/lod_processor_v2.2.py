import os
import sys
import math
import PIL
from PIL import Image
import json
import shutil
import time

GEO = sys.argv[1]
WORK_DIR = sys.argv[2]
TESTMODE = str(0)
if len(sys.argv)>2:
    TESTMODE = sys.argv[3]
TARGET_COUNT = 20000
REDUCE_FACTOR = 2

def clean(root_path,root_node,name,input_node):
    clean_node = hou.node(root_path+'/'+name)
    if clean_node == None:
        clean_node = root_node.createNode('clean',name)
    clean_node.parm('dodelattribs').set(1)
    clean_node.parm('delattribs').set('* ^uv ^shop_materialpath')
    clean_node.parm('dodelgroups').set(1)
    clean_node.parm('delgroups').set('*')
    clean_node.parm('deldegengeo').set(0)
    clean_node.parm('delunusedpts').set(0)
    clean_node.parm('delnans').set(0)
    clean_node.setInput(0,input_node)
    return clean_node

def wrangle(root_path,root_node,name,code,input_a=None,output_a=0,input_b=None,output_b=0,input_c=None,output_c=0):
    wrangle = hou.node(root_path+'/'+name)
    if wrangle == None:
        wrangle = root_node.createNode('attribwrangle',name)
    wrangle.setInput(0,input_a,output_a)
    wrangle.setInput(1,input_b,output_b)
    wrangle.setInput(2,input_c,output_c)
    wrangle.parm('class').set(0)
    wrangle.parm('snippet').set(code)
    return wrangle

def clip(root_path,root_node,name,input_a=None):#maybe also set origin
    clip = hou.node(root_path+'/'+name)
    if clip == None:
        clip = root_node.createNode('clip',name)
    clip.parm('clipop').set(2)
    clip.parm('newg').set(1)
    clip.setInput(0,input_a)
    return clip

def compile_begin(geo_path,geo_node,name,end_node,input):
    compile_begin = hou.node(geo_path+'/'+name)
    if compile_begin == None:
        compile_begin = geo_node.createNode('compile_begin',name)
    compile_begin.parm('blockpath').set('../'+end_node)
    compile_begin.setInput(0,input)
    return compile_begin

def compile_end(geo_path,geo_node,name,begin_node,input):
    compile_end = hou.node(geo_path+'/'+name)
    if compile_end == None:
        compile_end = geo_node.createNode('compile_end',name)
    compile_end.setInput(0,input)
    return compile_end

def foreach_begin(geo_path,geo_node,name,end_node,input):
    foreach_begin = hou.node(geo_path+'/'+name)
    if foreach_begin == None:
        foreach_begin = geo_node.createNode('block_begin',name)
    foreach_begin.parm('method').set(1)
    foreach_begin.parm('blockpath').set('../'+end_node)
    foreach_begin.setInput(0,input)
    return foreach_begin

def foreach_end(geo_path,geo_node,name,iterations,begin_node,input,single=1):
    foreach_end = hou.node(geo_path+'/'+name)
    if foreach_end == None:
        foreach_end = geo_node.createNode('block_end',name)
    foreach_end.parm('itermethod').set(1)
    foreach_end.parm('method').set(1)
    foreach_end.parm('class').set(0)
    foreach_end.parm('useattrib').set(1)
    foreach_end.parm('attrib').set('name')
    foreach_end.parm('blockpath').set('../'+begin_node)
    foreach_end.parm('templatepath').set('../'+begin_node)
    foreach_end.parm('dosinglepass').set(single)
    foreach_end.parm('singlepass').set(iterations)
    foreach_end.parm('multithread').set(1)
    foreach_end.setInput(0,input)
    return foreach_end

def foreach_meta(geo_path,geo_node,name,end_node):
    foreach_meta = hou.node(geo_path+'/'+name)
    if foreach_meta == None:
        foreach_meta = geo_node.createNode('block_begin',name)
    foreach_meta.parm('method').set(2)
    foreach_meta.parm('blockpath').set('../'+end_node)
    return foreach_meta

def split(geo_path,geo_node,name,input):
    split = hou.node(geo_path+'/'+name)
    if split == None:
        split = geo_node.createNode('split',name)
    split.parm('group').set('above_plane')
    split.setInput(0,input)
    return split

def uvlayout(geo_path,geo_node,name,input):
    layout = hou.node(geo_path+'/'+name)
    if layout == None:
        layout = geo_node.createNode('uvlayout::3.0',name)
    layout.parm('packincavities').set(0)
    layout.parm('scaletolerance').set(0.02)
    layout.parm('resolution').set(1)
    layout.setInput(0,input)
    return layout


def switch(geo_path,geo_node,name,input_a,input_b):
    switch = hou.node(geo_path+'/'+name)
    if switch == None:
        switch = geo_node.createNode('switch',name)
    switch.parm('input').set(0)
    switch.setInput(0,input_a)
    switch.setInput(1,input_b)
    return switch

def merge(geo_path,geo_node,name,input_a=None,input_b=None):
    merge = hou.node(geo_path+'/'+name)
    if merge == None:
        merge = geo_node.createNode('merge',name)
    merge.setInput(0,input_a)
    merge.setInput(1,input_b)
    return merge

def null(geo_path,geo_node,name,input):
    null = hou.node(geo_path+'/'+name)
    if null == None:
        null = geo_node.createNode('null',name)
    null.setInput(0,input)
    return null

def group_delete(geo_path,geo_node,name,input):
    grp_del = hou.node(geo_path+'/'+name)
    if grp_del == None:
        grp_del = geo_node.createNode('groupdelete',name)
    grp_del.parm('group1').set('*')
    grp_del.setInput(0,input)
    return grp_del

def apply_mat(root_path,root_node,name,mat_path,input_node):
    mat_node = hou.node(root_path+'/'+name)
    if mat_node == None:
        mat_node = root_node.createNode('material',name)
    mat_node.parm('shop_materialpath1').set('../'+mat_path)
    mat_node.setInput(0,input_node)
    return mat_node


def lod_process(geo_node,clean_node,prims,it,geo_dir):
    #create a folder to store export data
    export_dir = geo_dir+'export/'
    temp_dir = geo_dir+'temp/'
    geo_path = '/obj/lod_processing'
    try:
        os.mkdir(temp_dir)
        os.mkdir(export_dir)
    except:
        print('Directories already exists')

    #define a dictionary to store all bbox data
    bbox_data = {}

    #fetch input texture data
    files = os.listdir(geo_dir)
    start_tex_files = []
    res = 0
    start = 0
    print('PROCESSING TEXTURES')
    for file in files: #can it be assumed they are all the same values?
        if '.jpg' in file: #is it actually always jpg?
            start += 1
            PIL.Image.MAX_IMAGE_PIXELS = 1073741824
            img = PIL.Image.open(geo_dir+file)
            w, h = img.size
            start_tex_files.append(file)
            res += w
    res = res/start

    #create a list with tex resolutions for all lods and store new names
    res_list = []
    tex_list = []
    new_name = start_tex_files[0].split('.')
    ext = new_name[-1]
    new_name = new_name[0].split('_')[0]
    for i in range(it+1):
        num = it-i
        divisor = pow(2,num)
        new_res = res/divisor
        res_list.append(new_res)
        num = '%02d' % num
        input_tex = new_name+'_u%(U)d_v%(V)d_'+num+'.'+ext #are they all the non udim type?
        tex_list.append(input_tex)

    #create all textures
    for a in range(len(start_tex_files)):
        PIL.Image.MAX_IMAGE_PIXELS = 1073741824
        img = PIL.Image.open(geo_dir+start_tex_files[a])
        for i in range(it+1):
            num = '%02d' % i
            new_name = start_tex_files[a].split('.')
            new_name = new_name[0]+'_'+num+'.'+new_name[1]
            new_tex = temp_dir+new_name
            new_res = res_list[it-i]
            img = img.resize((new_res,new_res),PIL.Image.ANTIALIAS)
            img.save(new_tex)

    source_count = len(prims)

    #test with one iteration
    if TESTMODE=='1':
        it = 2

    #snippet for the first name attribute
    start_lod = it
    patch_name = 'lod_patch_%02d_0000' % start_lod
    snippet_first_name = 'int prims = nprimitives(0);\n'\
    'for(int i=0;i<prims;i++)\n'\
    '{\n'\
    '    string name = \''+patch_name+'\';\n'\
    '    setprimattrib(0,\'name\',i,name);\n'\
    '}'

    #first name attr wrangle
    wrangle_first_name = wrangle(geo_path,geo_node,'set_first_name_attr',snippet_first_name,clean_node)

    #setup polyreduce
    reduce_node = hou.node(geo_path+'/polyreduce')
    if reduce_node == None:
        reduce_node = geo_node.createNode('polyreduce::2.0','polyreduce')
    reduce_node.parm('target').set(2)
    reduce_node.parm('boundaryweight').set(100)
    reduce_node.setInput(0,wrangle_first_name)

    #set a switch for the last iteration
    switch_node = switch(geo_path,geo_node,'input_switch',reduce_node,wrangle_first_name)

    #create a cop network for texture baking
    copnet = hou.node(geo_path+'/baking')
    if copnet == None:
        copnet = geo_node.createNode('cop2net','baking')

    #define the first loop variables
    patch_proc_in = switch_node
    patch_proc_out = switch_node
    data_proc_in = wrangle_first_name
    data_proc_out = wrangle_first_name
    patch_num = 1
    print('PROCESSING LODs')
    #iterate over lods
    for i in range(it+1):
        #calculate prim count for current patch
        divisor = pow(2,it-i)
        prim_count = math.ceil(source_count/divisor)
        print('LOD prims: '+str(prim_count))

        #set path variable
        lod_num = it-i
        lod_num = '%02d' % lod_num
        path = ''
        for prim in prims:
            #fetch the shop_materialpath attr of the first prim
            path = prim.attribValue('shop_materialpath')
            path = path.split('/')[-1].split('_')[:-3]
            path = '_'.join(path)
            break
        lod_path = path+'_LOD_'+lod_num

        #adjust polyreduce polycount
        reduce_node.parm('finalcount').set(prim_count)

        #set the switch on the last iteration
        if i==it:
            print('LAST ITERATION!!!!!!!!!!!')
            switch_node.parm('input').set(1)

        #create a new vex snippet to fetch the patch color from the points
        snippet_fetch_axis = 'vector bbmin = getbbox_min(0);\n'\
        'vector bbmax = getbbox_max(0);\n'\
        'vector bbs = getbbox_size(0);\n'\
        'float max = max(bbs[0],bbs[1],bbs[2]);\n'\
        'int axis = 0;\n'\
        'for(int i=0;i<3;i++)\n'\
        '{\n'\
        '    if(bbs[i]==max)\n'\
        '    {\n'\
        '        axis = i;\n'\
        '    }\n'\
        '}\n'\
        'vector val = {0,0,0};\n'\
        'val[axis] = 1;\n'\
        'i@axis = axis;\n'\
        'f@dirx = val[0];\n'\
        'f@diry = val[1];\n'\
        'f@dirz = val[2];\n'\

        #create a new vex snippet to fetch the patch color from the points
        snippet_fetch_pos = 'int npoints = npoints(0);\n'\
        'int axis = i@axis;\n'\
        'vector sourcepos = set(f@dirx,f@diry,f@dirz);\n'\
        'sourcepos *= vector(-100);\n'\
        'int nearp[] = nearpoints(0,sourcepos,200,int(npoints/2));\n'\
        'float maxpos = -100.0;\n'\
        'for(int i=0;i<len(nearp);i++)\n'\
        '{\n'\
        '    vector pos = point(0,\'P\',nearp[i]);\n'\
        '    maxpos = max(maxpos,pos[axis]);\n'\
        '}\n'\
        'f@maxpos = maxpos*0.95;'

        snippet_set_patch_name_a = 'int nprims = nprimitives(0);\n'\
        'float maxpos = detail(0,\'maxpos\',0);\n'\
        'int axis = detail(0,\'axis\',0);\n'\
        'float dirx = detail(0,\'dirx\',0);\n'\
        'float diry = detail(0,\'diry\',0);\n'\
        'float dirz = detail(0,\'dirz\',0);\n'\
        'for(int i=0;i<nprims;i++)\n'\
        '{\n'\
        '    int it = detail(1,\'iteration\',0);\n'\
        '    string old_name = prim(0,\'name\',i);\n'\
        '    string name = \'lod_patch_a_'+str(lod_num)+'_\'+itoa(it);\n'\
        '    setprimattrib(0,\'name\',i,name);\n'\
        '    setprimattrib(0,\'old_name\',i,old_name);\n'\
        '    setprimattrib(0,\'maxpos\',i,maxpos);\n'\
        '    setprimattrib(0,\'dirx\',i,dirx);\n'\
        '    setprimattrib(0,\'diry\',i,diry);\n'\
        '    setprimattrib(0,\'dirz\',i,dirz);\n'\
        '}'

        snippet_set_patch_name_b = 'int nprims = nprimitives(0);\n'\
        'float maxpos = detail(0,\'maxpos\',0);\n'\
        'int axis = detail(0,\'axis\',0);\n'\
        'float dirx = detail(0,\'dirx\',0);\n'\
        'float diry = detail(0,\'diry\',0);\n'\
        'float dirz = detail(0,\'dirz\',0);\n'\
        'for(int i=0;i<nprims;i++)\n'\
        '{\n'\
        '    int it = detail(1,\'iteration\',0);\n'\
        '    string old_name = prim(0,\'name\',i);\n'\
        '    string name = \'lod_patch_b_'+str(lod_num)+'_\'+itoa(it);\n'\
        '    setprimattrib(0,\'name\',i,name);\n'\
        '    setprimattrib(0,\'old_name\',i,old_name);\n'\
        '    setprimattrib(0,\'maxpos\',i,maxpos);\n'\
        '    setprimattrib(0,\'dirx\',i,dirx);\n'\
        '    setprimattrib(0,\'diry\',i,diry);\n'\
        '    setprimattrib(0,\'dirz\',i,dirz);\n'\
        '}'
        # on the second loop start the splitting process
        if i>0:
            #first create the data tree
            data_foreach_begin_node = foreach_begin(geo_path,geo_node,'data_foreach_start'+str(lod_num),'data_foreach_end'+str(lod_num),data_proc_in)
            data_metadata_node = foreach_meta(geo_path,geo_node,'data_foreach_meta'+str(lod_num),'data_foreach_end'+str(lod_num))
            fetch_axis_wrangle = wrangle(geo_path,geo_node,'fetch_axis_'+str(lod_num),snippet_fetch_axis,data_foreach_begin_node)
            fetch_clip_pos_wrangle = wrangle(geo_path,geo_node,'fetch_clipping_pos_'+str(lod_num),snippet_fetch_pos,fetch_axis_wrangle)
            data_clip_mesh_node = clip(geo_path,geo_node,'data_clip_mesh_'+str(lod_num),fetch_clip_pos_wrangle)
            #adjust the clip node parameters
            data_clip_mesh_node.parm('dist').setExpression('detail(0,\'maxpos\',0)')
            data_clip_mesh_node.parm('dirx').setExpression('detail(0,\'dirx\',0)')
            data_clip_mesh_node.parm('diry').setExpression('detail(0,\'diry\',0)')
            data_clip_mesh_node.parm('dirz').setExpression('detail(0,\'dirz\',0)')
            data_split_mesh_node = split(geo_path,geo_node,'data_split_mesh_'+str(lod_num),data_clip_mesh_node)
            data_name_wrangle_a = wrangle(geo_path,geo_node,'set_data_name_a_attr_'+str(lod_num),snippet_set_patch_name_a,data_split_mesh_node,0,data_metadata_node)
            data_name_wrangle_b = wrangle(geo_path,geo_node,'set_data_name_b_attr_'+str(lod_num),snippet_set_patch_name_b,data_split_mesh_node,1,data_metadata_node)
            named_data_merge = merge(geo_path,geo_node,'merge_data_'+str(lod_num),data_name_wrangle_a,data_name_wrangle_b)
            data_foreach_end_node = foreach_end(geo_path,geo_node,'data_foreach_end'+str(lod_num),0,'data_foreach_start'+str(lod_num),named_data_merge,0)
            data_proc_out = group_delete(geo_path,geo_node,'remove_data_grps_'+str(lod_num),data_foreach_end_node)
            data_proc_in = data_proc_out

            #setup data patch loop
            loop_foreach_begin_node = foreach_begin(geo_path,geo_node,'loop_foreach_start'+str(lod_num),'loop_foreach_end'+str(lod_num),data_proc_in)
            loop_foreach_end_node = foreach_end(geo_path,geo_node,'loop_foreach_end'+str(lod_num),0,'loop_foreach_start'+str(lod_num),loop_foreach_begin_node)
            loop_foreach_end_node.parm('singlepass').setExpression('detail(\'../patch_foreach_meta'+str(lod_num)+'\',\'iteration\',0)')
            loop_foreach_end_node.parm('attrib').set('old_name')

            #second create the patches tree
            pat_foreach_begin_node = foreach_begin(geo_path,geo_node,'patch_foreach_start'+str(lod_num),'patch_foreach_end'+str(lod_num),patch_proc_in)
            pat_metadata_node = foreach_meta(geo_path,geo_node,'patch_foreach_meta'+str(lod_num),'patch_foreach_end'+str(lod_num))
            clip_mesh_node = clip(geo_path,geo_node,'clip_mesh_'+str(lod_num),pat_foreach_begin_node)
            #adjust the clip node parameters
            clip_mesh_node.parm('dist').setExpression('prim(\'../loop_foreach_end'+str(lod_num)+'\',0,\'maxpos\',0)')
            clip_mesh_node.parm('dirx').setExpression('prim(\'../loop_foreach_end'+str(lod_num)+'\',0,\'dirx\',0)')
            clip_mesh_node.parm('diry').setExpression('prim(\'../loop_foreach_end'+str(lod_num)+'\',0,\'diry\',0)')
            clip_mesh_node.parm('dirz').setExpression('prim(\'../loop_foreach_end'+str(lod_num)+'\',0,\'dirz\',0)')
            split_mesh_node = split(geo_path,geo_node,'split_mesh_'+str(lod_num),clip_mesh_node)
            name_wrangle_a = wrangle(geo_path,geo_node,'set_patch_name_a_attr_'+str(lod_num),snippet_set_patch_name_a,split_mesh_node,0,pat_metadata_node)
            name_wrangle_b = wrangle(geo_path,geo_node,'set_patch_name_b_attr_'+str(lod_num),snippet_set_patch_name_b,split_mesh_node,1,pat_metadata_node)
            named_patch_merge = merge(geo_path,geo_node,'merge_patches_'+str(lod_num),name_wrangle_a,name_wrangle_b)
            pat_foreach_end_node = foreach_end(geo_path,geo_node,'patch_foreach_end'+str(lod_num),0,'patch_foreach_start'+str(lod_num),named_patch_merge,0)
            patch_proc_out = group_delete(geo_path,geo_node,'remove_patch_grps_'+str(lod_num),pat_foreach_end_node)
            patch_proc_in = patch_proc_out

        #create a null as output for the patches process
        patch_end_null = null(geo_path,geo_node,'patches_OUT',patch_proc_out)

        #create a material network to store output materials
        matnet_output = hou.node(geo_path+'/output_materials')
        if matnet_output == None:
            matnet_output = geo_node.createNode('matnet','output_materials')
      
        #TEST WITH ONE PATCH ONLY
        if TESTMODE=='1':
            patch_num = 2
        
        print('LOD level: '+str(i))
        print('Number of patches calculated: '+str(patch_num))

        for a in range(patch_num):
            #create foreach begin
            name_foreach_begin_node = foreach_begin(geo_path,geo_node,'name_block_begin','name_block_end',patch_end_null)
            #create foreach end
            name_foreach_end_node = foreach_end(geo_path,geo_node,'name_block_end',a,'name_block_begin',name_foreach_begin_node)

            #unwrap uvs
            uv_unwrap_node = hou.node(geo_path+'/uv_unwrap')
            if uv_unwrap_node == None:
                uv_unwrap_node = geo_node.createNode('uvunwrap','uv_unwrap')
            uv_unwrap_node.parm('nplanes').set(0)
            uv_unwrap_node.parm('spacing').set(0)
            uv_unwrap_node.setInput(0,name_foreach_end_node)

            #layout uvs
            uv_layout_node = uvlayout(geo_path,geo_node,'uvlayout',uv_unwrap_node)

            #create a switch to switch bewteen uv methods
            uv_switch_node = switch(geo_path,geo_node,'uv_switch',uv_unwrap_node,uv_layout_node)
            if i>2:
                uv_switch_node.parm('input').set(1)

            #create a texture bake
            patch = '%04d' % a
            target_res = res_list[i]*start/patch_num

            #with test resolution
            if TESTMODE=='1':
                target_res = 231
            
            print('baking textures')
            export_tex_file = temp_dir+lod_path+'_'+patch+'.jpg'
            import_tex_file = temp_dir+tex_list[i]

            #apply uvs to pos
            #split along uvs
            uv_split_node = hou.node(geo_path+'/uv_split')
            if uv_split_node == None:
                uv_split_node = geo_node.createNode('splitpoints','uv_split')
            uv_split_node.parm('useattrib').set(1)
            uv_split_node.parm('attribname').set('uv')
            uv_split_node.parm('tol').set(0.001)
            uv_split_node.setInput(0,uv_switch_node)

            #set pos to uvs
            uvpos_snippet = 'int npoints = npoints(0);\n'\
            'for(int i=0;i<npoints;i++)\n'\
            '{\n'\
            '    int vtx = pointvertex(0,i);\n'\
            '    vector uv = vertex(0,\'uv\',vtx);\n'\
            '    setpointattrib(0,\'P\',i,uv);\n'\
            '}'
            uvpos_wrangle = wrangle(geo_path,geo_node,'uv_to_pos',uvpos_snippet,uv_split_node)
            uv_to_pos_null = null(geo_path,geo_node,'uv_to_pos_OUT',uvpos_wrangle)

            copnet_path = '/obj/lod_processing/baking'
            copnet_node = hou.node(copnet_path)
            tex_bake_node = hou.node(copnet_path+'/texture_bake')
            if tex_bake_node == None:
                tex_bake_node = copnet_node.createNode('construkted_tex_baker_v100','texture_bake')
            tex_bake_node.parm('texpathParm').set(import_tex_file)
            #change resolution to make sure it updates
            tex_bake_node.parm('compRezParmx').set(32)
            tex_bake_node.parm('compRezParmy').set(32)
            tex_bake_node.parm('compRezParmx').set(round(target_res))
            tex_bake_node.parm('compRezParmy').set(round(target_res))
            tex_bake_node.parm('outPathParm').set(export_tex_file)
            tex_bake_node.parm('exportBtn').pressButton()

            #create export material
            export_mat = hou.node(geo_path+'/output_materials/export_mat')
            if export_mat == None:
                export_mat = matnet_output.createNode('principledshader::2.0','export_mat')
            export_mat.parm('basecolorr').set(1.0)
            export_mat.parm('basecolorg').set(1.0)
            export_mat.parm('basecolorb').set(1.0)
            export_mat.parm('reflect').set(0)
            export_mat.parm('basecolor_useTexture').set(1)
            export_mat.parm('basecolor_texture').set(export_tex_file)

            #create a material node to assign the output material
            mat_export_node = apply_mat(geo_path,geo_node,'apply_export_mat','output_materials/export_mat',uv_switch_node)
            
            #clean the output data
            clean_output_node = clean(geo_path,geo_node,'clean_output_file',mat_export_node)

            #output the patch as glb file
            patch_name = lod_path+'_'+patch
            export_file = export_dir+patch_name+'.glb'
            geo_out_node = hou.node(geo_path+'/geo_out')
            if geo_out_node == None:
                geo_out_node = geo_node.createNode('rop_gltf','geo_out') #png or jpg in export?
            geo_out_node.parm('file').set(export_file)
            geo_out_node.parm('imageformat').set(1)
            geo_out_node.parm('imagequality').set(85)
            geo_out_node.setInput(0,clean_output_node)
            geo_out_node.parm('execute').pressButton()
            
            #store bounding box data 
            bbox = clean_output_node.geometry().intrinsicValue('bounds')
            bbox_data[patch_name] = bbox
            
            #print some information about the current process
            print(export_file)
            print('Texture resolution: '+str(int(target_res)))
            print('Bounding box: '+str(bbox))

            #remove temp texture
            #os.remove(export_tex_file)
            
        #adjust for new patch count and tex resolution
        patch_num *= 2
        if TESTMODE=='1':
            save_dir = geo_dir+'hipfile_'+str(lod_num)+'.hip'
            hou.hipFile.save(file_name=save_dir, save_to_recent_files=True)

    #save bounding box data in json file
    with open(geo_dir+'/export/bbox_data.json','w') as json_file:
        json.dump(bbox_data,json_file,indent=4)
    
    #remove the temp dir
    time.sleep(5)
    try:
        print('Finishing up')
        shutil.rmtree(geo_dir+'temp')
    except:
        print('No temp files to be cleaned')

        
def net_setup():
    #fetch geo directory
    geo_dir = GEO.split('\\')[0:-1]
    geo_dir = '/'.join(geo_dir)+'/'

    #install hdas
    baker_file = WORK_DIR+'/construkted_tex_baker_v100.hda'
    hou.hda.installFile(baker_file)

    #info about testmode
    if TESTMODE=='1':
        print('Testmode active')

    #remove json file
    json_file = geo_dir+'/export/bbox_data.json'
    try:
        os.remove(json_file)
    except:
        print('JSON initialized')

    #create the geo container for the lod process
    obj = hou.node('/obj')
    obj.createNode('geo','lod_processing')
    geo_path = '/obj/lod_processing'
    geo_node = hou.node(geo_path)

    #load the file
    geo_in_node = geo_node.createNode('file','geo_in')
    geo_in_node.parm('file').set(GEO)

    #clean the file
    clean_node = clean(geo_path,geo_node,'clean_input_file',geo_in_node)

    #fetch reduce data
    prims = clean_node.geometry().prims()
    nprims = len(prims)
    source = nprims/TARGET_COUNT
    it = math.ceil(math.log(source)/math.log(REDUCE_FACTOR))
    print('Source primitive count: '+str(nprims))
    print('Number of LODs calculated: '+str(it+1))

    #iterate each lod and process the geometry
    lod_process(geo_node,clean_node,prims,it,geo_dir)
    time.sleep(5)

net_setup()
