/**
****************************************************************************************************
****************************************************************************************************
@file: texture.cpp
@brief load TGA textures from file and converts them to suitable OpenGL format. TGA files can be
* compressed or uncompressed
Loading TGA texture code adapted from: http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=33
****************************************************************************************************
***************************************************************************************************/
#include "texture.h"

/**
****************************************************************************************************
@brief reset basic values (null name and texture data)
****************************************************************************************************/
Texture::Texture()
{
    m_texname = "null";
    m_textype = BASE;
    m_imageData = NULL;
    m_texID = m_width = m_height = m_bpp = 0;
    m_texmode = MODULATE;
    m_tileX = m_tileY = 1.0;
    m_texLoc = m_tileXLoc = m_tileYLoc = -1;
}

/**
****************************************************************************************************
@brief free all texture data by detaching texture
@todo texture destruction might be called after GL context is destroyed, so it has no effect
****************************************************************************************************/
Texture::~Texture()
{
    glDeleteTextures(1,&m_texID);
}

/**
****************************************************************************************************
@brief load TGA texture as texture image. If texture has been loaded, a texture pointer from cache is used
instead of reloading from file
@param texname texture name
@param textype texture type (can be BASE,ENV,BUMP)
@param filename external texture file (.tga)
@param texmode texture addition mode (can be ADD,MODULATE,DECAL,BLEND)
@param intensity texture color intensity (or bump intensity)
@param tileX count of horizontal tiles
@param tileY count of vertical tiles
@param mipmap sholud we generate mipmaps for texture?
@param aniso should we use anisotropic filtering?
@param cache whether we should use cached texture image or load new
@return new texture ID
****************************************************************************************************/
GLint Texture::Load(const char *texname, int textype, const char *filename, int texmode,
    GLfloat intensity, GLfloat tileX, GLfloat tileY, bool mipmap, bool aniso, GLint cache)
{
    ///if texture is loaded in cache, copy only pointer to texture data
    if(cache != -1)
    {
        m_texID = cache;
    }
    else      //load texture from file
    {
        if(!LoadTGA(filename))
            return ERR;

        //texture generation
        glGenTextures(1, &m_texID);
        glBindTexture(GL_TEXTURE_2D, m_texID);

        //texture with anisotropic filtering
        if(aniso)
        {
            //find out, if GFX supports aniso filtering
            if(!GLEW_EXT_texture_filter_anisotropic)
            {
                cout<<"Anisotropic filtering not supported. Using linear instead.\n";
            }
            else
            {
                float maxAnisotropy;
                //find out maximum supported anisotropy
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
            }
        }
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        //mip-mapped texture (if set)
        if(mipmap)
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        else
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

        if(textype == BUMP)	//don't compress bump maps
#ifdef _LINUX_
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_imageData);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_imageData);
#endif
        else					//compress color maps
#ifdef _LINUX_
            glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_imageData);
#else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_imageData);
#endif
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if(mipmap)
            glGenerateMipmap(GL_TEXTURE_2D);

        delete [] m_imageData;      //don't need image data after texture was created
    }

    //texture mode and type
    m_texname = texname;
    m_textype = textype;
    m_texmode = texmode;
    m_tileX = tileX;
    m_tileY = tileY;
    m_intensity = intensity;

    //cout<<"Texture \""<<texname<<"\" creation OK\n";

    return m_texID;
}


/**
****************************************************************************************************
@brief load 6 TGA texture as cube map. If texture has been loaded, a texture pointer from cache is used
instead of reloading from file
@param texname texture name
@param textype texture type (can be BASE,ENV,BUMP)
@param cube_files array of strings with filenames containing every side of cube map (.tga)
@param texmode texture addition mode (can be ADD,MODULATE,DECAL,BLEND)
@param intensity texture color intensity (or bump intensity)
@param tileX count of horizontal tiles
@param tileY count of vertical tiles
@param aniso should we use anisotropic filtering?
@param cache whether we should use cached texture image or load new
@return new texture ID
****************************************************************************************************/

GLint Texture::Load(const char *texname, int textype,
    const char **cube_files, int texmode, GLfloat intensity, GLfloat tileX, GLfloat tileY, bool aniso, GLint cache)
{
    ///if texture is loaded in cache, copy only pointer to texture data
    if(cache != -1)
    {
        m_texID = cache;
    }
    else      //load texture from file
    {
        //texture generation
        glGenTextures(1, &m_texID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);

        //load 6 images for cube map
        for(int i = 0; i < 6; i++)
        {
            if(!LoadTGA(cube_files[i]))
                return ERR;

            //texture with anisotropic filtering
            if(aniso)
            {
                //find out, if GFX supports aniso filtering
                if(!GLEW_EXT_texture_filter_anisotropic)
                {
                    cout<<"Anisotropic filtering not supported. Using linear instead.\n";
                }
                else
                {
                    float maxAnisotropy;
                    //find out maximum supported anisotropy
                    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
                    glTexParameterf(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
                }
            }

#ifdef _LINUX_
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COMPRESSED_RGB, m_width, m_height, 0, GL_BGR, GL_UNSIGNED_BYTE, m_imageData);
#else
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_COMPRESSED_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_imageData);
#endif

            delete [] m_imageData;      //don't need image data after texture was created
        }
    }

    //texture mode and type
    m_texname = texname;
    m_textype = textype;
    m_texmode = texmode;
    m_tileX = tileX;
    m_tileY = tileY;
    m_intensity = intensity;

    //cout<<"Texture \""<<texname<<"\" creation OK\n";

    return m_texID;
}

/**
****************************************************************************************************
@brief Create picture directly from data
@param texname texture name
@param textype texture type (can be BASE,ENV,BUMP)
@param texdata texture data in any format (int, float...)
@param tex_size texture width and height
@param tex_format OpenGL texture format (can be GL_RGBA, GL_RGBA16F or GL_RGBA32F)
@param tex_type internal data format (can be GL_UNSIGNED_BYTE or GL_FLOAT)
@param texmode texture addition mode (can be ADD,MODULATE,DECAL,BLEND)
@param intensity texture color intensity (or bump intensity)
@param tileX count of horizontal tiles
@param tileY count of vertical tiles
@param mipmap sholud we generate mipmaps for texture?
@param aniso should we use anisotropic filtering?
@return new texture ID
****************************************************************************************************/
GLint Texture::Load(const char *texname, int textype, const void *texdata, glm::vec2 tex_size, GLenum tex_format, GLenum data_type,
                    int texmode, GLfloat intensity, GLfloat tileX, GLfloat tileY, bool mipmap, bool aniso)
{
    m_width = (int)tex_size.x;
    m_height = (int)tex_size.y;

    //texture generation
    glGenTextures(1, &m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);

    //texture with anisotropic filtering
    if(aniso)
    {
        //find out, if GFX supports aniso filtering
        if(!GLEW_EXT_texture_filter_anisotropic)
            cout<<"Anisotropic filtering not supported. Using linear instead.\n";
        else
        {
            float maxAnisotropy;
            //find out maximum supported anisotropy
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
    }
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    //mip-mapped texture (if set)
    if(mipmap)
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

#ifdef _LINUX_
        glTexImage2D(GL_TEXTURE_2D, 0, tex_format, m_width, m_height, 0, GL_BGRA, data_type, texdata);
#else
        glTexImage2D(GL_TEXTURE_2D, 0, tex_format, m_width, m_height, 0, GL_RGBA, data_type, texdata);
#endif

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if(mipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

    //texture mode and type
    m_texname = texname;
    m_textype = textype;
    m_texmode = texmode;
    m_tileX = tileX;
    m_tileY = tileY;
    m_intensity = intensity;

    //cout<<"Texture \""<<texname<<"\" creation OK\n";

    return m_texID;
}


/**
****************************************************************************************************
@brief load and decode TGA image from file
@param filename file with image data
@return success/fail of texture loading
****************************************************************************************************/
bool Texture::LoadTGA(const char* filename)
{
    FILE* fTGA;											//file
    int type;

    GLubyte tgaheader[12];                             //TGA header

    //headers: for compressed and for uncompressed images
    GLubyte uTGAcompare[12] = { 0,0, 2,0,0,0,0,0,0,0,0,0 };
    GLubyte cTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };

    //open file
    fTGA = fopen(filename, "rb");
    if(fTGA == NULL)
    {
        string err = "Can't open texture file ";
        err += filename;
        err += " !";
        ShowMessage(err.c_str(), false);
        return false;
    }

    cout<<"Loading image "<<filename<<"...";

    ///read and check file header
    if(fread(&tgaheader, 12, 1, fTGA) == 0)
    {
        ShowMessage("Can't read image header!");
        if(fTGA != NULL) fclose(fTGA);
        return false;
    }

    ///find out from image, whether data are compressed or not
    if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
        type = UNCOMPRESSED;
    else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)
        type = COMPRESSED;
    //else error
    else
    {
        ShowMessage("Unknown TGA image type!");
        fclose(fTGA);
        return false;
    }

    //read further bytes of header
    if(fread(tgaheader, 6, 1, fTGA) == 0)
    {
        ShowMessage("Can't read image header!");
        if(fTGA != NULL) fclose(fTGA);
        return false;
    }

    ///extract width, height and bpp of image
    m_width = tgaheader[1] * 256 + tgaheader[0];
    m_height = tgaheader[3] * 256 + tgaheader[2];
    m_bpp = tgaheader[4];

    cout<<m_width<<"x"<<m_height<<"x"<<m_bpp<<"...";

    //check values
    if((m_width <= 0) || (m_height <= 0) || ((m_bpp != 24) && (m_bpp != 32)) )
    {
        ShowMessage("Unknown TGA image type!");
        if(fTGA != NULL) fclose(fTGA);
        return false;
    }
    /*
    //set color depth
    int gl_textype;
    if(m_bpp == 24)
    gl_textype = GL_RGB;
    else
    gl_textype = GL_RGBA;
    */
    ///allocate memory for image data
    unsigned bytesPerPixel = (m_bpp / 8);
    unsigned imageSize = (bytesPerPixel * m_width * m_height);
    m_imageData = new GLubyte[imageSize];
    if(m_imageData == NULL)
    {
        ShowMessage("Can't allocate image data!");
        fclose(fTGA);
        return false;
    }

    ///load from file according to compression
    if(type == UNCOMPRESSED)
    {
        ///uncompressed image - read data directly from file
        if(fread(m_imageData, 1, imageSize, fTGA) != imageSize)
        {
            ShowMessage("Can't read image data!");
            if(m_imageData != NULL) delete [] m_imageData;
            fclose(fTGA);
            return false;
        }
#ifndef _LINUX_
        //conversion from BGR to RGB
        for(unsigned cswap = 0; cswap < imageSize; cswap += bytesPerPixel)
            m_imageData[cswap] ^= m_imageData[cswap+2] ^= m_imageData[cswap] ^= m_imageData[cswap+2];
#endif
    }

    ///compressed image - decode using RLE compression
    else if(type == COMPRESSED)
    {
        GLuint pixelcount = m_height * m_width;			//pixel count
        GLuint currentpixel = 0;					//actual pixel
        GLuint currentbyte = 0;						//actual byte

        //allocate memory for image pixel
        GLubyte* colorbuffer = new GLubyte[bytesPerPixel];
        if(colorbuffer == NULL)
        {
            ShowMessage("Can't allocate color buffer data!");
            fclose(fTGA);
            return false;
        }

        do
        {
            //find out header type: RLE or RAW
            GLubyte chunkheader = 0;
            if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)
            {
                ShowMessage("Can't read RLE header!");
                if(fTGA != NULL) fclose(fTGA);
                if(m_imageData != NULL) delete [] m_imageData;
                if(colorbuffer != NULL) delete [] colorbuffer;
                return false;
            }

            //1. RAW part of file
            if(chunkheader < 128)
            {
                chunkheader++;				//count pixels before other header
                //nactavame kazdy pixel
                for(short counter = 0; counter < chunkheader; counter++)
                {
                    if(fread(colorbuffer, 1, bytesPerPixel, fTGA) != bytesPerPixel)
                    {
                        ShowMessage("Can't read image data!");
                        if(fTGA != NULL) fclose(fTGA);
                        if(colorbuffer != NULL) delete [] colorbuffer;
                        if(m_imageData != NULL) delete [] m_imageData;
                        return false;
                    }
                    //write to memory, conversion to RGB
#ifdef _LINUX_
                    m_imageData[currentbyte] = colorbuffer[0];
                    m_imageData[currentbyte + 1] = colorbuffer[1];
                    m_imageData[currentbyte + 2] = colorbuffer[2];
#else
                    m_imageData[currentbyte] = colorbuffer[2];
                    m_imageData[currentbyte + 1] = colorbuffer[1];
                    m_imageData[currentbyte + 2] = colorbuffer[0];
#endif
                    ///@todo alpha channel
                    //if(bytesPerPixel == 4) m_imageData[currentbyte + 3] = colorbuffer[3];
                    //move to next byte
                    currentbyte += bytesPerPixel;
                    currentpixel++;

                    //check pixel count, if it's according to image dimensions
                    if(currentpixel > pixelcount)
                    {
                        ShowMessage("Wrong image format - too many pixels.");
                        if(fTGA != NULL) fclose(fTGA);
                        if(colorbuffer != NULL) delete [] colorbuffer;
                        if(m_imageData != NULL) delete [] m_imageData;
                        return false;
                    }
                }
            }
            //2. RLE part of image
            else
            {
                chunkheader -= 127;						//pixel count in section
                //read pixel after pixel
                if(fread(colorbuffer, 1, bytesPerPixel, fTGA) != bytesPerPixel)
                {
                    ShowMessage("Can't read image data!");
                    if(fTGA != NULL) fclose(fTGA);
                    if(colorbuffer != NULL) delete [] colorbuffer;
                    if(m_imageData != NULL) delete [] m_imageData;
                    return false;
                }
                //pixel copy
                for(short counter = 0; counter < chunkheader; counter++)
                {
                    //write to memory, conversion to RGB
#ifdef _LINUX_
                    m_imageData[currentbyte] = colorbuffer[0];
                    m_imageData[currentbyte + 1] = colorbuffer[1];
                    m_imageData[currentbyte + 2] = colorbuffer[2];
#else
                    m_imageData[currentbyte] = colorbuffer[2];
                    m_imageData[currentbyte + 1] = colorbuffer[1];
                    m_imageData[currentbyte + 2] = colorbuffer[0];
#endif
                    //TODO: alpha channel
                    //if(bytesPerPixel == 4) m_imageData[currentbyte + 3] = colorbuffer[3];
                    //move to next byte
                    currentbyte += bytesPerPixel;
                    currentpixel++;

                    //check pixel count, if it's according to image dimensions
                    if(currentpixel > pixelcount)
                    {
                        ShowMessage("Wrong image format - too many pixels.");
                        if(fTGA != NULL) fclose(fTGA);
                        if(colorbuffer != NULL) delete [] colorbuffer;
                        if(m_imageData != NULL) delete [] m_imageData;
                        return false;
                    }
                }
            }
        }while(currentpixel < pixelcount);

        //clear up memory
        if(colorbuffer != NULL) delete [] colorbuffer;
    }

    //done
    fclose(fTGA);
    cout<<"OK\n";
    return true;
}


/**
****************************************************************************************************
@brief Activates texture map for use by shader
@param tex_unit multitexture unit used for texture application
@param set_uniforms shall we also set uniform locations to shader?
****************************************************************************************************/
void Texture::ActivateTexture(GLint tex_unit, bool set_uniforms)
{
    ///1. set uniform values in shader (tiles, texture unit, texture matrix and intensity)
    if(set_uniforms)
    {
        if(m_tileXLoc > 0 && m_tileYLoc > 0)
        {
            glUniform1f(m_tileXLoc, m_tileX);
            glUniform1f(m_tileYLoc, m_tileY);
        }
        glUniform1f(m_intensityLoc, m_intensity);
    }
    //texture location must be updated regularly
    if(m_texLoc >= 0)
        glUniform1i(m_texLoc, tex_unit);

    ///2. activate and bind texture
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    //Various texture targets
    if(m_textype == CUBEMAP || m_textype == CUBEMAP_ENV)        //for cube map
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texID);
    else if(m_textype == SHADOW_OMNI)                         //for texture array
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_texID);
    else if(m_textype == RENDER_TEXTURE_MULTISAMPLE)          //for multisampled texture
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_texID);
    else                                                    //for regular 2D texture
        glBindTexture(GL_TEXTURE_2D, m_texID);
}

/**
****************************************************************************************************
@brief Gets uniform variables froms shader
@param shader handle to shader to bound with texture
****************************************************************************************************/
void Texture::GetUniforms(GLuint shader)
{
    if(HasTiles())
    {
        string tileX_str = m_texname + "_tileX";
        string tileY_str = m_texname + "_tileY";
        m_tileXLoc = glGetUniformLocation(shader,tileX_str.c_str() );
        m_tileYLoc = glGetUniformLocation(shader,tileY_str.c_str() );
    }

    ///1. to avoid mismatch variable names, use texture name as variable prefix
    string intensity_str = m_texname + "_intensity";

    ///2. get uniforms location
    m_texLoc = glGetUniformLocation(shader,m_texname.c_str());
    m_intensityLoc = glGetUniformLocation(shader,intensity_str.c_str() );
}
