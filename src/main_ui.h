//MAIN APPLICATION VARIABLES, SETTINGS AND TWEAK BARS

#include "glux_engine/engine.h"
#include "experiments.h"

using namespace std;

//FPS
unsigned cycle = 0;                     //drawn cycles in one second 
unsigned fps;

//scene settings
TScene *s;          
string msg;
bool wire = false;                    
int msaa = 1;
float anim = 0.0;
bool fullscreen = false;
int resx = 1024, resy = 640;
int mem_use = 0;
bool draw_ui = true;

//scenes
int scene = 1;
int ex = -1;

//camera rotation and position
glm::vec3 rot, pos; 
int cam_type = FPS;



//SHADOW CONTROLS

#define USE_DP

//paraboloid rotation
bool move_parab = false;
glm::vec3 parab_rot(0.0, 0.0, 0.0);

//paraboloid shadow map settings
bool parabola_cut = false;
bool dpshadow_tess = false;
bool use_pcf = false;
int dpshadow_method = WARP_DPSM;
glm::vec2 cut_angle( 0.0, 0.0 );
float dp_frontFOV = 0.0, dp_farPoint = 0.0;
int ipsm_texPrev = OUTPUT;

bool draw_error = false;
bool drawSM = true;
bool do_warp = true;

Experiment exper;
bool isExperiment = false;

bool show_smaps = true;

int SHADOW_RES = 1024;

//multires settings
int mip_level = 0;
float normal_threshold = 1.0;
float depth_threshold = 15.0;



///////////////////////////////////////////////////////////////////////////////////////////////////
// TWEAK BAR

//CALLBACK FUNCTIONS

//Save/load camera
void TW_CALL twSaveCamera(void *clientData){
    s->SaveCamera();
}
void TW_CALL twLoadCamera(void *clientData){
    s->LoadCamera();
}

//Reset paraboloid
void TW_CALL twResetParab(void *clientData){
    parab_rot.x = 0.0;
    parab_rot.y = 0.0; 
    s->RotateParaboloid(parab_rot);
    cut_angle = glm::vec2( 0.0 );
    s->DPSetCutAngle(cut_angle);
}

/**
******************************************************************************
* Init tweak bar with gadgets and info
******************************************************************************/
TwBar *ui;
void InitTweakBar()
{
    TwInit(TW_OPENGL, NULL);    
    TwWindowSize(resx, resy);
    ui = TwNewBar("TweakBar");

    //scene information
    TwAddVarRO(ui, "fps", TW_TYPE_INT32, &fps, 
               " label='FPS'");
    /* TODO: bug in AntTweakBar?
    string gfx = (const char*)glGetString(GL_RENDERER);
    TwAddVarRO(ui, "gfx", TW_TYPE_STDSTRING, &gfx, 
               " label='GFX' group='Scene' ");
    */
    TwAddVarRO(ui, "mem_use", TW_TYPE_INT32, &mem_use, 
               " label='Memory (MB)' group='Scene' ");
    TwAddVarRO(ui, "resx", TW_TYPE_INT32, &resx, 
               " label='Width' group='Scene' ");
    TwAddVarRO(ui, "resy", TW_TYPE_INT32, &resy, 
               " label='Height' group='Scene' ");
    TwAddVarRO(ui, "msaa", TW_TYPE_INT32, &msaa, 
               " label='Antialiasing' group='Scene' ");

    TwAddSeparator(ui, NULL, "group='Scene'");
    TwAddVarRW(ui, "wire", TW_TYPE_BOOL32, &wire, 
               " label='Wireframe' group='Scene' key=x");

    //camera
    TwEnumVal e_cam_type[] = { {FPS, "FPS"}, {ORBIT, "Orbit"}};
    TwType e_cam_types = TwDefineEnum("cam_types", e_cam_type, 2);
    TwAddVarRW(ui, "cam_mode", e_cam_types, &cam_type, "label='Type' key=c group='Camera'");
    //camera save/load
    TwAddButton(ui, "cam_save", twSaveCamera, NULL, "label='Save' group='Camera' key=F5"); 
    TwAddButton(ui, "cam_load", twLoadCamera, NULL, "label='Load' group='Camera' key=F9"); 


    //shadow settings   
    TwAddVarRO(ui, "sh_res", TW_TYPE_INT32, &SHADOW_RES, 
               " label='Resolution' group='Shadows' ");
    //enum all shadow types
    TwEnumVal sh_mode[3] = { {DPSM, "DPSM"}, {IPSM, "IPSM"}, {CUT, "CUT"} };
    TwType sh_modes = TwDefineEnum("sh_modes", sh_mode, 3);
    TwAddVarRO(ui, "sh_mode", sh_modes, &dpshadow_method, "label='Mode' group='Shadows'");
	//IPSM textures
	TwEnumVal ipsm_tex[4] = { {OUTPUT, "Ouput"}, {PING, "Ping"}, {PONG, "Pong"}, {MASK, "Mask"} };
	TwType ipsm_texs = TwDefineEnum("ipsm_texs", ipsm_tex, 4);
    TwAddVarRW(ui, "ipsm_tex", ipsm_texs, &ipsm_texPrev, "label='Texture' group='Shadows'");
    //parabola cut
    TwAddVarRO(ui, "parabola_cut", TW_TYPE_BOOLCPP, &parabola_cut, 
               " label='Parabola cut' group='Shadows' ");
    //near/far point
    TwAddVarRO(ui, "frontFOV", TW_TYPE_FLOAT, &dp_frontFOV, 
               " label='Front FOV' group='Shadows' precision=1");
    TwAddVarRO(ui, "far_point", TW_TYPE_FLOAT, &dp_farPoint, 
               " label='Far point' group='Shadows' precision=1");

    //toggle paraboloid rotation
    TwAddSeparator(ui, NULL, "group='Shadows'");
    TwAddVarRW(ui, "move_parab", TW_TYPE_BOOLCPP, &move_parab, 
               " label='Moving paraboloid' group='Shadows' key=q");
    //reset paraboloid
    TwAddButton(ui, "reset_parab", twResetParab, NULL, "label='Reset paraboloid' key=r"); 
    //draw aliasing error
    TwAddVarRW(ui, "draw_error", TW_TYPE_BOOLCPP, &draw_error, 
               " label='Show alias error' group='Shadows' key=f ");
    //show shadow maps
    TwAddVarRW(ui, "drawSM", TW_TYPE_BOOLCPP, &drawSM, 
               " label='Show shadow maps' group='Shadows' ");
	//do warping
    TwAddVarRW(ui, "do_warp", TW_TYPE_BOOLCPP, &do_warp, 
               " label='Enable warping' group='Shadows' key=v ");

    //bar settings
    TwDefine("TweakBar refresh=0.1 size='256 512' ");
}

/**
******************************************************************************
* Update values bound with tweak bar
******************************************************************************/
void UpdateTweakBar()
{
    s->Wireframe(wire);
    s->SetCamType(cam_type);
    s->DPDrawAliasError(draw_error);
    s->DPDrawSM(drawSM);
	s->SetTexturePreviewId(ipsm_texPrev);
	s->SetWarping(do_warp);
}

