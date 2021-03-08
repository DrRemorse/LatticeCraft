#include "atmosphere.hpp"

#include <library/config.hpp>
#include <library/opengl/opengl.hpp>
#include <library/opengl/vao.hpp>
#include "camera.hpp"
#include "sun.hpp"
#include "shaderman.hpp"
#include "textureman.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>

using namespace library;

namespace cppcraft
{
	class Scatterer
	{
	public:
		VAO vao;

		float innerRadius;
		float outerRadius;

		void init();
		void render(Camera& camera, int mode);
	};
	Scatterer scatter;

	const float PI = 4 * atan(1);

	void createSkyDome(float r_inner, float r_outer);

	// entry functions into this closed module
	void Atmosphere::init()
	{
		scatter.init();
	}
	void Atmosphere::render(Camera& camera, int mode)
	{
		scatter.render(camera, mode);
	}

	// scatterer implementation

	void Scatterer::init()
	{
		innerRadius = 10.0;
		outerRadius = 10.25;

		// create the skydome algorithmically
		createSkyDome(innerRadius, outerRadius);
	}

	void Scatterer::render(Camera& camera, int mode)
	{
		Shader& shd = shaderman[Shaderman::ATMOSPHERE];
		shd.bind();

		shd.sendVec3("v3LightPos", thesun.getRealtimeAngle());
		shd.sendFloat("sunAngle", thesun.getRealtimeRadianAngle());
		// 0.8 for reflections (at night), 0.4 for normal
		shd.sendFloat("starBrightness", (mode == 1) ? 0.8 : 0.4);

		// bind textures
		textureman.bind(1, Textureman::T_STARS);
		textureman.bind(0, Textureman::T_SKYBOX);

		// create view matrix
		glm::mat4 matview = camera.getRotationMatrix();
		matview *= glm::translate(glm::vec3(0.0f, -innerRadius, 0.0f));

		matview = camera.getProjection() * matview;
		shd.sendMatrix("matmvp", matview);
		shd.sendFloat("above", 1.0);

		// render
		scatter.vao.render(GL_TRIANGLES);

		matview = camera.getRotationMatrix();
		// multiply with negative-Y scaling matrix
		matview *= glm::scale(glm::vec3(1.0f, -1.0f, 1.0f));
		matview *= glm::translate(glm::vec3(0.0f, -innerRadius, 0.0f));
		matview = camera.getProjection() * matview;

		shd.sendMatrix("matmvp", matview);
		shd.sendFloat("above", 0.0);

		// render mirrored on y-axis
		scatter.vao.render(GL_TRIANGLES);
	}

	void createSkyDome(float r_inner, float r_outer)
	{
		struct domevertex_t
		{
			float x, y, z;

			domevertex_t() {}
			domevertex_t(float X, float Y, float Z) :
				x(X), y(Y), z(Z) {}
		};

		const int numX = 16;
		const int numY = 4;

		const int num_vertices = (2 * numY + 1) * numX * 3; // triangles * 3

		// total number of vertices allocated to buffer:
		domevertex_t* vertices = new domevertex_t[num_vertices];
		// vertices for triangle strip:
		domevertex_t va, vb, vc, vd;

		float stepa  = PI * 2.0f / numX;
		float startb = asin(r_inner / r_outer), stepb = (PI / 2.0f - startb) / 4.0f;

		domevertex_t* v = vertices;

		for (int y = 0; y < numY; y++)
		{
			float b = startb + stepb * y;

			for (int x = 0; x < numX; x++)
			{
				float a = stepa * x;

				va = domevertex_t( sinf(a)         * cosf(b) * r_outer,         sinf(b) * r_outer,         -cosf(a)         * cosf(b) * r_outer );
				vb = domevertex_t( sinf(a + stepa) * cosf(b) * r_outer,         sinf(b) * r_outer,         -cosf(a + stepa) * cosf(b) * r_outer );
				vc = domevertex_t( sinf(a + stepa) * cosf(b + stepb) * r_outer, sinf(b + stepb) * r_outer, -cosf(a + stepa) * cosf(b + stepb) * r_outer );
				vd = domevertex_t( sinf(a)         * cosf(b + stepb) * r_outer, sinf(b + stepb) * r_outer, -cosf(a)         * cosf(b + stepb) * r_outer );

				v[0] = va;
				v[1] = vb;
				v[2] = vc;

				v[3] = vc;
				v[4] = vd;
				v[5] = va;
				v += 6;
			}
		}

		float b = startb + stepb * 3;

		for (int x = 0; x < numX; x++)
		{
			float a = stepa * x;

			va = domevertex_t ( sinf(a)         * cosf(b) * r_outer, sinf(b) * r_outer, -cosf(a)         * cosf(b) * r_outer );
			vb = domevertex_t ( sinf(a + stepa) * cosf(b) * r_outer, sinf(b) * r_outer, -cosf(a + stepa) * cosf(b) * r_outer );
			vc = domevertex_t ( 0.0f, r_outer, 0.0f );

			v[0] = va;
			v[1] = vb;
			v[2] = vc;
			v += 3;
		}

		// upload to GPU
		scatter.vao.begin(sizeof(domevertex_t), num_vertices, vertices);
		scatter.vao.attrib(0, 3, GL_FLOAT, GL_FALSE, 0);
		//scatter.vao.indexes(indexes, indexCounter);
		scatter.vao.end();

		// cleanup
		delete[] vertices;
	}
}
