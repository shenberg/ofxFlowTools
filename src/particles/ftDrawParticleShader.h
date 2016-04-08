
#pragma once

#include "ofMain.h"
#include "ftShader.h"


namespace flowTools {
	
	class ftDrawParticleShader : public ftShader {
	public:
		ftDrawParticleShader() {
			bInitialized = 1;
			
			if (ofIsGLProgrammableRenderer())
				glThree();
			else
				glTwo();
			
			if (bInitialized)
				ofLogNotice("ftDrawParticleShader initialized");
			else
				ofLogWarning("ftDrawParticleShader failed to initialize");
		}
		
	protected:
		void glTwo() {
			vertexShader = GLSL120(
								   uniform sampler2DRect positionTexture;
								   uniform sampler2DRect ALMSTexture;
								   uniform float TwinkleSpeed;
								   uniform sampler2D HueToRGB;
								   
								   void main(){
									   
									   vec2 st = gl_Vertex.xy;
									   
									   vec3 texPosAndHue = texture2DRect(positionTexture, st).xyz;
									   vec2 texPos = texPosAndHue.xy;
									   gl_Position = gl_ModelViewProjectionMatrix * vec4(texPos, 0.0, 1.0);
									   vec4 alms = texture2DRect(ALMSTexture, st);
									   float age = alms.x;
									   float life = alms.y;
									   float mass = alms.z;
									   float size = alms.w;
									   gl_PointSize = size;

									   vec3 rgb = texture2D(HueToRGB, vec2(texPosAndHue.z, 0)).xyz;
									   
									   float alpha = min (0.5 - (age / life) * 0.5,age * 5.);
									   alpha *= 0.5 + (cos((age + size) * TwinkleSpeed * mass) + 1.0) * 0.5;
									   alpha = max(alpha, 0.0);
									   
									   gl_FrontColor = vec4(rgb, alpha);
									   
								   }
								   );
			
			
			bInitialized *= shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
			bInitialized *= shader.linkProgram();

			
		}
		
		void glThree() {
			vertexShader = GLSL150(
								   uniform	mat4 modelViewProjectionMatrix;
								   uniform	mat4 textureMatrix;
								   uniform	sampler2DRect PositionTexture;
								   uniform	sampler2DRect ALMSTexture;
								   uniform  sampler2D HueToRGB;
								   
								   in vec4	position;
								   in vec2	texcoord;
								   in vec4	color;
								   
								   uniform float TwinkleSpeed;
								   
								   out vec2 texCoordVarying;
								   out vec4 colorVarying;
								   
								   void main()
								   {
									   colorVarying = color;
									   texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
									   
									   
									   vec2 st = position.xy;
									   vec3 texPosAndHue = texture(PositionTexture, st).xyz;
									   vec2 texPos = texPosAndHue.xy;
									   gl_Position = modelViewProjectionMatrix * vec4(texPos, 0.0, 1.0);
									   
									   vec4 alms = texture(ALMSTexture, st);
									   float age = alms.x;
									   float life = alms.y;
									   float mass = alms.z;
									   float size = alms.w;
									   gl_PointSize = size;

									   vec3 rgb = texture(HueToRGB, vec2(texPosAndHue.z, 0)).xyz;
									   
									   float alpha = min (0.5 - (age / life) * 0.5,age * 5.);
									   alpha *= 0.5 + (cos((age + size) * TwinkleSpeed * mass) + 1.0) * 0.5;
									   alpha = max(alpha, 0.0);
									   
									   colorVarying = vec4(rgb, alpha);
								   }
								);
			
			
			
			// thanx to: http://mmmovania.blogspot.nl/2010/12/circular-point-sprites-in-opengl-33.html
			
			fragmentShader = GLSL150(
								  in vec4 colorVarying;
								  out vec4 fragColor;
								  
								  void main()
								  {
									  vec2 p = gl_PointCoord * 2.0 - vec2(1.0);
									  float d = dot(p,p);
									  float r = sqrt(d);
									  
									  if(d > r)
										  discard;
									  else
										  fragColor = colorVarying * (1.0, 1.0, 1.0, 1.0 - pow(r, 2.5));
								  }
								  );
			
			bInitialized *= shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
			bInitialized *= shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
			bInitialized *= shader.bindDefaults();
			bInitialized *= shader.linkProgram();
		}
		
	public:
		
		void update(ofVboMesh &particleVbo, int _numParticles, ofTexture& _positionTexture, ofTexture& _ALMSTexture, float _twinkleSpeed, ofTexture& _hueLookup){
			shader.begin();
			shader.setUniformTexture("PositionTexture", _positionTexture, 0);
			shader.setUniformTexture("ALMSTexture", _ALMSTexture, 1);
			shader.setUniformTexture("HueToRGB", _hueLookup, 2);
			shader.setUniform1f("TwinkleSpeed", _twinkleSpeed);
			
			bool dinges = true;
			//glEnable(GL_POINT_SMOOTH);
			glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			
			particleVbo.draw();
			
			glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			//glDisable(GL_POINT_SMOOTH);
			shader.end();
			
		}
	};
}