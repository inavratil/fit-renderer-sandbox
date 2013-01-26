#ifndef _POLYNOMIALWARPEDSHADOW_H_
#define _POLYNOMIALWARPEDSHADOW_H_

#include "IShadowTechnique.h"

class PolynomialWarpedShadow :
	public IShadowTechnique
{
protected:
	glm::mat4 m_matCoeffsX;
	glm::mat4 m_matCoeffsY;

public:
	PolynomialWarpedShadow();
	PolynomialWarpedShadow( glm::mat4 _coeffsX, glm::mat4 _coeffsY );
	~PolynomialWarpedShadow(void);

	//-- Virtual methods from parent class
	virtual bool Initialize(TLight* _light);
	virtual void PreRender();
	virtual void PostRender();

	virtual glm::vec2 ComputeDiff( glm::vec2 _P );

	//-- Class-specific methods
	void SetMatrices( glm::mat4 _coeffsX, glm::mat4 _coeffsY );
	inline void SetMatCoeffsX( glm::mat4 _coeffsX );
	inline void SetMatCoeffsY( glm::mat4 _coeffsY );
	inline glm::mat4 GetMatCoeffsX();
	inline glm::mat4 GetMatCoeffsY();

protected:
	glm::mat4 compute2DPolynomialCoeffsX( float *z );
	glm::mat4 compute2DPolynomialCoeffsY( float *z );
};

#endif
