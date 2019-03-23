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
					return { pre.y - now.y    ,pre.x - now.x };
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
			dr(0.0),
			trans(Math::mat3<double>::id()),
			depth(_data.depth)
		{
		}
		void init(OpenGL::FrameScale const& _size)
		{
			persp.init(_size);
			bufferData.trans.z0 = float(_size.h) / (2.0 * tan(Math::Pi * persp.fovy / 180.0));
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
			dxyz.data[2] = scroll.operate();
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
				bufferData.trans.z0 = persp.y / (2.0 * tan(Math::Pi * persp.fovy / 180.0));
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
			vec4 r;
			vec4 t;
			vec3 g;
			float n;
		};
		struct Sphere
		{
			vec4 sphere;
			Color color;
		};
		struct Circle
		{
			vec4 plane;
			vec4 sphere;
			Color color;
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

			PlaneData planes;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool numChanged;
			bool upToDate;
			bool updated;
			Planes(Info const& _info)
				:
				buffer(&planes),
				config(&buffer, _info.type, _info.index),
				numChanged(true),
				updated(true)
			{
			}
			void dataInit()
			{
				if (numChanged)
				{

				}
				if (updated)
				{
					updated = false;
					if (numChanged)
					{
						config.dataInit();
						numChanged = false;
					}
					else
					{
						config.refreshData();
					}
				}
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
					return nullptr;
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
					OpenGL::Buffer::Data(DynamicDraw)
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
			bool updated;
			bool numChanged;
			bool recalculate;
			Triangles(Info const& _info)
				:
				trianglesOriginBuffer(&trianglesOrigin),
				trianglesGPUBuffer(&trianglesGPU),
				trianglesOriginConfig(&trianglesOriginBuffer, OpenGL::ShaderStorageBuffer, _info.indexOrigin),
				trianglesGPUConfig(&trianglesGPUBuffer, OpenGL::ShaderStorageBuffer, _info.indexGPU),
				updated(true),
				numChanged(false),
				recalculate(false)
			{
			}
			void dataInit()
			{
				if (updated)
				{
					trianglesOriginConfig.dataInit();
					if (trianglesGPU.num != trianglesOrigin.trianglesOrigin.length)
					{
						trianglesGPU.num = trianglesOrigin.trianglesOrigin.length;
						numChanged = true;
						trianglesGPUConfig.dataInit();
					}
					updated = false;
					recalculate = true;
				}
			}
		};
		struct GeometryNum
		{
			struct Data :OpenGL::Buffer::Data
			{
				struct Num
				{
					unsigned int planeNum;
					unsigned int triangleNum;
					unsigned int sphereNum;
					unsigned int circleNum;
					Num()
						:
						planeNum(0),
						triangleNum(0),
						sphereNum(0),
						circleNum(0)
					{
					}
				};
				Num num;
				Data()
					:
					OpenGL::Buffer::Data(StaticDraw),
					num()
				{

				}
				Data(Num const& _num)
					:
					OpenGL::Buffer::Data(StaticDraw),
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
			Data data;
			OpenGL::Buffer buffer;
			OpenGL::BufferConfig config;
			bool updated;
			GeometryNum(Info const& _info)
				:
				buffer(&data),
				config(&buffer, OpenGL::UniformBuffer, _info.index),
				updated(true)
			{
			}
			void dataInit()
			{
				if (updated)
				{
					config.dataInit();
					updated = false;
				}
			}
		};

		struct Info
		{
			Planes::Info planesInfo;
			Triangles::Info trianglesInfo;
			GeometryNum::Info geometryNumInfo;
		};


		Planes planes;
		Triangles triangles;
		GeometryNum geometryNum;


		Model(Info const& _info)
			:
			planes(_info.planesInfo),
			triangles(_info.trianglesInfo),
			geometryNum(_info.geometryNumInfo)
		{
		}
		void dataInit()
		{
			bool updated(false);
			if (planes.updated)
				planes.dataInit();
			triangles.dataInit();
			geometryNum.dataInit();
		}
		void refresh()
		{
			if (geometryNum.data.num.planeNum != planes.planes.planes.length)
			{
				geometryNum.data.num.planeNum = planes.planes.planes.length;
				geometryNum.updated = true;
			}
			if (geometryNum.data.num.triangleNum != triangles.trianglesOrigin.trianglesOrigin.length)
			{
				geometryNum.data.num.planeNum = planes.planes.planes.length;
				geometryNum.updated = true;
			}
		}
	};
}
