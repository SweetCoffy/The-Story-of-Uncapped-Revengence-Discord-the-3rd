// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2020 by Jaime "Lactozilla" Passos.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file gl_shaders.c
/// \brief OpenGL shaders

#include "gl_shaders.h"
#include "../r_glcommon/r_glcommon.h"

// NOTE: HAVE_GLES2 should imply GL_SHADERS

boolean gl_shadersenabled = false;
hwdshaderoption_t gl_allowshaders = HWD_SHADEROPTION_OFF;

typedef GLuint (R_GL_APIENTRY *PFNglCreateShader)       (GLenum);
typedef void   (R_GL_APIENTRY *PFNglShaderSource)       (GLuint, GLsizei, const GLchar**, GLint*);
typedef void   (R_GL_APIENTRY *PFNglCompileShader)      (GLuint);
typedef void   (R_GL_APIENTRY *PFNglGetShaderiv)        (GLuint, GLenum, GLint*);
typedef void   (R_GL_APIENTRY *PFNglGetShaderInfoLog)   (GLuint, GLsizei, GLsizei*, GLchar*);
typedef void   (R_GL_APIENTRY *PFNglDeleteShader)       (GLuint);
typedef GLuint (R_GL_APIENTRY *PFNglCreateProgram)      (void);
typedef void   (R_GL_APIENTRY *PFNglDeleteProgram)      (GLuint);
typedef void   (R_GL_APIENTRY *PFNglAttachShader)       (GLuint, GLuint);
typedef void   (R_GL_APIENTRY *PFNglLinkProgram)        (GLuint);
typedef void   (R_GL_APIENTRY *PFNglGetProgramiv)       (GLuint, GLenum, GLint*);
typedef void   (R_GL_APIENTRY *PFNglUseProgram)         (GLuint);
typedef void   (R_GL_APIENTRY *PFNglUniform1i)          (GLint, GLint);
typedef void   (R_GL_APIENTRY *PFNglUniform1f)          (GLint, GLfloat);
typedef void   (R_GL_APIENTRY *PFNglUniform2f)          (GLint, GLfloat, GLfloat);
typedef void   (R_GL_APIENTRY *PFNglUniform3f)          (GLint, GLfloat, GLfloat, GLfloat);
typedef void   (R_GL_APIENTRY *PFNglUniform4f)          (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void   (R_GL_APIENTRY *PFNglUniform1fv)         (GLint, GLsizei, const GLfloat*);
typedef void   (R_GL_APIENTRY *PFNglUniform2fv)         (GLint, GLsizei, const GLfloat*);
typedef void   (R_GL_APIENTRY *PFNglUniform3fv)         (GLint, GLsizei, const GLfloat*);
typedef void   (R_GL_APIENTRY *PFNglUniformMatrix4fv)   (GLint, GLsizei, GLboolean, const GLfloat *);
typedef GLint  (R_GL_APIENTRY *PFNglGetUniformLocation) (GLuint, const GLchar*);
typedef GLint  (R_GL_APIENTRY *PFNglGetAttribLocation)  (GLuint, const GLchar*);

static PFNglCreateShader pglCreateShader;
static PFNglShaderSource pglShaderSource;
static PFNglCompileShader pglCompileShader;
static PFNglGetShaderiv pglGetShaderiv;
static PFNglGetShaderInfoLog pglGetShaderInfoLog;
static PFNglDeleteShader pglDeleteShader;
static PFNglCreateProgram pglCreateProgram;
static PFNglDeleteProgram pglDeleteProgram;
static PFNglAttachShader pglAttachShader;
static PFNglLinkProgram pglLinkProgram;
static PFNglGetProgramiv pglGetProgramiv;
static PFNglUseProgram pglUseProgram;
static PFNglUniform1i pglUniform1i;
static PFNglUniform1f pglUniform1f;
static PFNglUniform2f pglUniform2f;
static PFNglUniform3f pglUniform3f;
static PFNglUniform4f pglUniform4f;
static PFNglUniform1fv pglUniform1fv;
static PFNglUniform2fv pglUniform2fv;
static PFNglUniform3fv pglUniform3fv;
static PFNglUniformMatrix4fv pglUniformMatrix4fv;
static PFNglGetUniformLocation pglGetUniformLocation;
static PFNglGetAttribLocation pglGetAttribLocation;

gl_shader_t gl_shaders[HWR_MAXSHADERS];
gl_shader_t gl_usershaders[HWR_MAXSHADERS];
shadersource_t gl_customshaders[HWR_MAXSHADERS];

gl_shaderstate_t gl_shaderstate;

static GLRGBAFloat shader_defaultcolor = {1.0f, 1.0f, 1.0f, 1.0f};

// Shader info
static INT32 shader_leveltime = 0;

#ifdef HAVE_GLES2
#include "shaders_gles2.h"
#else
#include "shaders_gl2.h"
#endif

// ================
//  Shader sources
// ================

static struct {
	const char *vertex;
	const char *fragment;
} const gl_shadersources[] = {
	// Default shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_DEFAULT_FRAGMENT_SHADER},

	// Floor shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER},

	// Wall shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER},

	// Sprite shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER},

	// Model shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SOFTWARE_FRAGMENT_SHADER},

	// Model shader + diffuse lighting from above
	{GLSL_MODEL_LIGHTING_VERTEX_SHADER, GLSL_SOFTWARE_MODEL_LIGHTING_FRAGMENT_SHADER},

	// Water shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_WATER_FRAGMENT_SHADER},

	// Fog shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_FOG_FRAGMENT_SHADER},

	// Sky shader
	{GLSL_DEFAULT_VERTEX_SHADER, GLSL_SKY_FRAGMENT_SHADER},

	{NULL, NULL},
};

void Shader_LoadFunctions(void)
{
	pglCreateShader = GLBackend_GetFunction("glCreateShader");
	pglShaderSource = GLBackend_GetFunction("glShaderSource");
	pglCompileShader = GLBackend_GetFunction("glCompileShader");
	pglGetShaderiv = GLBackend_GetFunction("glGetShaderiv");
	pglGetShaderInfoLog = GLBackend_GetFunction("glGetShaderInfoLog");
	pglDeleteShader = GLBackend_GetFunction("glDeleteShader");
	pglCreateProgram = GLBackend_GetFunction("glCreateProgram");
	pglDeleteProgram = GLBackend_GetFunction("glDeleteProgram");
	pglAttachShader = GLBackend_GetFunction("glAttachShader");
	pglLinkProgram = GLBackend_GetFunction("glLinkProgram");
	pglGetProgramiv = GLBackend_GetFunction("glGetProgramiv");
	pglUseProgram = GLBackend_GetFunction("glUseProgram");
	pglUniform1i = GLBackend_GetFunction("glUniform1i");
	pglUniform1f = GLBackend_GetFunction("glUniform1f");
	pglUniform2f = GLBackend_GetFunction("glUniform2f");
	pglUniform3f = GLBackend_GetFunction("glUniform3f");
	pglUniform4f = GLBackend_GetFunction("glUniform4f");
	pglUniform1fv = GLBackend_GetFunction("glUniform1fv");
	pglUniform2fv = GLBackend_GetFunction("glUniform2fv");
	pglUniform3fv = GLBackend_GetFunction("glUniform3fv");
	pglUniformMatrix4fv = GLBackend_GetFunction("glUniformMatrix4fv");
	pglGetUniformLocation = GLBackend_GetFunction("glGetUniformLocation");
	pglGetAttribLocation = GLBackend_GetFunction("glGetAttribLocation");
}

#ifdef HAVE_GLES2
int Shader_AttribLoc(int loc)
{
	gl_shader_t *shader = gl_shaderstate.current;
	int pos, attrib;

	glattribute_t LOC_TO_ATTRIB[glattribute_max] =
	{
		glattribute_position,     // LOC_POSITION
		glattribute_texcoord,     // LOC_TEXCOORD + LOC_TEXCOORD0
		glattribute_normal,       // LOC_NORMAL
		glattribute_colors,       // LOC_COLORS
		glattribute_fadetexcoord, // LOC_TEXCOORD1
	};

	if (shader == NULL)
		I_Error("Shader_AttribLoc: shader not set");

	attrib = LOC_TO_ATTRIB[loc];
	pos = shader->attributes[attrib];

	if (pos == -1)
		return 0;

	return pos;
}
#endif

//
// Shader info
// Those are given to the uniforms.
//

void Shader_SetInfo(hwdshaderinfo_t info, INT32 value)
{
	switch (info)
	{
		case HWD_SHADERINFO_LEVELTIME:
			shader_leveltime = value;
			break;
		default:
			break;
	}
}

//
// Custom shader loading
//
void Shader_LoadCustom(int number, char *code, size_t size, boolean isfragment)
{
	shadersource_t *shader;

	if (!pglUseProgram)
		return;

	if (number < 1 || number > HWR_MAXSHADERS)
		I_Error("Shader_LoadCustom: cannot load shader %d (min 1, max %d)", number, HWR_MAXSHADERS);
	else if (code == NULL)
		I_Error("Shader_LoadCustom: empty shader");

	shader = &gl_customshaders[number];

#define COPYSHADER(source) { \
	if (shader->source) \
		free(shader->source); \
	shader->source = malloc(size+1); \
	strncpy(shader->source, code, size); \
	shader->source[size] = 0; \
	}

	if (isfragment)
		COPYSHADER(fragment)
	else
		COPYSHADER(vertex)
}

void Shader_Set(int type)
{
	gl_shader_t *shader = gl_shaderstate.current;

#ifndef HAVE_GLES2
	if (gl_allowshaders == HWD_SHADEROPTION_OFF)
		return;
#endif

	if ((shader == NULL) || (GLuint)type != gl_shaderstate.type)
	{
		gl_shader_t *baseshader = &gl_shaders[type];
		gl_shader_t *usershader = &gl_usershaders[type];

		if (usershader->program)
			shader = (gl_allowshaders == HWD_SHADEROPTION_NOCUSTOM) ? baseshader : usershader;
		else
			shader = baseshader;

		gl_shaderstate.current = shader;
		gl_shaderstate.type = type;
		gl_shaderstate.changed = true;
	}

	if (gl_shaderstate.program != shader->program)
	{
		gl_shaderstate.program = shader->program;
		gl_shaderstate.changed = true;
	}

#ifdef HAVE_GLES2
	Shader_SetTransform();
	gl_shadersenabled = true;
#else
	gl_shadersenabled = (shader->program != 0);
#endif
}

void Shader_UnSet(void)
{
#ifdef HAVE_GLES2
	Shader_Set(SHADER_BASE);
	Shader_SetUniforms(NULL, &shader_defaultcolor, NULL, NULL);
#else
	gl_shaderstate.current = NULL;
	gl_shaderstate.type = 0;
	gl_shaderstate.program = 0;

	if (pglUseProgram)
		pglUseProgram(0);
	gl_shadersenabled = false;
#endif
}

void Shader_Clean(void)
{
	INT32 i;

	for (i = 1; i < HWR_MAXSHADERS; i++)
	{
		shadersource_t *shader = &gl_customshaders[i];

		if (shader->vertex)
			free(shader->vertex);

		if (shader->fragment)
			free(shader->fragment);

		shader->vertex = NULL;
		shader->fragment = NULL;
	}
}

static void Shader_CompileError(const char *message, GLuint program, INT32 shadernum)
{
	GLchar *infoLog = NULL;
	GLint logLength;

	pglGetShaderiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength)
	{
		infoLog = malloc(logLength);
		pglGetShaderInfoLog(program, logLength, NULL, infoLog);
	}

	GL_MSG_Error("Shader_CompileProgram: %s (%s)\n%s", message, HWR_GetShaderName(shadernum), (infoLog ? infoLog : ""));

	if (infoLog)
		free(infoLog);
}

static boolean Shader_CompileProgram(gl_shader_t *shader, GLint i, const GLchar *vert_shader, const GLchar *frag_shader)
{
	GLuint gl_vertShader, gl_fragShader;
	GLint result;

	//
	// Load and compile vertex shader
	//
	gl_vertShader = pglCreateShader(GL_VERTEX_SHADER);
	if (!gl_vertShader)
	{
		GL_MSG_Error("Shader_CompileProgram: Error creating vertex shader %s\n", HWR_GetShaderName(i));
		return false;
	}

	pglShaderSource(gl_vertShader, 1, &vert_shader, NULL);
	pglCompileShader(gl_vertShader);

	// check for compile errors
	pglGetShaderiv(gl_vertShader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		Shader_CompileError("Error compiling vertex shader", gl_vertShader, i);
		pglDeleteShader(gl_vertShader);
		return false;
	}

	//
	// Load and compile fragment shader
	//
	gl_fragShader = pglCreateShader(GL_FRAGMENT_SHADER);
	if (!gl_fragShader)
	{
		GL_MSG_Error("Shader_CompileProgram: Error creating fragment shader %s\n", HWR_GetShaderName(i));
		pglDeleteShader(gl_vertShader);
		pglDeleteShader(gl_fragShader);
		return false;
	}

	pglShaderSource(gl_fragShader, 1, &frag_shader, NULL);
	pglCompileShader(gl_fragShader);

	// check for compile errors
	pglGetShaderiv(gl_fragShader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		Shader_CompileError("Error compiling fragment shader", gl_fragShader, i);
		pglDeleteShader(gl_vertShader);
		pglDeleteShader(gl_fragShader);
		return false;
	}

	shader->program = pglCreateProgram();
	pglAttachShader(shader->program, gl_vertShader);
	pglAttachShader(shader->program, gl_fragShader);
	pglLinkProgram(shader->program);

	// check link status
	pglGetProgramiv(shader->program, GL_LINK_STATUS, &result);

	// delete the shader objects
	pglDeleteShader(gl_vertShader);
	pglDeleteShader(gl_fragShader);

	// couldn't link?
	if (result != GL_TRUE)
	{
		GL_MSG_Error("Shader_CompileProgram: Error linking shader program %s\n", HWR_GetShaderName(i));
		pglDeleteProgram(shader->program);
		return false;
	}

#define GETUNI(uniform) pglGetUniformLocation(shader->program, uniform)

#ifdef HAVE_GLES2
	memset(shader->projMatrix, 0x00, sizeof(fmatrix4_t));
	memset(shader->viewMatrix, 0x00, sizeof(fmatrix4_t));
	memset(shader->modelMatrix, 0x00, sizeof(fmatrix4_t));

	// transform
	shader->uniforms[gluniform_model]       = GETUNI("model");
	shader->uniforms[gluniform_view]        = GETUNI("view");
	shader->uniforms[gluniform_projection]  = GETUNI("projection");

	// samplers
	shader->uniforms[gluniform_startscreen] = GETUNI("start_screen");
	shader->uniforms[gluniform_endscreen]   = GETUNI("end_screen");
	shader->uniforms[gluniform_fademask]    = GETUNI("fade_mask");

	// misc.
	shader->uniforms[gluniform_isfadingin]  = GETUNI("is_fading_in");
	shader->uniforms[gluniform_istowhite]   = GETUNI("is_to_white");
#endif

	// lighting
	shader->uniforms[gluniform_poly_color]  = GETUNI("poly_color");
	shader->uniforms[gluniform_tint_color]  = GETUNI("tint_color");
	shader->uniforms[gluniform_fade_color]  = GETUNI("fade_color");
	shader->uniforms[gluniform_lighting]    = GETUNI("lighting");
	shader->uniforms[gluniform_fade_start]  = GETUNI("fade_start");
	shader->uniforms[gluniform_fade_end]    = GETUNI("fade_end");

	// misc. (custom shaders)
	shader->uniforms[gluniform_leveltime]   = GETUNI("leveltime");

#undef GETUNI

#ifdef HAVE_GLES2

#define GETATTRIB(attribute) pglGetAttribLocation(shader->program, attribute)

	shader->attributes[glattribute_position]     = GETATTRIB("attribute_position");
	shader->attributes[glattribute_texcoord]     = GETATTRIB("attribute_texcoord");
	shader->attributes[glattribute_normal]       = GETATTRIB("attribute_normal");
	shader->attributes[glattribute_colors]       = GETATTRIB("attribute_colors");
	shader->attributes[glattribute_fadetexcoord] = GETATTRIB("attribute_fadetexcoord");

#undef GETATTRIB

#endif

	return true;
}

boolean Shader_Compile(void)
{
	GLint i;

	if (!pglUseProgram)
		return false;

	gl_customshaders[SHADER_DEFAULT].vertex = NULL;
	gl_customshaders[SHADER_DEFAULT].fragment = NULL;

	for (i = 0; gl_shadersources[i].vertex && gl_shadersources[i].fragment; i++)
	{
		gl_shader_t *shader, *usershader;
		const GLchar *vert_shader = gl_shadersources[i].vertex;
		const GLchar *frag_shader = gl_shadersources[i].fragment;

		if (i >= HWR_MAXSHADERS)
			break;

		shader = &gl_shaders[i];
		usershader = &gl_usershaders[i];

		if (shader->program)
			pglDeleteProgram(shader->program);
		if (usershader->program)
			pglDeleteProgram(usershader->program);

		shader->program = 0;
		usershader->program = 0;

		if (!Shader_CompileProgram(shader, i, vert_shader, frag_shader))
			shader->program = 0;

		// Compile custom shader
		if ((i == SHADER_DEFAULT) || !(gl_customshaders[i].vertex || gl_customshaders[i].fragment))
			continue;

		// 18032019
		if (gl_customshaders[i].vertex)
			vert_shader = gl_customshaders[i].vertex;
		if (gl_customshaders[i].fragment)
			frag_shader = gl_customshaders[i].fragment;

		if (!Shader_CompileProgram(usershader, i, vert_shader, frag_shader))
		{
			GL_MSG_Warning("Shader_Compile: Could not compile custom shader program for %s\n", HWR_GetShaderName(i));
			usershader->program = 0;
		}
	}

	Shader_Set(SHADER_DEFAULT);

#ifdef HAVE_GLES2
	pglUseProgram(gl_shaderstate.program);
	gl_shaderstate.changed = false;
#endif

	return true;
}

void Shader_SetColors(FSurfaceInfo *Surface, GLRGBAFloat *poly, GLRGBAFloat *tint, GLRGBAFloat *fade)
{
	gl_shader_t *shader = gl_shaderstate.current;

	if (gl_shadersenabled && (shader != NULL) && pglUseProgram)
	{
		if (!shader->program)
			return;

		if (gl_shaderstate.changed)
		{
			pglUseProgram(shader->program);
			gl_shaderstate.changed = false;
		}

		Shader_SetUniforms(Surface, poly, tint, fade);
	}
}

#ifdef HAVE_GLES2
void Shader_SetTransform(void)
{
	gl_shader_t *shader = gl_shaderstate.current;
	if (!shader)
		return;

	if (memcmp(projMatrix, shader->projMatrix, sizeof(fmatrix4_t)))
	{
		memcpy(shader->projMatrix, projMatrix, sizeof(fmatrix4_t));
		pglUniformMatrix4fv(shader->uniforms[gluniform_projection], 1, GL_FALSE, (float *)projMatrix);
	}

	if (memcmp(viewMatrix, shader->viewMatrix, sizeof(fmatrix4_t)))
	{
		memcpy(shader->viewMatrix, viewMatrix, sizeof(fmatrix4_t));
		pglUniformMatrix4fv(shader->uniforms[gluniform_view], 1, GL_FALSE, (float *)viewMatrix);
	}

	if (memcmp(modelMatrix, shader->modelMatrix, sizeof(fmatrix4_t)))
	{
		memcpy(shader->modelMatrix, modelMatrix, sizeof(fmatrix4_t));
		pglUniformMatrix4fv(shader->uniforms[gluniform_model], 1, GL_FALSE, (float *)modelMatrix);
	}
}
#endif

void Shader_SetUniforms(FSurfaceInfo *Surface, GLRGBAFloat *poly, GLRGBAFloat *tint, GLRGBAFloat *fade)
{
	gl_shader_t *shader = gl_shaderstate.current;

	if (gl_shadersenabled && (shader != NULL) && pglUseProgram)
	{
		if (!shader->program)
		{
			pglUseProgram(0);
			return;
		}

		if (gl_shaderstate.changed)
		{
			pglUseProgram(shader->program);
			gl_shaderstate.changed = false;
		}

		// Color uniforms can be left NULL and will be set to white (1.0f, 1.0f, 1.0f, 1.0f)
		if (poly == NULL)
			poly = &shader_defaultcolor;
		if (tint == NULL)
			tint = &shader_defaultcolor;
		if (fade == NULL)
			fade = &shader_defaultcolor;

		#define UNIFORM_1(uniform, a, function) \
			if (uniform != -1) \
				function (uniform, a);

		#define UNIFORM_2(uniform, a, b, function) \
			if (uniform != -1) \
				function (uniform, a, b);

		#define UNIFORM_3(uniform, a, b, c, function) \
			if (uniform != -1) \
				function (uniform, a, b, c);

		#define UNIFORM_4(uniform, a, b, c, d, function) \
			if (uniform != -1) \
				function (uniform, a, b, c, d);

		// polygon
		UNIFORM_4(shader->uniforms[gluniform_poly_color], poly->red, poly->green, poly->blue, poly->alpha, pglUniform4f);
		UNIFORM_4(shader->uniforms[gluniform_tint_color], tint->red, tint->green, tint->blue, tint->alpha, pglUniform4f);
		UNIFORM_4(shader->uniforms[gluniform_fade_color], fade->red, fade->green, fade->blue, fade->alpha, pglUniform4f);

		if (Surface != NULL)
		{
			UNIFORM_1(shader->uniforms[gluniform_lighting], Surface->LightInfo.light_level, pglUniform1f);
			UNIFORM_1(shader->uniforms[gluniform_fade_start], Surface->LightInfo.fade_start, pglUniform1f);
			UNIFORM_1(shader->uniforms[gluniform_fade_end], Surface->LightInfo.fade_end, pglUniform1f);
		}

		UNIFORM_1(shader->uniforms[gluniform_leveltime], ((float)shader_leveltime) / TICRATE, pglUniform1f);

		#undef UNIFORM_1
		#undef UNIFORM_2
		#undef UNIFORM_3
		#undef UNIFORM_4
	}
}

void Shader_SetSampler(gluniform_t uniform, GLint value)
{
	gl_shader_t *shader = gl_shaderstate.current;
	if (!shader)
		return;

	if (gl_shaderstate.changed)
	{
		pglUseProgram(shader->program);
		gl_shaderstate.changed = false;
	}

	if (shader->uniforms[uniform] != -1)
		pglUniform1i(shader->uniforms[uniform], value);
}
