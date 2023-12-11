// This template code was originally written by Matt Overby while a TA for CSci5607
// This code was built upon further by Jacob Secunda for Assignment 2B

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Includes
#include "trimesh.hpp"
#include "shader.hpp"

#include "core/Matrix.hpp"

// Constants
const int WIN_WIDTH = 500;
const int WIN_HEIGHT = 500;

// For Viewing Transformation Matrix
const Vector3Df kInitialUpDir(0, 1, 0);
const Vector3Df kInitialViewDir(1, 0, 0);
const Vector3Df kInitialEyePos(-10, -13, 0);

// For Perspective Transformation Matrix
constexpr float kInitialViewLeft = 1;
const float kInitialViewRight = -1;
const float kInitialViewTop = 1;
const float kInitialViewBottom = -1;
const float kInitialViewNear = 1.0;
const float kInitialViewFar = 50.0;

// Scale change factors
const float kTranslateFactor = 0.2f;
const float kRotateFactor = std::numbers::pi_v<float> / 110.f;

//
//	Global state variables
//
namespace Globals {
	float win_width = WIN_WIDTH;
	float win_height = WIN_HEIGHT; // window size
	float aspect = win_width / win_height;
	GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	TriMesh mesh;

	//  Model, view and projection matrices, initialized to the identity
	GLmatrix gModelMatrix;
	GLmatrix gViewMatrix;
	GLmatrix gProjectionMatrix;

	// State
	Vector3Df gEyePos = kInitialEyePos;
	Vector3Df gViewDir = kInitialViewDir;
	Vector3Df gUpDir = kInitialUpDir;
	
	float gViewTop = kInitialViewTop;
	float gViewBottom = kInitialViewBottom;
	float gViewLeft = kInitialViewLeft;
	float gViewRight = kInitialViewRight;
	float gViewNear = kInitialViewNear;
	float gViewFar = kInitialViewFar;
}


//
// Function to set up geometry & matrices
//
void init_scene();

void calculate_viewing_matrix();
void calculate_projection_matrix();

void calculate_viewing_matrix_for_eye_change();


//
//	Callbacks
//
static void
error_callback(int error, const char* description)
{
	std::cerr << "Error: " << description << '\n';
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Close on escape or Q
	if (action == GLFW_PRESS)
	{
		switch (key) {
			case GLFW_KEY_ESCAPE:
			case GLFW_KEY_Q:
			{
				glfwSetWindowShouldClose(window, GL_TRUE);
				break;
			}
		}
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key) {
			// Translate camera forward using the viewing direction
			case GLFW_KEY_W:
			{
				Globals::gEyePos += Globals::gViewDir * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Translate camera backward using the opposite of the viewing direction
			case GLFW_KEY_S:
			{
				Globals::gEyePos -= Globals::gViewDir * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Translate camera horizontally to the left using the -u direction
			case GLFW_KEY_A:
			{
				Vector3Df n = Globals::gViewDir.Normalize() * -1.f;
				Vector3Df u = Globals::gUpDir.CrossProduct(n);
				u.NormalizeSelf();

				Globals::gEyePos += u * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Translate camera horizontally to the right using the u direction
			case GLFW_KEY_D:
			{
				Vector3Df n = Globals::gViewDir.Normalize() * -1.f;
				Vector3Df u = Globals::gUpDir.CrossProduct(n);
				u.NormalizeSelf();

				Globals::gEyePos -= u * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Translate camera downwards
			case GLFW_KEY_LEFT_BRACKET:
			{
				Globals::gEyePos -= Globals::gUpDir * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Translate camera upwards
			case GLFW_KEY_RIGHT_BRACKET:
			{
				Globals::gEyePos += Globals::gUpDir * kTranslateFactor;
				calculate_viewing_matrix_for_eye_change();

				break;
			}

			// Rotate viewing direction to the left
			case GLFW_KEY_LEFT:
			{
				GLmatrix rotateMatrix;
				rotateMatrix.RotateAroundYBy(kRotateFactor);

				Globals::gViewDir = rotateMatrix * Globals::gViewDir;
				calculate_viewing_matrix();

				break;
			}

			// Rotate viewing direction to the right
			case GLFW_KEY_RIGHT:
			{
				GLmatrix rotateMatrix;
				rotateMatrix.RotateAroundYBy(-kRotateFactor);

				Globals::gViewDir = rotateMatrix * Globals::gViewDir;
				calculate_viewing_matrix();

				break;
			}
		}
	}
}

static void
framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Globals::win_width = float(width);
	Globals::win_height = float(height);
	Globals::aspect = Globals::win_width / Globals::win_height;
	
	glViewport(0, 0, width, height);

	if (Globals::aspect > 1) {
		Globals::gViewTop = kInitialViewTop / Globals::aspect;
		Globals::gViewBottom = kInitialViewBottom / Globals::aspect;
		Globals::gViewLeft = kInitialViewLeft;
		Globals::gViewRight = kInitialViewRight;
	} else if (Globals::aspect < 1) {
		Globals::gViewLeft = kInitialViewLeft * Globals::aspect;
		Globals::gViewRight = kInitialViewRight * Globals::aspect;
		Globals::gViewTop = kInitialViewTop;
		Globals::gViewBottom = kInitialViewBottom;
	} else {
		Globals::gViewTop = kInitialViewTop;
		Globals::gViewBottom = kInitialViewBottom;
		Globals::gViewLeft = kInitialViewLeft;
		Globals::gViewRight = kInitialViewRight;
	}

	calculate_projection_matrix();
}


//
//	Main
//
int
main(int argc, char* argv[])
{
	// Load the mesh
	std::stringstream obj_file;
	obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	if (!Globals::mesh.load_obj(obj_file.str()))
		return 0;

	Globals::mesh.print_details();
	// FYI: the model dimensions are: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0

	// Setup initial viewing transformation matrix
	calculate_viewing_matrix();

	// Setup initial perspective transformation matrix
	calculate_projection_matrix();
    
    // Define the error callback function
	glfwSetErrorCallback(&error_callback);

	// Initialize glfw
	if (!glfwInit())
		return EXIT_FAILURE;

	// Ask for OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	GLFWwindow* window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2b", nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Define callbacks to handle user input and window resizing
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// More setup stuff
	glfwMakeContextCurrent(window); // Make the window current
    glfwSwapInterval(1); // Set the swap interval

	// make sure the openGL code can be found; folks using Windows need this
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cout << "Failed to gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Initialize the shaders
	// MY_SRC_DIR was defined in CMakeLists.txt
	// it specifies the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss;
	ss << MY_SRC_DIR << "shader.";
	shader.init_from_files(ss.str() + "vert", ss.str() + "frag");

	// Initialize the scene
	init_scene();
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Perform some OpenGL initializations
	glEnable(GL_DEPTH_TEST);  // turn hidden surface removal on
	glClearColor(1.f, 1.f, 1.f, 1.f);  // set the background to white

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();

	// Bind buffers
	glBindVertexArray(Globals::tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);
    
	// Game loop
	while (!glfwWindowShouldClose(window)) {
		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Send updated info to the GPU
		glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, Globals::gModelMatrix); // model transformation
		glUniformMatrix4fv(shader.uniform("view"), 1, GL_FALSE, Globals::gViewMatrix); // viewing transformation
		glUniformMatrix4fv(shader.uniform("projection"), 1, GL_FALSE, Globals::gProjectionMatrix); // projection matrix

		// Draw
		glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size() * 3, GL_UNSIGNED_INT, nullptr);

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();
    
	return EXIT_SUCCESS;
}


void
init_scene()
{
	using namespace Globals;

	// Create the buffer for vertices
	glGenBuffers(1, verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size() * sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
	glBindVertexArray(tris_vao);

	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), nullptr);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), nullptr);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), nullptr);

	// Done setting data for the vao
	glBindVertexArray(0);
}

void
calculate_viewing_matrix()
{
	Vector3Df n = Globals::gViewDir;
	n.NormalizeSelf();
	n *= -1.f;
	
	Vector3Df u = Globals::gUpDir.CrossProduct(n);
	u.NormalizeSelf();

	Vector3Df v = n.CrossProduct(u);
	v.NormalizeSelf();

	Globals::gViewMatrix[0] = u.dx;
	Globals::gViewMatrix[1] = v.dx;
	Globals::gViewMatrix[2] = n.dx;
	Globals::gViewMatrix[3] = 0;

	Globals::gViewMatrix[4] = u.dy;
	Globals::gViewMatrix[5] = v.dy;
	Globals::gViewMatrix[6] = n.dy;
	Globals::gViewMatrix[7] = 0;

	Globals::gViewMatrix[8] = u.dz;
	Globals::gViewMatrix[9] = v.dz;
	Globals::gViewMatrix[10] = n.dz;
	Globals::gViewMatrix[11] = 0;

	Globals::gViewMatrix[12] = -Globals::gEyePos.DotProduct(u);
	Globals::gViewMatrix[13] = -Globals::gEyePos.DotProduct(v);
	Globals::gViewMatrix[14] = -Globals::gEyePos.DotProduct(n);
	Globals::gViewMatrix[15] = 1;
}


void
calculate_projection_matrix()
{
	Globals::gProjectionMatrix[0] = (2.f * Globals::gViewNear) / (Globals::gViewRight - Globals::gViewLeft);
	Globals::gProjectionMatrix[1] = 0;
	Globals::gProjectionMatrix[2] = 0;
	Globals::gProjectionMatrix[3] = 0;

	Globals::gProjectionMatrix[4] = 0;
	Globals::gProjectionMatrix[5] = (2.f * Globals::gViewNear) / (Globals::gViewTop - Globals::gViewBottom);
	Globals::gProjectionMatrix[6] = 0;
	Globals::gProjectionMatrix[7] = 0;

	Globals::gProjectionMatrix[8] = (Globals::gViewRight + Globals::gViewLeft) / (Globals::gViewRight - Globals::gViewLeft);
	Globals::gProjectionMatrix[9] = (Globals::gViewTop + Globals::gViewBottom) / (Globals::gViewTop - Globals::gViewBottom);
	Globals::gProjectionMatrix[10] = -(Globals::gViewFar + Globals::gViewNear) / (Globals::gViewFar - Globals::gViewNear);
	Globals::gProjectionMatrix[11] = -1;

	Globals::gProjectionMatrix[12] = 0;
	Globals::gProjectionMatrix[13] = 0;
	Globals::gProjectionMatrix[14] = (-2.f * Globals::gViewFar * Globals::gViewNear) / (Globals::gViewFar - Globals::gViewNear);
	Globals::gProjectionMatrix[15] = 0;
}


void
calculate_viewing_matrix_for_eye_change()
{
	Vector3Df n = Globals::gViewDir;
	n.NormalizeSelf();
	n *= -1.f;

	Vector3Df u = Globals::gUpDir.CrossProduct(n);
	u.NormalizeSelf();

	Vector3Df v = n.CrossProduct(u);
	v.NormalizeSelf();

	Globals::gViewMatrix[12] = -Globals::gEyePos.DotProduct(u);
	Globals::gViewMatrix[13] = -Globals::gEyePos.DotProduct(v);
	Globals::gViewMatrix[14] = -Globals::gEyePos.DotProduct(n);
}

