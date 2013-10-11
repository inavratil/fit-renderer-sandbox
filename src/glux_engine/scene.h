/**
****************************************************************************************************
****************************************************************************************************
@file: scene.h
@brief initialization and drawing the scene - declarations
****************************************************************************************************
***************************************************************************************************/
#ifndef _SCENE_H_
#define _SCENE_H_

#include "globals.h"
#include "engine.h"

#include "sdk/Material.h"
#include "sdk/Pass.h"
#include "resources/TextureCache.h"
#include "resources/MaterialManager.h"
#include "sdk/GeometryMaterial.h"
#include "camera.h"
#include "shadow.h"
#include "sdk/RenderListener.h"
#include "object.h"
#include "light.h"
#include "shadows/IShadowTechnique.h"

const int align = sizeof(glm::vec4);      //BUG: ATI Catalyst 10.12 drivers align uniform block values to vec4

/**
@class TScene
@brief containts list of objects, inits and draws 3D scene.
It has 3 main lists: objects, materials and lights. Materials are being applied to objects and
all this is lit by lights.
***************************************************************************************************/
class TScene
{
protected:
    ///associative array with all objects
    map<string,TObject*> m_objects;
    ///iterator for objects container
    map<string,TObject*>::iterator m_io;
    
	///associative array with all lights
    vector<TLight*> m_lights;
    ///iterator for lights container
    vector<TLight*>::iterator m_il, m_il2;
    
    int m_shadow_textures;    //count of shader textures

    ///3DS objects cache - purpose is the same as texture cache
    map<string,VBO> m_obj_cache;
    ///iterator for object cache container
    map<string,VBO>::iterator m_iob;

    ///uniform buffers
    GLuint m_uniform_matrices, m_uniform_lights;

    ///2D font texture
    Texture *m_font2D_tex;
    ///Background texture
    Texture *m_font2D_bkg;
    ///Display list for font
    GLuint m_font2D;

    ///screen resolution
    GLint m_resx, m_resy;

    ///scene camera
    TCamera *m_cam;

    ///number of multisaples in antialiasing
    GLint m_msamples;

    ///camera matrices
    glm::mat4 m_viewMatrix, m_projMatrix;

    ///shall we use HDR, SSAO or shadows?
    bool m_useHDR, m_useSSAO, m_useShadows, m_useNormalBuffer;
    ///wireframe
    bool m_wireframe;

    ///framebuffer/renderbuffer objects for color/depth
    GLuint m_f_buffer, m_r_buffer_depth, m_f_buffer_select,
           m_f_bufferMSAA, m_r_buffer_colorMSAA, m_r_buffer_normalMSAA, m_r_buffer_depthMSAA;


    ///render target sizes
    GLint m_RT_resX, m_RT_resY;				

    ///number of items to be loaded
    float m_load_list, m_load_actual;

    ///subscene ID - used when we need to hide some parts of the scene
    int m_sceneID;

    ///settings for DPSM
    float m_dp_FOV;
    glm::vec3 m_parab_angle;
    bool m_dpshadow_tess, m_draw_shadow_map;
    bool m_use_pcf, m_draw_aliasError;
    int m_dpshadow_method;
    float m_min_depth, m_avg_depth, m_max_depth;
    glm::vec3 m_avg_normal;
    float *m_select_buffer;
    glm::vec2 m_cut_angle;

	//-----------------------------------------------------------------------------
	//FIXME
	int m_texPreview_id;
	IShadowTechnique* m_shadow_technique; //?? Musi se to opravovat

//FIXME: Tohle by melo prijit do tridy Application
protected:
	//-- Texture Cache
	TextureCachePtr							m_texture_cache;
	//-- Texture Cache
	MaterialManagerPtr						m_material_manager;

	//-- I use 'set', because of 'find' method
	set<RenderListener*>			m_render_listeners;
	set<RenderListener*>::iterator	m_it_render_listeners;

public:


	void AddRenderListener( RenderListener* _listener )
	{
		if(m_render_listeners.find(_listener) != m_render_listeners.end())
		{
			cerr<<"WARNING (AddRenderListener): listener has already been added to the list.\n";
			return;
		}
		m_render_listeners.insert( _listener );
	}

	void RemoveRenderListener( RenderListener* _listener )
	{
		m_it_render_listeners = m_render_listeners.find( _listener );
		if( m_it_render_listeners != m_render_listeners.end() )
			m_render_listeners.erase( m_it_render_listeners ); 
	}
//-----------------------------------------------------------------------------
	

public:
    //basic constructor
    TScene();
    //free dynamic data
    ~TScene();

    //scene initialization
    bool PreInit(GLint resx, GLint resy, GLint msamples = 1, bool load_font = false);
    bool PostInit();
    //resize window
    void Resize(GLint resx, GLint resy);
    //draw scene - setup, render targets and objects
    void Redraw(bool delete_buffer = true);
    //draw all objects in scene
    void DrawScene(int drawmode);
    void DrawSceneDepth(const char* shadow_mat, glm::mat4& lightMatrix);
    void DrawGeometry(const char* _shader, glm::mat4& _mvMatrix );

    //draw load screen
    void LoadScreen(bool swap = true);
    //destroy scene, free dynamic data
    void Destroy(bool delete_cache = true);
    ///@brief set load items count (extern objects, textures and scenes)
    void SetLoadList(int count){ 
        m_load_list = count; m_load_actual = count; 
    }
    ///@brief update load list (when loading scene)
    void UpdateLoadList(int count){  
		float tmp = 1.0;
		if( m_load_actual > 0.0)
			tmp = m_load_list/m_load_actual;
		m_load_actual += count; 
        m_load_list = m_load_actual * tmp;
    }

    //load whole scene from 3DS file
    void LoadScene(const char* file, bool load_materials = true, bool load_lights = true, string name_space = "");

    ///Change scene ID. All objects will from now belong to this ID. Only one sceneID section can be active at time
    void ChangeSceneID(int id){ m_sceneID = id; }
	int GetSceneID(){ return m_sceneID; }

    ///Toggle wireframe rendering
    void SetWireframe( bool flag ){ m_wireframe = flag;}
	bool IsWireframe(){ return m_wireframe; }

	MaterialManager* GetMaterialManager(){ return m_material_manager; }
	TextureCache* GetTextureCache(){ return m_texture_cache; }

/////////////////////////////////////////// CAMERA ////////////////////////////////////////

	TCamera* CreateCamera()
	{ 
		if( !m_cam )
			m_cam = new TCamera(); 
		return m_cam; 
	}

    ///@brief Update new camera position and modelview matrix(for lights) in uniform buffer
    void UpdateCameraUniform(){
        glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_lights);
        //glBufferSubData(GL_UNIFORM_BUFFER, lights.size()*5*align, sizeof(glm::vec3), glm::value_ptr(cam.GetPos())); 
        int i; //update light positions with new modelview matrix
        for(i = 0, m_il = m_lights.begin(); m_il != m_lights.end(); ++m_il, ++i)
            glBufferSubData( GL_UNIFORM_BUFFER, i*align, sizeof(glm::vec3), 
                             glm::value_ptr(glm::vec3(m_viewMatrix * glm::vec4((*m_il)->GetPos(), 1.0))) );
    }

	glm::mat4 GetViewMatrix()
	{
		return m_cam->GetMatrix();
	}

	glm::mat4 GetProjMatrix()
	{
		return m_projMatrix;
	}

	//TODO: MUST be called before drawing
	void UpdateCamera()
	{
		m_viewMatrix = m_cam->UpdateMatrix();
		UpdateCameraUniform();
	}

	void SetFreelookCamera(glm::vec3 pos, glm::vec3 up, glm::vec3 focusPoint)
	{
		m_cam->setFreelookCamera(pos, up, focusPoint);
	}
	
	void HandleCameraInputMessage(TCamera::cam_events e)
	{ 
		m_cam->handleInputMessage(e);
	}

	void AdjustFreelookCamera(float pitch, float yaw)
	{
		m_cam->adjustOrientation(pitch, yaw);
	}


    ///@brief Print out camera position
    void PrinTFreelookCamera(){ 
        cout<<"POS: "<<m_cam->GetPos().x<<","<<m_cam->GetPos().y<<","<<m_cam->GetPos().z<<"\n";
     }

	TCamera* GetCameraPtr()
	{
		return m_cam;
	}
    ///@brief Get screen-space camera position
    glm::vec3 GetCameraPos(){ 
        return m_cam->GetPos(); 
    }


    //Save camera into file
    void SaveCamera(){
        m_cam->Save();
    }
    //Load camera position
    void LoadCamera(){
        m_viewMatrix = m_cam->Load();
        UpdateCameraUniform();
    }


/////////////////////////////////////////// LIGHTS ////////////////////////////////////////

    TLight* AddLight(GLint _lights, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, glm::vec3 lpos, GLfloat radius = 1000.0);

    ///@brief Remove light identified by number
    ///@todo remove selected light only, not whole list
    void RemoveLight(/*int lightnum*/){ 
        m_lights.clear(); 
    }

    void MoveLight(GLint light, glm::vec3 w);
    void ChangeLightColor(GLint light, GLint component, glm::vec3 color);

    ///@brief Return number of lights in scene
    int GetLightCount(){ 
        return m_lights.size(); 
    }

    ///@brief Return light position
    glm::vec3 GetLightPos(int light){
        if(light < 0 || (unsigned)light >= m_lights.size()) { cerr<<"WARNING: no light with index "<<light<<"\n"; return glm::vec3(0.0); }
        else return m_lights[light]->GetPos();
    }

    ///@brief set light radius
    void SetLightRadius(int light, GLfloat radius){
        if(light < 0 || (unsigned)light >= m_lights.size()) 
            cerr<<"WARNING: no light with index "<<light<<"\n";
        else {
            m_lights[light]->SetRadius(radius);
            //update uniform buffer
            glBindBuffer(GL_UNIFORM_BUFFER, m_uniform_lights);
            glBufferSubData(GL_UNIFORM_BUFFER, m_lights.size()*4*align + light*align, sizeof(float), &radius); 
        }
    }

    /////////////////////////////////////////// OBJECTS ////////////////////////////////////////

    ///@brief Add new object (see TObject() into list. Index is object name
    TObject* AddObject(const char *name, int primitive, GLfloat size = 0.0, GLfloat height = 0.0, GLint sliceX = 1, GLint sliceY = 1){
        TObject *o = new TObject(name, primitive, size, height, sliceX, sliceY);
        m_objects[name] = o;
        LoadScreen(); //update loading screen
        m_objects[name]->SetSceneID(m_sceneID);
		return o;
    }
    ///@brief add new object as instance from existing object
    void AddObjectInstance(const char *ref_name, const char *inst_name){
        if(m_objects.find(ref_name) == m_objects.end()) 
            cerr<<"WARNING (AddObjectInstance): no reference object with name "<<ref_name<<"\n";
        else{
            m_objects[inst_name]->CreateInstance(*m_objects[ref_name]);
            m_objects[inst_name]->SetSceneID(m_sceneID);
        }
    }
    //add new object from external file
    TObject* AddObject(const char *name, const char* file);

    ///@brief Return object's vertex buffer ID
    GLint GetVertexBuffer(const char* name){
        return m_objects[name]->GetVertexBuffer(); 
    }


    ///@brief Get object position
    glm::vec3 GetObjPosition(const char *name){
        return m_objects[name]->GetPosition(); 
    }

    ///@brief toggle object drawing
    void DrawObject(const char* obj_name, bool flag){
        m_objects[obj_name]->DrawObject(flag); 
    }

    ///@brief Set count of object instances
    void SetGInstances(const char* obj_name, int count){ 
        m_objects[obj_name]->SetGInstances(count); 
    }

	///////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////SHADOWS ///////////////////////////////////////

    //create shadow map for selected light
    bool CreateShadowMap(vector<TLight*>::iterator ii);
    //render shadow map (spot)
    void RenderShadowMap(TLight *l);
    //render shadow map (omnidirectional, dual-paraboloid)
    void RenderShadowMapOmni(TLight *l);


    ///@brief Set shadow parameters(shadow size and intensity) for selected light (by index)
    ///(see TLight::SetShadow()
    void SetShadow(GLint lightNum, GLint shadow_size = 2048, int type = SPOT, 
                   GLfloat _shadow_intensity = 0.5, bool shadow = true){
        m_useShadows = true;
        if(lightNum < 0 || (unsigned)lightNum > m_lights.size()) 
            cerr<<"WARNING: no light with index "<<lightNum<<"\n";
        else 
        {
            m_lights[lightNum]->SetShadow(shadow_size,_shadow_intensity, type, shadow);
            m_shadow_textures++;
        }
    }

    ///@brief Change shadow paraboloid rotation
    void RotateParaboloid(glm::vec3 angle){
        m_parab_angle = angle;
    }


    ///@brief GLobally enable/disable shadows (for enable, shadows must be created for light that casts them
    void UseShadows(bool flag = true){ 
        m_useShadows = flag; 
    }

	IShadowTechnique* SetShadowTechnique( IShadowTechnique* _p_technique )
	{
		m_shadow_technique = _p_technique;

		this->AddRenderListener( reinterpret_cast<RenderListener*>(m_shadow_technique) );
		return m_shadow_technique;
	}

	IShadowTechnique* GetShadowTechnique()
	{
		return m_shadow_technique;
	}

    //IMPROVED DUAL-PARABOLOID SHADOWS SETTINGS

    void DPSetCutAngle( glm::vec2 _angle ){
		m_cut_angle = _angle;
	}

    ///@brief set usage of PCF
    void DPSetPCF(bool flag){
        m_use_pcf = flag;
    }

    ///@brief Toggle dual-paraboloid  shadow enhancements
    void DPShadowMethod(int type){
        m_dpshadow_method = type;
        if(type == DPSM)
        {
            m_dp_FOV = 179.0f;
            m_avg_depth = 1000.0f;
        }
    }


    ///@brief Get far point value
    float DPGetFarPoint(){
        return m_avg_depth;
    }
    ///@brief Get average normal
    glm::vec3 DPGetAvgNormal(){
        return m_avg_normal;
    }

    ///@brief Get actual shadow FOV (front paraboloid)
    float DPGetFOV(){
        return m_dp_FOV;
    }

	bool DPGetTessellation()	  	
	{	  	
		return m_dpshadow_tess; 	  	
	}

    ///@brief toggle tessellation in paraboloid projection
    void DPSetTessellation(bool flag){
        if(flag && !GLEW_ARB_gpu_shader5)
        {
            cout<<"OpenGL 4 not supported. Cannot enable tessellation :(\n";
            flag = false;
        }
        m_dpshadow_tess = flag;
    }

    ///@brief Toggle shadow map drawing
    void DPDrawSM(bool flag){
        m_draw_shadow_map = flag;
    }

    void DPDrawAliasError(bool flag){
        m_draw_aliasError = flag;
    }

	void SetTexturePreviewId( int _id )
	{
		m_texPreview_id = _id;
	}

	void SetWarping( bool _isEnabled );

    ////////////////////////// RENDER TARGETS, HDR AND SSAO ////////////////////////

    //prepare screen quad
    void AddScreenQuad();

    //resize render target textures
    void ResizeHDRRenderTarget(int resX, int resY, GLint tex_format = GL_RGBA16F, GLenum tex_type = GL_FLOAT);
    //check framebuffer status
    bool CheckFBO();
    //render screen quad with attached shader
    void RenderPass(const char* material);
    //render small quad over scene (to visualize some buffers etc)
    void RenderSmallQuad(const char* material, float offset_x, float offset_y, float size);
    ///@brief toggle use of HDR
    void UseHDR(bool flag = true){ 
        m_useHDR = flag; 
    }
    ///@brief toggle use of SSAO
    void UseSSAO(bool flag = true){ 
        m_useSSAO = flag; 
        m_useNormalBuffer = flag;        
    }


    ///////////////////////////////////// TEXT ////////////////////////////////////////////////

    //Draw text on screen
    void DrawScreenText(const char *s, float x, float y, float size = 1.0, bool center = true);
    void BuildFont();

    ////////////////////////////////////// MISCELLANEOUS //////////////////////////////////////

    ///@brief Get screen width
    int GetResX(){
        return m_resx; 
    }
    ///@brief Get screen height
    int GetResY(){ 
        return m_resy; 
    }
};

typedef TScene* ScenePtr;

#endif
