#include <iostream>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.inl>

#include "entity.hpp"

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
		
		GLint compileResult;
		glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compileResult);
		if(compileResult != GL_TRUE) {
            GLsizei logLength;
            GLchar  log[1024];
            glGetShaderInfoLog(vertex_shader, sizeof(log), &logLength, log);
            std::cerr << "vertex_shader: " << std::endl << log << std::endl;
		}

		glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
		glCompileShader(fragment_shader);
		
		glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compileResult);
		if(compileResult != GL_TRUE) {
            GLsizei logLength;
            GLchar  log[1024];
            glGetShaderInfoLog(fragment_shader, sizeof(log), &logLength, log);
            std::cerr << "fragment_shader: " << std::endl << log << std::endl;
		}

		glAttachShader(program, vertex_shader);
		glAttachShader(program, fragment_shader);
		glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &compileResult);
        if(compileResult != GL_TRUE)
        {
            std::cerr << "Shader compilation of file '" << "foof" << "' failed." << std::endl;

            GLsizei logLength;
            GLchar  log[1024];
            glGetProgramInfoLog(program, sizeof(log), &logLength, log);
            std::cerr << "program: " << std::endl << log << std::endl;
        }
	}
	explicit operator GLuint() const
	{
		return program;
	}
    template<class T>
    void assign_buffer_from_vector(const std::string& name, std::vector<T> input) {
        const GLint location = glGetAttribLocation((GLuint)program, (GLchar*)name.data());
        GLuint vertex_buffer;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, input.size() * sizeof(T), &input[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, std::tuple_size<T>::value, GL_FLOAT, GL_FALSE,
                              sizeof(input[0]), (void*)nullptr);
        std::cout << location << std::endl;
    }
};

template<class T>
class Uniform {
public:
    const GLint location;
    Uniform(ShaderProgram& program, std::string name) :
        location{glGetUniformLocation((GLuint)program, name.data()) }
        { }
    void assign(T value) const {
        static_assert(sizeof(T) == 0, "Only specializations of assign can be used");
    }
};
template<>
void Uniform<glm::mat4>::assign(glm::mat4 value) const {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
template<>
void Uniform<glm::vec3>::assign(glm::vec3 value) const {
    glUniform3fv(location, 1, glm::value_ptr(value));
}
template<>
void Uniform<glm::vec4>::assign(glm::vec4 value) const {
    glUniform4fv(location, 1, glm::value_ptr(value));
}

class FunShader : public ShaderProgram {
public:

    using Position = std::tuple<float, float>;
    using Color = std::tuple<float, float, float>;
    const Uniform<glm::mat4> model_view_projection;
    const Uniform<glm::vec4> extra_data;

    explicit FunShader(std::vector<Position> vertex_positions, std::vector<Color> vertex_colors) :
        ShaderProgram{
            R"glsl(#version 110

                uniform mat4 model_view_projection;
                uniform vec4 extra_data;
                attribute vec3 vert_color;
                attribute vec2 vert_position;
                varying vec3 frag_color;

                void main()
                {
                    gl_Position = model_view_projection * vec4(vert_position, 0.0, 1.0);
                    frag_color = vert_color * (sin(extra_data.x * 10.0) + 1.0);
                }

            )glsl",
            R"glsl(#version 110

                varying vec3 frag_color;

                void main()
                {
                    gl_FragColor = vec4(frag_color, 1.0);
                }

        )glsl",
    },
    model_view_projection{ *this, "model_view_projection" },
    extra_data{ *this, "extra_data" }
    {
        assign_buffer_from_vector("vert_color", std::move(vertex_colors));
        assign_buffer_from_vector("vert_position", std::move(vertex_positions));
    }
};

/** 😎 Objectives:
 * ✅ Take shader settings
 * ✅ Use to create shader
 * Use to create render command
 */
namespace ShaderBuilder {

    using namespace std;
    using namespace hyp;
    using namespace glm;

    struct Color : SimpleComponent<vec3>{};

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
        mat4,
        sampler2D,
    };

    template<GLSLUnit unit>
    class Varying {};

    template<GLSLUnit unit, bool instanced>
    class Attribute {};

    class Element {};

    template<GLSLUniformUnit unit, int count>
    class Uniform {

    };

    // ⚠ MSVC specific class name gathering solution
    template<class T>
    string get_class_name(T member) {
        string base_name = typeid(member).name();
        int last_space = std::max(
            base_name.find_last_of("::") + 1,
            base_name.find_last_of(" "));
        return base_name.substr(last_space);
    }

    template<class... T>
    auto get_class_names(T... classes) {
        return list{get_class_name(classes)...};
    }

    template<GLSLUnit unit, bool instanced>
    void vert_text(ostream& stream, Attribute<unit, instanced> member, string name) {
        stream << "attribute ";
        switch (unit) {
            case GLSLUniformUnit::single: stream << "single "; break;
            case GLSLUniformUnit::vec2: stream << "vec2 "; break;
            case GLSLUniformUnit::vec3: stream << "vec3 "; break;
            case GLSLUniformUnit::vec4: stream << "vec4 "; break;
            case GLSLUniformUnit::mat4: stream << "mat4 "; break;
            case GLSLUniformUnit::sampler2D: stream << "sampler2D "; break;
        }
        stream << name << ";" << endl;
    }

    template<GLSLUniformUnit unit, int count>
    void vert_text(ostream& stream, Uniform<unit, count> member, string name) {
        stream << "uniform ";
        switch (unit) {
            case GLSLUniformUnit::sampler2D: break;
            default: stream << "highp ";
        }
        switch (unit) {
            case GLSLUniformUnit::single: stream << "single "; break;
            case GLSLUniformUnit::vec2: stream << "vec2 "; break;
            case GLSLUniformUnit::vec3: stream << "vec3 "; break;
            case GLSLUniformUnit::vec4: stream << "vec4 "; break;
            case GLSLUniformUnit::mat4: stream << "mat4 "; break;
            case GLSLUniformUnit::sampler2D: stream << "sampler2D "; break;
        }
        cout << name;
        if (count > 1) {
            cout << "[" << count << "]";
        }
        cout << ";" << endl;
    }

    template<class... Members>
    class Shader : public Entity<Members...>{
    public:
        Shader(Members... members) : Entity{ members... } {}

        template<class... T>
        void render(T... binds) {
            [&stream](auto&... args){
                ((cout << "WOW!" << endl), ...);
            }(binds...);
        }

        /**
         * 📝 Write out the GLSL instructions to initialize members of
         * the shader in the vertex shader
         */
        void to_vert_text(ostream& stream) {
            std::apply([&stream](auto&... args){
                ((vert_text(stream, args, get_class_name(args))), ...);
            }, components);
        }
    };

    // 👨‍🔬
    void test() {
        class vert_color : public Attribute<GLSLUnit::vec3, false> {};
        class vert_position : public Attribute<GLSLUnit::vec2, false> {};
        class model_view_projection : public Uniform<GLSLUniformUnit::mat4, 1> {};

        auto shader = Shader{vert_color{}, vert_position{}, model_view_projection()};
        shader.to_vert_text(cout);

        const auto make_from = make_tuple<vector<vec2>,  vector<vec3>, mat4>(
            {
                { -0.6f, -0.4f },
                {  0.6f, -0.4f },
                {   0.f,  0.6f },
            },
            {
                {1.f, 1.f, 0.f},
                {0.f, 1.f, 1.f},
                {1.f, 0.f, 1.f},
            },
            {}
        );
        // shader.render(lister);
    }

    // 🔍 Output state of tuple
    template<typename... Ts>
    std::ostream& operator<<(std::ostream& os, std::tuple<Ts...> const& theTuple)
    {
        std::apply
        (
            [&os](Ts const&... tupleArgs)
            {
                os << '[';
                std::size_t n{0};
                ((os << tupleArgs << (++n != sizeof...(Ts) ? ", " : "")), ...);
                os << ']';
            }, theTuple
        );
        return os;
    }

}
// <- 😎
            

int main()
{
    hyp::test();
    ShaderBuilder::test();

	std::cout << "Hey ho! my Worldlings!" << std::endl;

    return 0;

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

	auto fun_shader = FunShader{
		{
			{ -0.6f, -0.4f },
			{  0.6f, -0.4f },
			{   0.f,  0.6f },
		},
        {
            {1.f, 1.f, 0.f},
            {0.f, 1.f, 1.f},
            {1.f, 0.f, 1.f},
        }
	};

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = (float)width / (float)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

        glm::quat rotation = glm::angleAxis((float)glfwGetTime(), glm::vec3{0.0f, 0.0f, 1.0f});
        auto object = glm::toMat4(rotation);
        auto perspective = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		auto model_view_projection = perspective * object;
		glUseProgram(fun_shader.program);
		fun_shader.model_view_projection.assign(model_view_projection);
        fun_shader.extra_data.assign(glm::vec4((float)glfwGetTime(), (float)glfwGetTime(), (float)glfwGetTime(), 0));
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	std::cout << "Complete!" << std::endl;
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