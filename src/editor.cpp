#include <iostream>
#include <vector>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "world.hyper"
#include "entity.hpp"

using namespace std;

class ShaderProgram {
public:
	const GLuint program;
	const GLuint vertex_shader;
	const GLuint fragment_shader;
	ShaderProgram(
		const char* vertex_shader_source,
		const char* fragment_shader_source
	) :
		program{ glCreateProgram() },
		vertex_shader{ glCreateShader(GL_VERTEX_SHADER) },
		fragment_shader{ glCreateShader(GL_FRAGMENT_SHADER) }
	{
		glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
		glCompileShader(vertex_shader);

		glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
		glCompileShader(fragment_shader);

		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);
	}
	explicit operator GLuint() const
	{
		return program;
	}
};

class ShaderData {
private:
public:
	struct Structure
	{
		float x, y;
		float r, g, b;
	};
	const GLint mvp_location;
	const GLint vpos_location;
	const GLint vcol_location;
	ShaderData(ShaderProgram program, vector<Structure> vertices) :
		mvp_location{ glGetUniformLocation((GLuint)program, "MVP") },
		vpos_location{ glGetAttribLocation((GLuint)program, "vPos") },
		vcol_location{ glGetAttribLocation((GLuint)program, "vCol") }
	{
		GLuint vertex_buffer;
		glGenBuffers(1, &vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Structure), &vertices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(vpos_location);
		glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
			sizeof(vertices[0]), (void*)nullptr);
		glEnableVertexAttribArray(vcol_location);
		glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
			sizeof(vertices[0]), (void*)(sizeof(float) * 2));
	}
};

// ShaderBuilder?

enum class GLSLUnit {
	single,
	vec2,
	vec3,
	vec4,
};
enum class GLSLUniformUnit {
	single,
	vec2,
	vec3,
	vec4,
	sampler2D,
};

using ShaderStream = ostringstream;


#define SHADER_MEMBER(n) \
	template<GLSLUnit T>\
	class member_##n {\
	public:\
		void to_vert_text(ShaderStream& stream);\
	};\
	template <> void member_##n<GLSLUnit::single>::to_vert_text (ShaderStream& stream) {\
		stream << "varying highp float " #n << endl;\
	}\
	template <> void member_##n<GLSLUnit::vec2>::to_vert_text (ShaderStream& stream) {\
		stream << "varying highp vec2 " #n << endl;\
	}\
	template <> void member_##n<GLSLUnit::vec3>::to_vert_text (ShaderStream& stream) {\
		stream << "varying highp vec3 " #n << endl;\
	}\
	template <> void member_##n<GLSLUnit::vec4>::to_vert_text (ShaderStream& stream) {\
		stream << "varying highp vec4 " #n << endl;\
	}
// end SHADER_MEMBER

SHADER_MEMBER(NOICE);

// Alternative shader member defs?


template<GLSLUnit T> void why_no_work(ShaderStream& stream, const string shader_name);
template <> void why_no_work<GLSLUnit::single>(ShaderStream& stream, const string shader_name) {
	stream << "varying highp float " << shader_name << endl;
}
template <> void why_no_work<GLSLUnit::vec2>(ShaderStream& stream, const string shader_name) {
	stream << "varying highp vec2 " << shader_name << endl;
}
template <> void why_no_work<GLSLUnit::vec3>(ShaderStream& stream, const string shader_name) {
	stream << "varying highp vec3 " << shader_name << endl;
}
template <> void why_no_work<GLSLUnit::vec4>(ShaderStream& stream, const string shader_name) {
	stream << "varying highp vec4 " << shader_name << endl;
}

template<class T, GLSLUnit U>
class ShaderMember {
public:
	void to_vert_text(ShaderStream& stream) {
		why_no_work<U>(stream, ((string)typeid(T).name()).substr(size("class")));
	}
};

template<class T>
class Typed
{
public:
	const T value;
	Typed(T val) : value { val } { }
};

class NamedMemberExample : public Typed<int> {};

const auto hey = NamedMemberExample { 0 };

class Positions : public ShaderMember<Positions, GLSLUnit::vec2> { };

// Shader defs?
template<typename T>
struct ShaderInput {
	T input_type;
};

template<typename T>
struct ShaderOutput {
	T output_type;
};


// Test!

int main()
{
	hyp::test();

	cout << "Hello Worldlings!" << endl;
	const auto thingie { tuple_cat(make_tuple(1), make_tuple(2)) };
	const auto thinger = tuple_cat(make_tuple(0), thingie);
	const auto thingerino = tupler(1, 2, 3);
	cout << get<1>(thingerino) << endl;

	const auto funcie = [](auto first, auto second) { return first + second; };
	// const auto resulter = std::apply(funcie, make_tuple(1, 2));
	// const auto unwrapped = std::apply(unwrap, make_tuple(Typed<int>{ 1 }));

	ostringstream streamer;
	member_NOICE<GLSLUnit::vec2>{}.to_vert_text(streamer);
	member_NOICE<GLSLUnit::vec2>{}.to_vert_text(streamer);
	auto membie = Positions{};
	membie.to_vert_text(streamer);

	cout << streamer.str() << endl;

	if (!glfwInit())
	{
		// Initialization failed
	}
	const auto window = glfwCreateWindow(640, 480, "HyperChill", nullptr, nullptr);
	if (!window)
	{
		// Window or OpenGL context creation failed
	}
	glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    });

	glfwSwapInterval(1);

	auto program = ShaderProgram{
		R"glsl(#version 110

			uniform mat4 MVP;
			attribute vec3 vCol;
			attribute vec2 vPos;
			varying vec3 color;

			void main()
			{
				gl_Position = MVP * vec4(vPos, 0.0, 1.0);
				color = vCol;
			}

		)glsl",

		R"glsl(#version 110

			varying vec3 color;

			void main()
			{
				gl_FragColor = vec4(color, 1.0);
			}

		)glsl",
	};

	auto data = ShaderData{
		program,
		{
			{ -0.6f, -0.4f, 1.f, 1.f, 0.f },
			{  0.6f, -0.4f, 0.f, 1.f, 1.f },
			{   0.f,  0.6f, 1.f, 0.f, 1.f }
		}
	};

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		mat4x4 m, p, mvp;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = (float)width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		mat4x4_identity(m);
		mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		mat4x4_mul(mvp, p, m);

		glUseProgram((GLuint)program);
		glUniformMatrix4fv(data.mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	cout << "Complete!" << endl;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

