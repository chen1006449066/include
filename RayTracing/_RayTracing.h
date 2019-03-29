#pragma once
#include <_Vector.h>
#include <_Math.h>
#include <_File.h>
#include <GL/_OpenGL.h>

namespace RayTracing
{
	struct View :OpenGL::Buffer::Data
	{
		Math::mat<float, 4, 2>vertices;
		View()
			:
			Data(StaticDraw),
			vertices
			({ {-1.0f,-1.0f},
				{1.0f,-1.0f},
				{1.0f,1.0f},
				{-1.0f,1.0f} })
		{
		}
		virtual void* pointer()override
		{
			return vertices.array;
		}
		virtual unsigned int size()override
		{
			return sizeof(vertices);
		}
	};
	struct FrameScale :OpenGL::Buffer::Data
	{
		Math::vec2<unsigned int>scale;
		FrameScale(Math::vec2<unsigned int>const& _scale)
			:
			Data(StaticDraw),
			scale(_scale)
		{
		}
		virtual void* pointer()override
		{
			return scale.data;
		}
		virtual unsigned int size()override
		{
			return sizeof(scale);
		}
	};
	struct FrameData :OpenGL::Buffer::Data
	{
		FrameScale* frameSize;
		FrameData() = delete;
		FrameData(FrameScale* _frameSize)
			:
			Data(DynamicDraw),
			frameSize(_frameSize)
		{
		}
		virtual void* pointer()override
		{
			return nullptr;
		}
		virtual unsigned int size()override
		{
			return sizeof(Math::vec4<float>)*
				frameSize->scale.data[0] *
				frameSize->scale.data[1];
		}
	};
	struct Transform
	{
		struct Data
		{
			struct Perspective
			{
				double fovy;
				unsigned int y;
			};
			struct Scroll
			{
				double increaseDelta;
				double decreaseRatio;
				double threshold;
			};
			struct Key
			{
				double ratio;
			};

			Perspective persp;
			Scroll scroll;
			Key key;
			Math::vec3<double> initialPosition;
			double depth;
		};
		struct Perspective
		{
			double fovy;
			unsigned int y;
			bool updated;
			Perspective()
				:
				fovy(Math::Pi* 100.0 / 180.0),
				y(1024),
				updated(false)
			{
			}
			Perspective(Data::Perspective const& _persp)
				:
				fovy(_persp.fovy),
				y(_persp.y),
				updated(false)
			{
			}
			Perspective(Perspective const&) = default;
			void init(OpenGL::FrameScale const& _size)
			{
				y = _size.h;
				updated = false;
			}
			void refresh(int _w, int _h)
			{
				y = _h;
				updated = true;
			}
		};
		struct Scroll
		{
			double increaseDelta;
			double decreaseRatio;
			double threshold;
			double total;
			Scroll()
				:
				increaseDelta(0.05),
				decreaseRatio(0.95),
				threshold(0.01),
				total(threshold)
			{
			}
			Scroll(Data::Scroll const& _scroll)
				:
				increaseDelta(_scroll.increaseDelta),
				decreaseRatio(_scroll.decreaseRatio),
				threshold(_scroll.threshold),
				total(threshold)
			{
			}
			void refresh(double _d)
			{
				total += _d * increaseDelta;
			}
			double operate()
			{
				if (abs(total) > threshold)
				{
					total *= decreaseRatio;
					return total;
				}
				else return 0.0;
			}
		};
		struct Key
		{
			bool left;
			bool right;
			bool up;
			bool down;
			double ratio;
			Key()
				:
				left(false),
				right(false),
				up(false),
				down(false),
				ratio(0.05)
			{
			}
			Key(Data::Key const& _key)
				:
				left(false),
				right(false),
				up(false),
				down(false),
				ratio(_key.ratio)
			{
			}
			void refresh(int _key, bool _operation)
			{
				switch (_key)
				{
				case 0:left = _operation; break;
				case 1:right = _operation; break;
				case 2:up = _operation; break;
				case 3:down = _operation; break;
				}
			}
			Math::vec2<double> operate()
			{
				Math::vec2<double>t
				{
					ratio * ((int)right - (int)left),
					ratio * ((int)up - (int)down)
				};
				return t;
			}
		};
		struct Mouse
		{
			struct Pointer
			{
				double x;
				double y;
				bool valid;
				Pointer()
					:
					valid(false)
				{
				}
			};
			Pointer now;
			Pointer pre;
			bool left;
			bool middle;
			bool right;
			Mouse()
				:
				now(),
				pre(),
				left(false),
				middle(false),
				right(false)
			{
			}
			void refreshPos(double _x, double _y)
			{
				if (left)
				{
					if (now.valid)
					{
						pre = now;
						now.x = _x;
						now.y = _y;
					}
					else
					{
						now.valid = true;
						now.x = _x;
						now.y = _y;
					}
				}
				else
				{
					now.valid = false;
					pre.valid = false;
				}
			}
			void refreshButton(int _button, bool _operation)
			{
				switch (_button)
				{
				case 0:	left = _operation; break;
				case 1:	middle = _operation; break;
				case 2:	right = _operation; break;
				}

			}
			Math::vec2<double> operate()
			{
				if (now.valid && pre.valid)
				{
					pre.valid = false;
					return { now.y - pre.y   ,now.x - pre.x };
				}
				else return { 0.0,0.0 };
			}
		};
		struct BufferData :OpenGL::Buffer::Data
		{
			struct Trans
			{
				Math::mat<float, 3, 4>ans;
				Math::vec3<float>r0;
				float z0;
			};
			Trans trans;
			BufferData()
				:Data(DynamicDraw)
			{
			}
			virtual void* pointer()override
			{
				return &trans;
			}
			virtual unsigned int size()override
			{
				return sizeof(Trans);
			}
		};


		Perspective persp;
		Scroll scroll;
		Key key;
		Mouse mouse;
		BufferData bufferData;
		Math::vec3<double>dr;
		Math::mat3<double>trans;
		double depth;
		bool updated;

		Transform()
			:
			persp(),
			scroll(),
			mouse(),
			updated(false),
			bufferData(),
			dr(0.0),
			trans(Math::mat3<double>::id()),
			depth(500.0)
		{
		}
		Transform(Data const& _data)
			:
			persp(_data.persp),
			scroll(_data.scroll),
			key(_data.key),
			mouse(),
			updated(false),
			bufferData(),
			dr(_data.initialPosition),
			trans(Math::mat3<double>::id()),
			depth(_data.depth)
		{
		}
		void init(OpenGL::FrameScale const& _size)
		{
			persp.init(_size);
			bufferData.trans.z0 = -float(_size.h) / (2.0 * tan(Math::Pi * persp.fovy / 180.0));
			calcAns();
			updated = true;
		}
		void resize(int _w, int _h)
		{
			persp.refresh(_w, _h);
		}
		void calcAns()
		{
			bufferData.trans.ans = trans;
			bufferData.trans.r0 = dr;
		}
		void operate()
		{
			Math::vec3<double>dxyz(key.operate());
			dxyz.data[2] = -scroll.operate();
			Math::vec2<double>axis(mouse.operate());
			bool operated(false);
			if (dxyz != 0.0)
			{
				dr += (trans, dxyz);
				operated = true;
			}
			if (axis != 0.0)
			{
				double l(axis.length() / depth);
				trans = ((trans, Math::vec3<double>(axis)).rotMat(l), trans);
				operated = true;
			}
			if (persp.updated)
			{
				bufferData.trans.z0 = -(persp.y / (2.0 * tan(Math::Pi * persp.fovy / 180.0)));
				persp.updated = false;
				operated = true;
			}
			if (operated)
			{
				calcAns();
				updated = true;
			}
		}
	};

	struct Model
	{
		//Notice:	Something small can be placed in uniform buffer;
		//			but something much bigger(more than 64KB for example)
		//			must be placed in shader storage buffer...
		using vec4 = Math::vec4<float>;
		using vec3 = Math::vec3<float>;
		using mat34 = Math::mat<float, 3, 4>;

		struct Color
		{
			vec3 r;
			int texR;
			vec3 t;
			int texT;
			vec3 d;
			int texD;
			vec3 g;
			int texG;
			vec3 blank;
			float n;
		};

		struct Planes
		{
			struct PlaneData :OpenGL::Buffer::Data
			{
				struct Plane
				{
					vec4 paras;	//Ax + By + Cz + W = 0, this is (A, B, C, W).
					Color color;
				};
				Vector<Plane>planes;
				PlaneData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()
				{
					return planes.data;
				}
				virtual unsigned int size()
				{
					return sizeof(Plane)* planes.length;
				}
			};
			struct Info
			{
				OpenGL::BufferType type;
				unsigned int index;
			};

			PlaneData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			Planes(Info const& _info)
				:
				buffer(&data),
				config(&buffer, _info.type, _info.index),
				numChanged(false),
				upToDate(true)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
				}
				else if (!upToDate)
				{
					config.refreshData();
				}
				upToDate = true;
			}
		};
		struct Triangles
		{
			struct TriangleOriginData :OpenGL::Buffer::Data
			{
				struct TriangleOrigin
				{
					mat34 vertices;
					Color color;
				};
				Vector<TriangleOrigin>trianglesOrigin;
				TriangleOriginData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()
				{
					return trianglesOrigin.data;
				}
				virtual unsigned int size()
				{
					return sizeof(TriangleOrigin)* trianglesOrigin.length;
				}
			};
			struct TriangleGPUData :OpenGL::Buffer::Data
			{
				struct TriangleGPU
				{
					vec4 plane;
					vec4 p1;
					vec4 k1;
					vec4 k2;
					Color color;
				};
				unsigned int num;
				TriangleGPUData()
					:
					OpenGL::Buffer::Data(DynamicDraw),
					num(0)
				{
				}
				virtual void* pointer()
				{
					return nullptr;
				}
				virtual unsigned int size()
				{
					return sizeof(TriangleGPU)* num;
				}
			};
			struct Info
			{
				unsigned int indexOrigin;
				unsigned int indexGPU;
			};
			TriangleOriginData trianglesOrigin;
			TriangleGPUData trianglesGPU;
			OpenGL::Buffer trianglesOriginBuffer;
			OpenGL::Buffer trianglesGPUBuffer;
			OpenGL::BufferConfig trianglesOriginConfig;
			OpenGL::BufferConfig trianglesGPUConfig;
			bool numChanged;
			bool originUpToDate;
			bool GPUUpToDate;
			Triangles(Info const& _info)
				:
				trianglesOriginBuffer(&trianglesOrigin),
				trianglesGPUBuffer(&trianglesGPU),
				trianglesOriginConfig(&trianglesOriginBuffer, OpenGL::ShaderStorageBuffer, _info.indexOrigin),
				trianglesGPUConfig(&trianglesGPUBuffer, OpenGL::ShaderStorageBuffer, _info.indexGPU),
				numChanged(true),
				originUpToDate(false),
				GPUUpToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					trianglesOriginConfig.dataInit();
					trianglesGPU.num = trianglesOrigin.trianglesOrigin.length;
					trianglesGPUConfig.dataInit();
					GPUUpToDate = false;
				}
				else if (!originUpToDate)
				{
					trianglesOriginConfig.refreshData();
					GPUUpToDate = false;
				}
				if (!trianglesOrigin.trianglesOrigin.length)
					GPUUpToDate = false;
				originUpToDate = true;
			}
		};
		struct Spheres
		{
			struct SphereData :OpenGL::Buffer::Data
			{
				struct Sphere
				{
					vec4 sphere;
					vec4 e1;
					vec4 e2;
					Color color;
				};
				Vector<Sphere>spheres;
				SphereData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()override
				{
					return spheres.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Sphere)* spheres.length;
				}
			};
			struct Info
			{
				unsigned int index;
			};

			SphereData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			Spheres(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::ShaderStorageBuffer, _info.index),
				numChanged(true),
				upToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
				}
				else if (!upToDate)
				{
					config.refreshData();
				}
				upToDate = true;
			}
		};
		struct Circles
		{
			struct CircleData :OpenGL::Buffer::Data
			{
				struct Circle
				{
					vec4 plane;		//n(unnormalized)
					vec3 sphere;	//p, R^2
					float r2;
					vec4 e1;			//e(unnormalized)
					Color color;
				};
				Vector<Circle>circles;
				CircleData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()override
				{
					return circles.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Circle)* circles.length;
				}
			};
			struct Info
			{
				unsigned int index;
			};
			CircleData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			bool GPUUpToDate;
			Circles(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::ShaderStorageBuffer, _info.index),
				numChanged(true),
				upToDate(false),
				GPUUpToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
					GPUUpToDate = false;
				}
				else if (!upToDate)
				{
					config.refreshData();
					GPUUpToDate = false;
				}
				upToDate = true;
			}
		};
		struct Cylinders
		{
			struct CylinderData :OpenGL::Buffer::Data
			{
				struct Cylinder
				{
					vec3 c;
					float r2;
					vec3 n;
					float l;
					vec4 e1;
					Color color;
				};
				Vector<Cylinder> cylinders;
				CylinderData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()override
				{
					return cylinders.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Cylinder)* cylinders.length;
				}
			};
			struct Info
			{
				unsigned int index;
			};
			CylinderData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			bool GPUUpToDate;
			Cylinders(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::ShaderStorageBuffer, _info.index),
				numChanged(true),
				upToDate(false),
				GPUUpToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
					GPUUpToDate = false;
				}
				else if (!upToDate)
				{
					config.refreshData();
					GPUUpToDate = false;
				}
				upToDate = true;
			}
		};
		struct Cones
		{
			struct ConeData :OpenGL::Buffer::Data
			{
				struct Cone
				{
					vec3 c;
					float c2;
					vec3 n;
					float l2;
					vec4 e1;
					Color color;
				};
				Vector<Cone>cones;
				ConeData()
					:
					Data(DynamicDraw)
				{
				}
				virtual void* pointer()override
				{
					return cones.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(Cone)* cones.length;
				}
			};
			struct Info
			{
				unsigned int index;
			};
			ConeData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			bool GPUUpToDate;
			Cones(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::ShaderStorageBuffer, _info.index),
				numChanged(true),
				upToDate(false),
				GPUUpToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
					GPUUpToDate = false;
				}
				else if (!upToDate)
				{
					config.refreshData();
					GPUUpToDate = false;
				}
				upToDate = true;
			}
		};
		struct PointLights
		{
			struct PointLightData :OpenGL::Buffer::Data
			{
				struct PointLight
				{
					vec4 color;
					vec4 p;
				};
				Vector<PointLight>pointLights;
				PointLightData()
					:
					Data(StaticDraw)
				{
				}
				virtual void* pointer()override
				{
					return pointLights.data;
				}
				virtual unsigned int size()override
				{
					return sizeof(PointLight)* pointLights.length;
				}
			};
			struct Info
			{
				unsigned int index;
			};
			PointLightData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			bool GPUUpToDate;
			PointLights(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::ShaderStorageBuffer, _info.index),
				numChanged(true),
				upToDate(false),
				GPUUpToDate(false)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{
					config.dataInit();
					GPUUpToDate = false;
				}
				else if (!upToDate)
				{
					config.refreshData();
					GPUUpToDate = false;
				}
				upToDate = true;
			}
		};
		struct GeometryNum
		{
			struct NumData :OpenGL::Buffer::Data
			{
				struct Num
				{
					unsigned int planeNum;
					unsigned int triangleNum;
					unsigned int sphereNum;
					unsigned int circleNum;
					unsigned int cylinderNum;
					unsigned int coneNum;
					unsigned int pointLightNum;
					unsigned int blank[1];
					Num()
						:
						planeNum(0),
						triangleNum(0),
						sphereNum(0),
						circleNum(0),
						cylinderNum(0),
						pointLightNum(0)
					{
					}
				};
				Num num;
				NumData()
					:
					Data(StaticDraw),
					num()
				{
				}
				NumData(Num const& _num)
					:
					Data(StaticDraw),
					num(_num)
				{
				}
				virtual void* pointer()override
				{
					return &num;
				}
				virtual unsigned int size()override
				{
					return sizeof(Num);
				}
			};
			struct Info
			{
				unsigned int index;
			};
			NumData data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			GeometryNum(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::UniformBuffer, _info.index)
			{
			}
			void dataInit()
			{
				config.dataInit();
			}
		};

		struct Info
		{
			Planes::Info planesInfo;
			Triangles::Info trianglesInfo;
			Spheres::Info spheresInfo;
			Circles::Info circlesInfo;
			Cylinders::Info cylindersInfo;
			Cones::Info conesInfo;
			PointLights::Info pointLightsInfo;
			GeometryNum::Info geometryNumInfo;
		};


		Planes planes;
		Triangles triangles;
		Spheres spheres;
		Circles circles;
		Cylinders cylinders;
		Cones cones;
		PointLights pointLights;
		GeometryNum geometryNum;

		Model(Info const& _info)
			:
			planes(_info.planesInfo),
			triangles(_info.trianglesInfo),
			spheres(_info.spheresInfo),
			circles(_info.circlesInfo),
			cylinders(_info.cylindersInfo),
			cones(_info.conesInfo),
			pointLights(_info.pointLightsInfo),
			geometryNum(_info.geometryNumInfo)
		{
		}
		void dataInit()
		{
			bool numChanged(false);
			planes.dataInit();
			triangles.dataInit();
			spheres.dataInit();
			circles.dataInit();
			cylinders.dataInit();
			cones.dataInit();
			pointLights.dataInit();
			if (planes.numChanged)
			{
				geometryNum.data.num.planeNum = planes.data.planes.length;
				planes.numChanged = false;
				numChanged = true;
			}
			if (triangles.numChanged)
			{
				geometryNum.data.num.triangleNum = triangles.trianglesOrigin.trianglesOrigin.length;
				triangles.numChanged = false;
				numChanged = true;
			}
			if (spheres.numChanged)
			{
				geometryNum.data.num.sphereNum = spheres.data.spheres.length;
				spheres.numChanged = false;
				numChanged = true;
			}
			if (circles.numChanged)
			{
				geometryNum.data.num.circleNum = circles.data.circles.length;
				circles.numChanged = false;
				numChanged = true;
			}
			if (cylinders.numChanged)
			{
				geometryNum.data.num.cylinderNum = cylinders.data.cylinders.length;
				cylinders.numChanged = false;
				numChanged = true;
			}
			if (cones.numChanged)
			{
				geometryNum.data.num.coneNum = cones.data.cones.length;
				cones.numChanged = false;
				numChanged = true;
			}
			if (pointLights.numChanged)
			{
				geometryNum.data.num.pointLightNum = pointLights.data.pointLights.length;
				pointLights.numChanged = false;
				numChanged = true;
			}
			if (numChanged)
			{
				geometryNum.dataInit();
			}
		}
		void addCylinder(Cylinders::CylinderData::Cylinder const& _cylinder)
		{
			Circles::CircleData::Circle circle0;
			Circles::CircleData::Circle circle1;
			cylinders.data.cylinders.pushBack(_cylinder);
			circles.data.circles.pushBack
			(
				{
					-_cylinder.n,
					_cylinder.c,
					_cylinder.r2,
					_cylinder.e1,
					_cylinder.color
				}
			);
			circles.data.circles.pushBack
			(
				{
					_cylinder.n,
					_cylinder.c + _cylinder.n * _cylinder.l,
					_cylinder.r2,
					_cylinder.e1,
					_cylinder.color
				}
			);
			circles.numChanged = true;
			cylinders.numChanged = true;
		}
		void addCone(Cones::ConeData::Cone const& _cone)
		{
			circles.data.circles.pushBack
			(
				{
					_cone.n,
					_cone.c + _cone.n * sqrtf(_cone.l2 * _cone.c2),
					_cone.l2 * (1 - _cone.c2),
					_cone.e1,
					_cone.color
				}
			);
			cones.data.cones.pushBack(_cone);
			circles.numChanged = true;
			cones.numChanged = true;
		}
	};
}
