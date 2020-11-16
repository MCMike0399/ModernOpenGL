using OpenTK.Windowing.Desktop;
using OpenTK.Graphics.OpenGL4;
using OpenTK.Windowing.Common;
using OpenTK.Mathematics;
using OpenTK.Windowing.GraphicsLibraryFramework;

namespace MiniProyecto2
{
    /*
     * OpenGL expects that all vertices, that we want to become visible, to be normalized device coordinates after each 
     * vertex shader run. That is, x,y,z coord of each vertex should be between 1.0 and -1.0, coordinates outside this range
     * will not be visible. What we usually do, is specify the coordinates in a range (or space) we determine ourselves and
     * in the vertex shader transform these coordinates to normalized device coordinates (NDC). These NDC are the given to 
     * the rasterizer to transform them to 2D coordinates/pixels on your screen.
     * 
     * Transforming coord to NDC is usually accomplished in a step-by-step fashion where we transform an object's vertices to 
     * several coordinates systems before finally transforming them to NDC. The advantage of transforming them to several 
     * intermidiate coordinate systems is that some operations/calculations are easier in certain coordinate systems. There are
     * 5 different coordinate systems that are important: Local space, World Space, View Space, Clip Space, Screen Space.
     * 
     * To transform the coordinates from one space to the next coordinate space we'll use several transformation matrices of which
     * the most important are the model, view and projection matrix. Our Vertex coordinate first start in local space as local coordinates,
     * and are then further processed to world coordinates, view coordinates, clip coordinates, and eventually end up as screen coordinates.
     * The following image displays the process (see imageCoord1.png)
     * 
     * 1. Local coordinates are the coordinates of your object relative to its local origin, they are the coordinates your object begins in.
     * 2. The next step is to transform the local coordinates to world-space coordinates which are coordinates in respect of a larger world. These coordinates are relative to some global origin of the world, together with many other objects also placed relative to this world's origin.
     * 3. Next we transform the world coordinates to view-space coordinates in such a way that each coordinate is as seen from the camera or viewer's point of view.
     * 4. After the coordinates are in view space we want to project them to clip coordinates. Clip coordinates are processed to the -1.0 and 1.0 range and determine which vertices will end up on the screen. Projection to clip-space coordinates can add perspective if using perspective projection.
     * 5. And lastly we transform the clip coordinates to screen coordinates in a process called viewport transform that transform the coordinates from -1.0 to 1.0 to the coordinate range defined by glViewPort. The resulting coordinates are then sent to the rasterizer to turn them into fragments.
     * 
     * Local Space: Is the coordinate space that is local to your object, i.e where your object begins in. The vertices of the container we've been using were specified as coordinates between -0.5 and 0.5 as its origin. These are local coordinates.
     * World Space: The coordinate of all your vertices relative to a (game) world. This is the coordinate space where you want your objects transformed to in such a way that they are all scattered around the place. The coordinates of your objects are transformed from local to world space. This is accomplished with the model matrix.
     * View Space: Refered as the camera of OpenGL. Is the result of transforming your world space coordinates to coordinates that are in front of the user's view. The view space is thus the space as seen from's the camera point of view. This is usually accomplished with a combination of translations and rotations to translate/rotate the scene so that certain items are transformed to the front of the camera. This is stored in the view matrix.
     * Clip Space: At the end of each vertex shader run, OpenGL expects the coordinates to be within a specific range and any coordinate that falls outside this range is clipped. Coordinates that are clipped are discarded, so the remaining coordinates will end up as fragments visible on your screen.
     *              To transform vertex coordinates to clip-space we defined a projection matrix that specifies a range of coordinates (e.g -1000 to 1000 in each dimension). The projection matrix then transforms coordinates within this specified range to normalized device coordinates (-1.0,1.0). All coordinates outside this range will not be mapped between -1.0 and 1.0 and therefore be clipped. Ex: a coordinate (1250,500,700) would no be visible, since the x coordinate is out of range and thus get converted to a coordinate higher that 1.0 in NDC and is therefore clipped.
     *              Note that if only a part of a primitive e.g a triangle is outside the clipping volume, OpenGL will reconstruct the triangle as one or more triangles to fit inside the clipping range. 
     * 
     * Frustum: viewing box created by a projection matrix. Each coordinate that ends up in the frustrum will end up in the user's screen.
     * Projection: The total process to convert coordinates within a specified range to NDC that can be easily be mapped to 2D view-space coordinates. It's called projection since the projection matrix projects 3D coordinates to the easy-to-map-to-2D normalized coordinates.
     * 
     * Once all the vertices are transformed to clip space a final operation called perspective division is performed where we can devide the x,y,z componentes of the positions vector by the vector's homogeneous w component.
     * It is after this stage where the resulting coordinate are mapped to screen coordinates (glViewPort) and turned into fragments.
     *
     * The projectin matrix to transform view coordinates to clip coordinates usually takes two different forms, where each form defines its own unique frustum. We can either create an ortographic projection matrix or a perspective projection matrix.
     * 
     * Ortographic projection matrix: Defines a cube-like frustum box that defines the clipping space where each vertex outside this box is clipped. When creating an ortographic projection matrix we specify the width, height and length of the visible frustum. All the coordinates inside the frustum will end up within the NDC range after transformed by its matrix and thus won't be clipped. The frustum looks like a container (see picture 1) .
     * The frustum defines the visible coordinate and is specified by a width, a height and a near and far plane. Any coordinate in front of the near plane is clipped and the same applies to coordinates behind the far plane.
     * The ortographic frustum directly maps all coordinates inside the frustum to normalized device coordinates without any special side effect since it won't touch the w component of the transformed vector, if the w component remains equal to 1.0 respective division won't change the coordinates.
     * 
     * Perspective projection matrix: Maps a given frustum range to clip space, but also manipulates the w value of each vertex coord in such a way that the further away a vertex coord is from the viewer, the higher this w component becomes.
     * Once the coordinates are transformed to clip space they are in range -w to w. OpenGL requires that the visible coordinates fall between the range of -1.0 and 1.0 as the final vertex shader output, thus once the coordinates are in clip space, perspective division is applied to the clip space coordinates (see image 2).
     * Each component of the vertex coordinate is divided by its w component giving smaller vertex coordinate the further away a vertex is from the viewer. The resulting coordiantes are then in normalize device space. A perspective projection matrix can be created as follows: 
     * "private Matrix4 projection = Matrix4.CreatePerspectiveFieldOfView(MathHelper.DegreesToRadians(45f), Width / (float) Height, 0.1f, 100.0f);"
     * create a large frustum that defines the visible space, anything outside the frustum will not end up in the clip space volume and will thus become clipped. A perspective frustum can be visualized as a non-uniformly shaped box from where each coordinate inside this box will be mapped to a point in clip space. (see image 3).
     * Its first parameter defines the fov value, that stands for field of view and sets how large the viewspace is. For a realistic view it is usually set to 45 degrees, but for more doom-style results you could set it to a higher value. The second parameter sets the aspect ratio which is calculated by dividing the viewport's width by its height. The third and fourth parameter set the near and far plane of the frustum. We usually set the near distance to 0.1 and the far distance to 100.0. All the vertices between the near and far plane and inside the frustum will be rendered.
     * We create a transformation matrix for each of the aforementioned steps: model, view and projection matrix. A vertex coordinate is then transformed to clip coordinates as follows (see image 4)
     * 
    */

    class Coord : GameWindow
    {
        public Coord(GameWindowSettings gameWindowSettings, NativeWindowSettings nativeWindowSettings) : base
            (gameWindowSettings, nativeWindowSettings)
        { }
        private readonly float[] _vertices =
        {
            // Position         Texture coordinates
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top right
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // top left
        };

        private readonly uint[] _indices =
        {
            0, 1, 3,
            1, 2, 3
        };

        private int _elementBufferObject;

        private int _vertexBufferObject;

        private int _vertexArrayObject;

        private Shader _shader;

        private Texture _texture;

        private Texture _texture2;

        // We create a double to hold how long has passed since the program was opened.
        private double _time;

        // Then, we create two matrices to hold our view and projection. They're initialized at the bottom of OnLoad.
        // The view matrix is what you might consider the "camera". It represents the current viewport in the window.
        private Matrix4 _view;

        // This represents how the vertices will be projected. It's hard to explain through comments,
        // so check out the web version for a good demonstration of what this does.
        private Matrix4 _projection;

        protected override void OnLoad()
        {
            GL.ClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            // We enable depth testing here. If you try to draw something more complex than one plane without this,
            // you'll notice that polygons further in the background will occasionally be drawn over the top of the ones in the foreground.
            // Obviously, we don't want this, so we enable depth testing. We also clear the depth buffer in GL.Clear over in OnRenderFrame.
            GL.Enable(EnableCap.DepthTest);

            _vertexBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexBufferObject);
            GL.BufferData(BufferTarget.ArrayBuffer, _vertices.Length * sizeof(float), _vertices, BufferUsageHint.StaticDraw);

            _elementBufferObject = GL.GenBuffer();
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, _elementBufferObject);
            GL.BufferData(BufferTarget.ElementArrayBuffer, _indices.Length * sizeof(uint), _indices, BufferUsageHint.StaticDraw);

            // shader.vert has been modified. Take a look at it after the explanation in OnRenderFrame.
            _shader = new Shader("Shaders/vert.glsl", "Shaders/frag.glsl");
            _shader.Use();

            _texture = new Texture("Textures/out/container.png");
            _texture.Use();

            _texture2 = new Texture("Textures/out/awesomeface.png");
            _texture2.Use(TextureUnit.Texture1);

            _shader.SetInt("texture0", 0);
            _shader.SetInt("texture1", 1);

            _vertexArrayObject = GL.GenVertexArray();
            GL.BindVertexArray(_vertexArrayObject);

            GL.BindBuffer(BufferTarget.ArrayBuffer, _vertexArrayObject);
            GL.BindBuffer(BufferTarget.ElementArrayBuffer, _elementBufferObject);

            var vertexLocation = _shader.GetAttribLocation("aPosition");
            GL.EnableVertexAttribArray(vertexLocation);
            GL.VertexAttribPointer(vertexLocation, 3, VertexAttribPointerType.Float, false, 5 * sizeof(float), 0);

            var texCoordLocation = _shader.GetAttribLocation("aTexCoord");
            GL.EnableVertexAttribArray(texCoordLocation);
            GL.VertexAttribPointer(texCoordLocation, 2, VertexAttribPointerType.Float, false, 5 * sizeof(float), 3 * sizeof(float));

            // For the view, we don't do too much here. Next tutorial will be all about a Camera class that will make it much easier to manipulate the view.
            // For now, we move it backwards three units on the Z axis.
            _view = Matrix4.CreateTranslation(0.0f, 0.0f, -3.0f);

            // For the matrix, we use a few parameters.
            //   Field of view. This determines how much the viewport can see at once. 45 is considered the most "realistic" setting, but most video games nowadays use 90
            //   Aspect ratio. This should be set to Width / Height.
            //   Near-clipping. Any vertices closer to the camera than this value will be clipped.
            //   Far-clipping. Any vertices farther away from the camera than this value will be clipped.
            _projection = Matrix4.CreatePerspectiveFieldOfView(MathHelper.DegreesToRadians(45f), Size.X / (float) Size.Y, 0.1f, 100.0f);

            // Now, head over to OnRenderFrame to see how we setup the model matrix

            base.OnLoad();
        }

        protected override void OnRenderFrame(FrameEventArgs e)
        {
            // We add the time elapsed since last frame, times 4.0 to speed up animation, to the total amount of time passed.
            _time += 4.0 * e.Time;

            // We clear the depth buffer in addition to the color buffer
            GL.Clear(ClearBufferMask.ColorBufferBit | ClearBufferMask.DepthBufferBit);

            GL.BindVertexArray(_vertexArrayObject);

            _texture.Use();
            _texture2.Use(TextureUnit.Texture1);
            _shader.Use();

            // Finally, we have the model matrix. This determines the position of the model.
            var model = Matrix4.Identity * Matrix4.CreateRotationX((float)MathHelper.DegreesToRadians(_time));

            // Then, we pass all of these matrices to the vertex shader.
            // You could also multiply them here and then pass, which is faster, but having the separate matrices available is used for some advanced effects

            // IMPORTANT: OpenTK's matrix types are transposed from what OpenGL would expect - rows and columns are reversed.
            // They are then transposed properly when passed to the shader.
            // If you pass the individual matrices to the shader and multiply there, you have to do in the order "model, view, projection",
            // but if you do it here and then pass it to the vertex, you have to do it in order "projection, view, model".
            _shader.SetMatrix4("model", model);
            _shader.SetMatrix4("view", _view);
            _shader.SetMatrix4("projection", _projection);

            GL.DrawElements(PrimitiveType.Triangles, _indices.Length, DrawElementsType.UnsignedInt, 0);

            SwapBuffers();

            base.OnRenderFrame(e);
        }


        protected override void OnResize(ResizeEventArgs e)
        {
            GL.Viewport(0, 0, Size.X, Size.Y);
            base.OnResize(e);
        }

        protected override void OnUnload()
        {
            GL.BindBuffer(BufferTarget.ArrayBuffer, 0);
            GL.BindVertexArray(0);
            GL.UseProgram(0);

            GL.DeleteBuffer(_vertexBufferObject);
            GL.DeleteVertexArray(_vertexArrayObject);

            GL.DeleteProgram(_shader.Handle);
            GL.DeleteTexture(_texture.Handle);
            GL.DeleteTexture(_texture2.Handle);

            base.OnUnload();
        }
    }
}