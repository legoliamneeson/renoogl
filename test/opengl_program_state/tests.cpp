// Linux/Mesa integration test for the portable native program-state helper.
// See README_OPENGL.md for the generated GLAD dependency and build command.
#include <cassert>
#include <cstdio>
#include <dlfcn.h>
#include "../../external/reshade/source/opengl/opengl_program_state.hpp"
using namespace reshade::opengl;
static GladGLContext gl;
using E=void*;
extern "C" {
E eglGetPlatformDisplay(unsigned, void*, const intptr_t*);
unsigned eglInitialize(E,int*,int*);
unsigned eglBindAPI(unsigned);
unsigned eglChooseConfig(E,const int*,E*,int,int*);
E eglCreatePbufferSurface(E,E,const int*);
E eglCreateContext(E,E,E,const int*);
unsigned eglMakeCurrent(E,E,E,E);
void (*eglGetProcAddress(const char*))();
unsigned eglDestroyContext(E,E);
unsigned eglDestroySurface(E,E);
unsigned eglTerminate(E);
}
GLuint compile(GLenum type,const char* source) {
 auto id=gl.CreateShader(type);gl.ShaderSource(id,1,&source,nullptr);gl.CompileShader(id);
 GLint ok;gl.GetShaderiv(id,GL_COMPILE_STATUS,&ok);
 if(!ok){char log[4096];gl.GetShaderInfoLog(id,4096,nullptr,log);std::puts(log);} assert(ok);return id;
}
GLuint program(const char* fragment, GLuint original=0) {
 const char* vertex="#version 430\nin vec4 position; void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);gl_Position=vec4(p*2.-1.,0.,1.)+position;}";
 auto vs=compile(GL_VERTEX_SHADER,vertex),fs=compile(GL_FRAGMENT_SHADER,fragment),p=gl.CreateProgram();
 gl.AttachShader(p,vs);gl.AttachShader(p,fs);
 if(original) assert(copy_program_interfaces(gl,original,p));else gl.BindAttribLocation(p,5,"position");
 gl.LinkProgram(p);GLint ok;gl.GetProgramiv(p,GL_LINK_STATUS,&ok);assert(ok);
 gl.DeleteShader(vs);gl.DeleteShader(fs);return p;
}
int main(){
 auto display=eglGetPlatformDisplay(0x31DD,nullptr,nullptr);int major,minor;assert(eglInitialize(display,&major,&minor));assert(eglBindAPI(0x30A2));
 int ca[]={0x3033,1,0x3040,8,0x3038};E cfg;int n;assert(eglChooseConfig(display,ca,&cfg,1,&n)&&n);
 int sa[]={0x3057,1,0x3056,1,0x3038};auto surface=eglCreatePbufferSurface(display,cfg,sa);
 int xa[]={0x3098,4,0x30FB,3,0x30FD,1,0x3038};auto context=eglCreateContext(display,cfg,nullptr,xa);assert(context);assert(eglMakeCurrent(display,surface,surface,context));
 assert(gladLoadGLContext(&gl,reinterpret_cast<GLADloadfunc>(eglGetProcAddress)));
 std::printf("GL %s\n",gl.GetString(GL_VERSION));
 const char* fragment=R"(#version 430
 uniform vec4 color; uniform float factors[2]; uniform mat2 transform;
 uniform int mode; uniform uint mask; uniform sampler2D tex;
 layout(std140) uniform Game { vec4 game; };
 layout(std430) buffer Storage { vec4 data; };
 out vec4 result;
 void main(){result=color*factors[0]+factors[1]*game+vec4(transform[0],float(mode),float(mask))+data+texture(tex,vec2(0.5));})";
 auto original=program(fragment),replacement=program(fragment,original);
 assert(gl.GetAttribLocation(replacement,"position")==5);
 auto state=std::make_shared<program_replacement_state>();state->original=original;state->replacement=replacement;assert(build_program_state(gl,state.get()));
 gl.UseProgram(original);
 float color[]={0.1f,0.2f,0.3f,0.4f},factors[]={2,3},matrix[]={1,2,3,4};
 gl.Uniform4fv(gl.GetUniformLocation(original,"color"),1,color);gl.Uniform1fv(gl.GetUniformLocation(original,"factors"),2,factors);
 gl.UniformMatrix2fv(gl.GetUniformLocation(original,"transform"),1,GL_FALSE,matrix);
 gl.Uniform1i(gl.GetUniformLocation(original,"mode"),7);gl.Uniform1ui(gl.GetUniformLocation(original,"mask"),9);gl.Uniform1i(gl.GetUniformLocation(original,"tex"),4);
 auto block=gl.GetUniformBlockIndex(original,"Game");gl.UniformBlockBinding(original,block,6);
 auto ssbo=gl.GetProgramResourceIndex(original,GL_SHADER_STORAGE_BLOCK,"Storage");gl.ShaderStorageBlockBinding(original,ssbo,7);
 {program_replacement_scope scope(gl,state);GLint p;gl.GetIntegerv(GL_CURRENT_PROGRAM,&p);assert(GLuint(p)==replacement);
 float actual[4];gl.GetUniformfv(replacement,gl.GetUniformLocation(replacement,"color"),actual);for(int i=0;i<4;i++)assert(actual[i]==color[i]);
 gl.GetUniformfv(replacement,gl.GetUniformLocation(replacement,"factors[1]"),actual);assert(actual[0]==3);
 gl.GetUniformfv(replacement,gl.GetUniformLocation(replacement,"transform"),actual);for(int i=0;i<4;i++)assert(actual[i]==matrix[i]);
 GLint value;gl.GetUniformiv(replacement,gl.GetUniformLocation(replacement,"mode"),&value);assert(value==7);
 GLuint uv;gl.GetUniformuiv(replacement,gl.GetUniformLocation(replacement,"mask"),&uv);assert(uv==9);
 gl.GetUniformiv(replacement,gl.GetUniformLocation(replacement,"tex"),&value);assert(value==4);
 gl.GetActiveUniformBlockiv(replacement,gl.GetUniformBlockIndex(replacement,"Game"),GL_UNIFORM_BLOCK_BINDING,&value);assert(value==6);
 GLenum prop=GL_BUFFER_BINDING;gl.GetProgramResourceiv(replacement,GL_SHADER_STORAGE_BLOCK,gl.GetProgramResourceIndex(replacement,GL_SHADER_STORAGE_BLOCK,"Storage"),1,&prop,1,nullptr,&value);assert(value==7);
 }
 GLint current;gl.GetIntegerv(GL_CURRENT_PROGRAM,&current);assert(GLuint(current)==original);
 gl.Uniform1i(gl.GetUniformLocation(original,"mode"),11);
 {program_replacement_scope scope(gl,state);GLint value;gl.GetUniformiv(replacement,gl.GetUniformLocation(replacement,"mode"),&value);assert(value==11);}
 state->alive=false;{program_replacement_scope scope(gl,state);gl.GetIntegerv(GL_CURRENT_PROGRAM,&current);assert(GLuint(current)==original);}
 auto mismatch=program("#version 430\nuniform float color;out vec4 result;void main(){result=vec4(color);}",original);
 program_replacement_state bad;bad.original=original;bad.replacement=mismatch;assert(!build_program_state(gl,&bad));
 // Draw the same original, replacement, and restored original without rebinding.
 auto draw_original=program("#version 430\nuniform vec4 color;out vec4 result;void main(){result=color;}");
 auto draw_replacement=program("#version 430\nuniform vec4 color;out vec4 result;void main(){result=color*0.5;}",draw_original);
 auto draw_state=std::make_shared<program_replacement_state>();draw_state->original=draw_original;draw_state->replacement=draw_replacement;assert(build_program_state(gl,draw_state.get()));
 GLuint vao;gl.GenVertexArrays(1,&vao);gl.BindVertexArray(vao);gl.VertexAttrib4f(5,0,0,0,0);gl.Viewport(0,0,1,1);
 gl.UseProgram(draw_original);gl.Uniform4f(gl.GetUniformLocation(draw_original,"color"),1,0,0,1);
 auto draw_red=[&](){gl.DrawArrays(GL_TRIANGLES,0,3);GLfloat pixel[4];gl.ReadPixels(0,0,1,1,GL_RGBA,GL_FLOAT,pixel);return pixel[0];};
 assert(draw_red()>0.95f);
 {program_replacement_scope scope(gl,draw_state);auto red=draw_red();assert(red>0.45f&&red<0.55f);}
 assert(draw_red()>0.95f);
 gl.UseProgram(0);gl.DeleteProgram(draw_original);gl.DeleteProgram(draw_replacement);gl.DeleteVertexArrays(1,&vao);
 GLenum error=gl.GetError();if(error)std::printf("GL error: 0x%x\n",error);assert(error==GL_NO_ERROR);
 gl.UseProgram(0);gl.DeleteProgram(original);gl.DeleteProgram(replacement);gl.DeleteProgram(mismatch);
 eglMakeCurrent(display,nullptr,nullptr,nullptr);eglDestroyContext(display,context);eglDestroySurface(display,surface);eglTerminate(display);
 std::puts("OpenGL state integration tests passed");
}
