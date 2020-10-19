#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//Declare all the input vertex attributes in the vertex shader with the in keyword. Since each vertex has a 3D coordinate we create a vec3 input variable with the name aPos. We also specifically set the location of the input variable via layout (location = 0)
//Vector: Mathematical concept that represent the positions/directions in any space. A vector in GLSL has a maximum size of 4 and each of its value can be retrieved via vec.x,vec.y,vec.z and vec.w, where each coordinate represents a coordinate in space. The vec.w is called perspective division. 
//To set the output of the vertex shader we have to assign the position data to the predefined gL_Position variable which is a vec4. Since our input is a vec3 we have to cast it and set its component to 1.0f.
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
//The Fragment shader only requires one output variable and that is a vector of size 4 (vec4) that defines the final color output that we should calculate ourselves. We assign the vec4 to the color output that is FragColor.
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // vertex shader: Is the first part of our 3D, takes as input a single vertex. Transforms 3D coordinates into different 3D coordinates and allows us to do some basic processing on vertex attribute.
    //In order to use the vertexshader we have to dinamically compile. The firs thing is glCreateShader.
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //We attach the shader source code to the shader object and compile the shader.
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader: Calculate the Final color of a pixel. En pocas palabras, es el llenado. Is all about calculating the output of your pixels. Same process as vertex shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    //A shader program object is the final linked version of multiple shaders combined. To use the frag and vert shader we have to link them to a shader program object and then activate this shader program when rendering objects.
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    //We dont need them anymore
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //Vertex Data: Input to the graphics pipeline as a 3D coordinates that should form a triangle in an array. this vertex data is a collection of vertices. A vertex is a collection of data per 3D Coordinate. This vertex data is represented using "vertex attributes" that can contain any data we like (como texeles). En este caso vamos a usarlo para guardar posiciones

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    //VBO: Vertex Buffer Object: Memory on the GPU where we store the vertex data. Just like any OpenGL object, this buffer has a unique ID corresponding to that buffer. glGenBuffers() generate one with a buffer ID
    //VAO: Vertex Array Object: Can be bound just like any VBO and any subsequent vertex attribute calls from that point on will be stored inside the VAO. When configure Vertex attrib pointers you only have to make those calls once and whenever we want to draw the object, we can just bind the corresponding VAO. This makes switching between different vertex data and attribute configurations as easy as binding a different VAO.
    //The VAO stores the following: 
        //Calls the GLEnableVertexArrays
        //Vertex Attribute configurations via glVertexAttribPointer
        //Vertex buffer associated with vertex attributes to glVertexAttribPointer
    //EBO: Is a buffer, just like VBO, that stores indices that OpenGL uses to decide what vertices to draw. This is called indexed drawing.
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    //OpenGL has many tips of buffer objects one of them is GL_ARRAY_BUFFER. OpenGL Allows to bind several buffers at once as long as they have a different buffer type.
    //We link our recently created BVO with the buffer GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //From that point any buffer calls we make  (on GL_ARRAY_BUFFER) will be used to configure the currently bound buffer (VBO). We use glBufferData that copies the defined vertex data (vertices) into VBO.
    //Its first argument is the type of the buffer, the second argument specifies the size of the data in bytes, the third argument is the actual data we want to send, the 4th argument specifies how we want the graphis card to manage the given data:
        //STREAM: the data is set only once and used by the GPU a few times
        //STATIC: the data is set only once and used many times
        //DYNAMIC: the data is changed a lot and used many times
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //We are giving GL_ELEMENT_ARRAY_BUFFER as the buffer target. 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    //We have to specify how OpenGL should interpret the vertex data before rendering.
    //The first parameter specifies which vertex attribute we want to configure. In this case position vertex attribute in the vertex shader with layout=0. We pass 0.
    //The second parameter specifies the size of the vertex attribute. The vertex attribute is vec3 so we pass 3.
    //The third parameter specifies the type of the data.
    //The fourth parameter specifies if we want the data to be normalized.
    //The fifth argument is known as the stride and tells us the space of the consecutive vertex attributes. Since the next attribute is located exactly 3 times the size of a float away we pass 3*sizeof(float).
    //The last parameter is a type void*. Is an offset of where the position data begins in the buffer.
    //Each vertex attribute takes its data from memory managed by a VBO and which VBO it takes its data from is determined by the current VBO bound to GL_ARRAY_BUFFER when calling glVertexAttribPointer.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //We need to enable the vertex attribute giving the vertex attribute location as its argument. Vertex Attribute are disabled by default.
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        //As soon we want to draw an object, we simply bind the VAO with the preferred settings before drawing the object.
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //First parameter takes the primitive type we want to draw, the second parameter specifies the starting index, the third argumment specifies how many vertices we want to draw, which is 3.
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        //The first argument specifies the mode we want to draw, the second argument is the count or number of elements we'd to draw. The third argument is the type of indices. The fouth argument is to specify an offset in the EBO
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time 

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        //En Resumen: a VAO that stores our vertex attribute configuration and which VBO to use. Usually when you have multiple objects we want to draw, you first generate/configure all the VAOs (and thus the required VBO and attribute pointers) and store those for later use. The moment we want to draw one of our objects, we take the corresponding VAO, bind it, then draw the object and unbind the VAO again.
    }
    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}