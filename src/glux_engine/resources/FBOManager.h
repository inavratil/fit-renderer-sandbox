#ifndef _FBOMANAGER_H_
#define _FBOMANAGER_H_

#include "globals.h"
#include "Singleton.h"
#include "FBO.h"


#define FBO_COLOR0 GL_COLOR_ATTACHMENT0

//-- FBO creation mode
enum FBOModes { FBO_NO_DEPTH, FBO_DEPTH_ONLY, FBO_DEPTH_AND_STENCIL };

class FBOManager : public Singleton<FBOManager>
{ 

//-----------------------------------------------------------------------------
//-- Member variables

protected:
	///associative array with all FBOs
	map<string,FBOPtr> m_fbos;
    ///iterator for fbos container
    map<string,FBOPtr>::iterator m_it_fbos;

	//-- ID of lastly bound FBO
	GLuint			m_lastFBO;


//-----------------------------------------------------------------------------
//-- Public methods

public:
	FBOManager(void);
	virtual ~FBOManager(void);

	void Add( const char* _name, FBOPtr _fbo );
	FBOPtr Get( const char* _name );
	GLuint GetID( const char* _name );
	
	FBOPtr CreateFBO( const char* _name = "", unsigned _mode = FBO_DEPTH_ONLY );
	FBOPtr CreateFBOAndAttachTexture( const char* _name, GLuint _tex, GLenum _target = FBO_BOTH );
	//TODO: pro vice FBO? a textur?
	//static GLuint CreateAndAttachNum( GLuint _tex, GLenum _target = FBO_BOTH );

	GLuint BindBuffer( const char* _name, GLenum _target = FBO_BOTH );
	void UnbindBuffer( const char* _name, GLenum _target = FBO_BOTH );
	
//-----------------------------------------------------------------------------
//-- Static methods

public:

	static bool CheckFBO();
};

typedef FBOManager* FBOManagerPtr;

#endif

