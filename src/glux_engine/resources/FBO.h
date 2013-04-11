#ifndef _FBO_H_
#define _FBO_H_

#include "globals.h"

#define FBO_READ GL_READ_FRAMEBUFFER
#define FBO_DRAW GL_DRAW_FRAMEBUFFER
#define FBO_BOTH GL_FRAMEBUFFER

//-- FBO creation mode
enum FBOModes { FBO_NO_DEPTH, FBO_DEPTH_ONLY, FBO_DEPTH_AND_STENCIL };

class FBO
{ 

//-----------------------------------------------------------------------------
//-- Member variables

protected:
	GLuint			m_id;
	GLuint			m_lastFBO;	
	unsigned int	m_width;
	unsigned int	m_height;
	
//-----------------------------------------------------------------------------
//-- Public methods 

public:
	FBO(void);
	virtual ~FBO(void);

	void Init();
	void Destroy();

	//-- Set/Get id
	void SetID( GLuint _id ){ m_id = _id; }
	GLuint GetID(){ return m_id; }

	GLuint Bind( GLenum _target = FBO_BOTH );
	void Unbind( GLenum _target = FBO_BOTH );

	void AttachColorTexture( GLuint _tex, unsigned _attachment = 0 );
	void AttachDepthTexture( GLuint _tex );
	void AttachDepthBuffer( unsigned _mode );
	bool CheckStatus(); 

};

typedef FBO* FBOPtr;

#endif

