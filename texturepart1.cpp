//A texture is a 2D image, used to add detail to an object.
//In order to map a texture to the triangle we need to tell each vertex of the triangle which part of the texture corresponds to.
//Each vertex should thus have a texture coordinate associated with them that specifies what part of the texture image to sample from. Fragment interpolation then does the rest for the other fragments.
//Texture coordinates range from 0 to 1 in the x and y axis (la parte de abajo a la izq es 0,0 y la parte arriba derecha es 1,1. No hay negativos). Retrieving the texture color using texture coordinates is called sampling. 
//We specify 3 texture coordinate points for the triangle. We want the bottom-left side of the triangle to correspond with the bottom-left side of the texture so we use (0,0) texture coordinate for the triangle bottom left-vertex. 
//The same applies to the bottom-right with (1,0) texture coordinate.
//The top of the triangle should correspond with the top center of the texture image so we take (0.5,1.0) as its texture coordinate.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION
#include <shader_s.h>
#include <glm.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


int main() {
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader ourShader("C:\\Users\\maqui\\Documents\\OpenGL\\OpenGL\\src\\shader.vert", "C:\\Users\\maqui\\Documents\\OpenGL\\OpenGL\\src\\shader.frag");

    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
//Vertex buffer
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute, we adjust the stride parameter 8*sizeof(float)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
//Texture Wrapping:What happens if we specify coordinates outside of 0 and 1? The default behavior is to repeat texture images. But there are more options:
    //GL_REPEAT: default, repeats the image.
    //GL_MIRRORED_REPEAT: mirrors the image with each repeat.
    //GL_CLAMP_TO_EDGE: Clamps the coordinates between 0 and 1. 
    //GL_CLAMP_TO_BORDER: Coordinates outside the range are now given a user-specified border color.
//Each option can be set per coordinate axis (s,t,r) equivalent to (x,y,z) with glTexParameter*:
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//The first argument specifies the texture target. The second argument requires us to tell what option we want to set and for which texture axis. If we want for both axis we need to call the function twice. The third argument is the wrapping option.
//Texture coordinates do not depend on resolution but can be any floating point value, thus OpenGL has to figure out which texture pixel (texel) to map the texture coordinate to. This is especially important if you have a very large object and a low resolution texture.
//Texture Filtering comes in handy. It has two major options:
    //GL_NEAREST: default. OpenGL selects the texel that center is closest to the texture coordinate.
    //GL_LINEAR: takes an interpolated value from the texture coordinate's neighboring texels, approximating a color between the texels. The smaller the distance from the texture coordinate to a texel's center, the more that texel's color contributes to the sample color.
//Texture filtering can be set for magnifying and minifying operations.
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//MipMaps (No le des mucha importancia): A collection of textures images where each subsequent texture is twice as small compared to the previous one.
    //After a certain distance thershold from the viewer, OpenGL will use a different mipmap texture that best suit the distance to the object
    //Creating a collection of mipmapped textures for each texture image is cumbersome to do manually. All the work is done with a single call to glGenerateMipmaps after we've created the texture.
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//Generating a texture
    unsigned int texture;
    //the first parameter takes as input how many textures we want to generate and stores them in an unsigned int. Se second parameter is an int array
    glGenTextures(1, &texture);
    //Now we bind the texture to a gl function
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    //wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //We start generating a texture using the previously loaded image data
    //The first argument specifies the texture target.
    //The second argument specifies the mipmap level for which we want to create a texture for if you want to set up manually. This is base level which is 0.
    //The third argument tells OpenGL in what kind of format we want to store the texture. Our image has only RGB values so we'll store the texture with RGB values as well.
    //4th and 5th argument sets the width and height of the resulting texture. 
    //6th argument should always be 0
    //7th argument and 8th argument specify the format and datatype of the source image. We loaded the image with RGB values and stored them as chars (bytes) so we'll pass in the corresponding values
    //8th the last argument is the actual image data
    //We use stb_image.h for image loading.
    int width, height, nrChannels;
    unsigned char* data = stbi_load("C:\\Users\\maqui\\Documents\\OpenGL\\Textures\\wall.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    //is a good practice to free the image memory
    stbi_image_free(data);
    //render
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // render container
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}