
#pragma once

#include "ofMain.h"
#include "ftShader.h"


namespace flowTools {
	
	class ftDrawParticleStretchedShader : public ftShader {
	public:
		ftDrawParticleStretchedShader() {
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
			string geometryShader;
			vertexShader = GLSL150(
								   uniform	mat4 modelViewProjectionMatrix;
								   uniform	mat4 textureMatrix;
								   uniform	sampler2DRect PositionTexture;
								   uniform	sampler2DRect ALMSTexture;
								   uniform  sampler2D HueToRGB;
								   
								   in vec4	position;
								   in vec4	color;
								   
								   uniform float TwinkleSpeed;
								   
								   void main()
								   {
									   gl_Position = position;
								   }
								);
			
			
			
			// thanx to: http://mmmovania.blogspot.nl/2010/12/circular-point-sprites-in-opengl-33.html
			
			fragmentShader = GLSL150(
                                  uniform sampler2D IntensityMap;
								  in vec4 colorVarying;
                                  in vec2 texCoordVarying;
								  out vec4 fragColor;
								  
								  void main()
								  {
									  /*
									  vec2 p = gl_PointCoord * 2.0 - vec2(1.0);
									  float d = dot(p,p);
									  float r = sqrt(d);
									  
									  if(d > r)
										  discard;
									  else
										  fragColor = colorVarying * (1.0, 1.0, 1.0, 1.0 - pow(r, 2.5));
										  */
									  fragColor = colorVarying * vec4(1,1,1,texture(IntensityMap, texCoordVarying.st).x);
								  }
								  );
			
			geometryShader = GLSL150(
				uniform mat4 modelViewProjectionMatrix;
			uniform	sampler2DRect PositionTexture;
			uniform	sampler2DRect ALMSTexture;
            uniform sampler2D HueToRGB;
			uniform sampler2DRect VelocityTexture;
            uniform float TwinkleSpeed;
            uniform float StretchScale;
            

            out vec4 colorVarying;
            out vec2 texCoordVarying;

			layout(points) in;
			layout(triangle_strip) out;
			layout(max_vertices = 4) out;

			void main() {
				vec2 index = gl_in[0].gl_Position.xy;
                // TODO: move this back to VS
				vec3 centerAndHue = texture(PositionTexture, index).xyz;
                vec2 center = centerAndHue.xy;
				vec2 velocity = texture(VelocityTexture, index * 0.25).xy;
                vec4 alms = texture(ALMSTexture, index);
                float age = alms.x;
                float life = alms.y;
                float mass = alms.z;
                float size = alms.w;
                
                float speed = length(velocity);
                vec2 dirX;
                if (speed != 0) {
                    dirX = velocity / speed;
                } else {
                    dirX = vec2(1,0);
                }
                // ortho vector to dirX
                vec2 dirY = vec2(dirX.y, -dirX.x);
                
                
                
                vec2 dx = dirX * size;
                vec2 dy = dirY * size;

                vec3 rgb = texture(HueToRGB, vec2(centerAndHue.z, 0)).rgb;
                
                float alpha = min (0.5 - (age / life) * 0.5,age * 5.);
                // TODO: differently. putting this back in kills performance
                //alpha *= 0.5 + (cos((age + size) * TwinkleSpeed * mass) + 1.0) * 0.5;
                alpha = max(alpha, 0.0);

                colorVarying = vec4(rgb,alpha);
                float scale = 0.5 + (StretchScale * speed);
                vec4 hackCenter = modelViewProjectionMatrix * vec4(center,0,1);
                // no translation for direction vectors (set w to 0)
                vec4 hackDx = modelViewProjectionMatrix * vec4(dx,0,0) * scale;
                vec4 hackDy = modelViewProjectionMatrix * vec4(dy,0,0) * 0.5;
                
                texCoordVarying = vec2(1,0);
				//gl_Position = modelViewProjectionMatrix * vec4(center + dx * scale - dy, 0, 1);
                gl_Position = hackCenter + hackDx - hackDy;
				EmitVertex();

                texCoordVarying = vec2(1,1);
				//gl_Position = modelViewProjectionMatrix * vec4(center + dx * scale + dy, 0, 1);
                gl_Position = hackCenter + hackDx + hackDy;
				EmitVertex();
/*
                texCoordVarying = vec2(0.5,0);
                gl_Position = modelViewProjectionMatrix * vec4(center + dx * speed - dy, 0, 1);
                EmitVertex();
                
                texCoordVarying = vec2(0.5,1);
                gl_Position = modelViewProjectionMatrix * vec4(center + dx * speed + dy, 0, 1);
                EmitVertex();

                texCoordVarying = vec2(0.5,0);
                gl_Position = modelViewProjectionMatrix * vec4(center - dx * speed - dy, 0, 1);
                EmitVertex();
                
                texCoordVarying = vec2(0.5,1);
                gl_Position = modelViewProjectionMatrix * vec4(center - dx * speed + dy, 0, 1);
                EmitVertex();
*/
                texCoordVarying = vec2(0,0);
				//gl_Position = modelViewProjectionMatrix * vec4(center - dx * scale - dy, 0, 1);
                gl_Position = hackCenter - hackDx - hackDy;
                EmitVertex();

                texCoordVarying = vec2(0,1);
				//gl_Position = modelViewProjectionMatrix * vec4(center - dx * scale + dy, 0, 1);
                gl_Position = hackCenter - hackDx + hackDy;
                EmitVertex();

				EndPrimitive();

			}
			);

			bInitialized *= shader.setupShaderFromSource(GL_VERTEX_SHADER, vertexShader);
			bInitialized *= shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragmentShader);
			bInitialized *= shader.setupShaderFromSource(GL_GEOMETRY_SHADER_EXT, geometryShader);
			bInitialized *= shader.bindDefaults();
			bInitialized *= shader.linkProgram();
		}
		
	public:
		
		void update(ofVboMesh &particleVbo, int _numParticles, ofTexture& _positionTexture, ofTexture& _ALMSTexture, float _twinkleSpeed, ofTexture& _hueLookup, ofTexture& _velocityTexture, ofTexture& _intensityMap, float _stretchScale){
			shader.begin();
			shader.setUniformTexture("PositionTexture", _positionTexture, 0);
			shader.setUniformTexture("ALMSTexture", _ALMSTexture, 1);
			shader.setUniformTexture("HueToRGB", _hueLookup, 2);
			shader.setUniformTexture("VelocityTexture", _velocityTexture, 3);
            shader.setUniformTexture("IntensityMap", _intensityMap, 4);
			shader.setUniform1f("TwinkleSpeed", _twinkleSpeed);
            shader.setUniform1f("StretchScale", _stretchScale);
			
			bool dinges = true;
			//glEnable(GL_POINT_SMOOTH);
			//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
			
			particleVbo.draw();
			
			//glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
			//glDisable(GL_POINT_SMOOTH);
			shader.end();
			
		}
	};
}