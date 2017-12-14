#ifdef _WIN32
  #include <windows.h>
#endif

// OpenGL & GLEW (GL Extension Wrangler)
#include <GL/glew.h>

// GLUT (GL Utility Toolkit)
#if defined(__APPLE__) || defined(MACOSX)
  #include <GLUT/glut.h>
#else
  #include <GL/freeglut.h>
#endif

// GLM for matrix transformation
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

// GLSL Wrangler
#include <glsw/glsw.h>

// Standard libraries
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <tools/TCamera.hpp>
#include <tools/Timer.hpp>
#include <tools/Logger.hpp>
#include <tools/gltools.hpp>

#include <GLType/ProgramShader.h>
#include <GLType/Texture.h>
#include <Mesh.h>

#define GL_ASSERT(x) {x; CHECKGLERROR()}

namespace
{
    const unsigned int WINDOW_WIDTH = 800u;
    const unsigned int WINDOW_HEIGHT = 600u;
	const char* WINDOW_NAME = "Irradiance Environment Mapping";

	int glut_windowHandle = 0;  

    ProgramShader m_program;
    Texture2D m_texture;
    PlaneMesh m_mesh;

    //~

    TCamera camera;

    // App app;

    bool bWireframe = false;
    bool bFullscreen = false;

    //~

	void initApp(int argc, char** argv);
	void initExtension();
	void initGL();
	void initWindow(int argc, char** argv);
    void finalizeApp();

    void glut_reshape_callback(int, int);    
    void glut_display_callback();
    void glut_keyboard_callback( unsigned char, int, int);
    void glut_special_callback( int, int, int);    
    void glut_specialUp_callback( int, int, int);    
    void glut_mouse_callback(int, int, int, int);
    void glut_motion_callback(int, int);
    void glut_idle_callback();
    void glut_timer_callback(int);
}

namespace {

	void initApp(int argc, char** argv)
	{
		// window maanger
		initWindow(argc, argv);

		// OpenGL extensions
		initExtension();

		// OpenGL
		initGL();

		// GLSW : shader file manager
		glswInit();
		glswSetPath("./shaders/", ".glsl");
		glswAddDirectiveToken("*", "#version 330 core");

        // App Objects
        camera.setViewParams( glm::vec3( 0.0f, 2.0f, 15.0f),
                glm::vec3( 0.0f, 0.0f, 0.0f) );
        camera.setMoveCoefficient(0.35f);

        Timer::getInstance().start();
        // Logger::getInstance().open("logfile");

        // app.init( &camera ); 

        GLuint VertexArrayID;
        GL_ASSERT(glGenVertexArrays(1, &VertexArrayID));
        GL_ASSERT(glBindVertexArray(VertexArrayID));

        m_program.initalize();
        m_program.addShader( GL_VERTEX_SHADER, "Default.Vertex");
        m_program.addShader( GL_FRAGMENT_SHADER, "Default.Fragment");
        m_program.link();  
	}

	void initExtension()
	{
        glewExperimental = GL_TRUE;

        GLenum result = glewInit(); 
        if (result != GLEW_OK)
        {
            fprintf( stderr, "Error: %s\n", glewGetErrorString(result));
            exit( EXIT_FAILURE );
        }

        fprintf( stderr, "GLEW version : %s\n", glewGetString(GLEW_VERSION));
	}

	void initGL()
	{
        // Clear the error buffer (it starts with an error).
        glGetError();

        std::printf("%s\n%s\n", 
                glGetString(GL_RENDERER),  // e.g. Intel HD Graphics 3000 OpenGL Engine
                glGetString(GL_VERSION)    // e.g. 3.2 INTEL-8.0.61
        );

        glClearColor( 0.15f, 0.15f, 0.15f, 0.0f);

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );

        glDisable( GL_STENCIL_TEST );
        glClearStencil( 0 );

        glDisable( GL_CULL_FACE );
        glCullFace( GL_BACK );    
        glFrontFace(GL_CCW);

        glDisable( GL_MULTISAMPLE );

        m_mesh.init();
        m_texture.initialize();
        m_texture.load("resource/girl.png");
	}

	void initWindow(int argc, char** argv)
	{
		glutInit(&argc, argv);
		glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

        glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
		glut_windowHandle = glutCreateWindow( WINDOW_NAME );

		if (glut_windowHandle < 1)
		{
			fprintf( stderr, "Error: window creation failed.\n");
			exit( EXIT_FAILURE );
		}

		// GLUT Events' Callback
		glutReshapeFunc( glut_reshape_callback );
		glutDisplayFunc( glut_display_callback );
		glutKeyboardFunc( glut_keyboard_callback );
		glutSpecialFunc( glut_special_callback );
		glutSpecialUpFunc( glut_specialUp_callback );
		glutMouseFunc( glut_mouse_callback );
		glutMotionFunc( glut_motion_callback );
		glutIdleFunc( glut_idle_callback );

	}

	void finalizeApp()
	{
        glswShutdown();  
        m_program.destroy();
        m_mesh.destroy();
        m_texture.destroy();
        Logger::getInstance().close();
	}

    // GLUT Callbacks_________________________________________________  


    void glut_reshape_callback(int w, int h)
    {
        glViewport( 0, 0, w, h);

        float aspectRatio = ((float)w) / ((float)h);
        camera.setProjectionParams( 60.0f, aspectRatio, 0.1f, 250.0f);
    }

    void glut_display_callback()
    {    
        Timer::getInstance().update();
        camera.update();
        // app.update();

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );    
        glPolygonMode(GL_FRONT_AND_BACK, (bWireframe)? GL_LINE : GL_FILL);

        // Use our shader
        m_program.bind();

        glm::mat4 mvp = camera.getViewProjMatrix() * m_mesh.getModelMatrix();
        m_program.setUniform( "uModelViewProjMatrix", mvp );
        m_texture.bind(0u);
        m_mesh.draw();

        // app.render();
        m_program.unbind();

        glutSwapBuffers();
    }

    void glut_keyboard_callback(unsigned char key, int x, int y)
    {
        switch (key)
        {
            case 27:
                exit( EXIT_SUCCESS );

            case 'w':
                bWireframe = !bWireframe;
                break;

            case 't':
                {
                    Timer &timer = Timer::getInstance();
                    printf( "fps : %d [%.3f ms]\n", timer.getFPS(), timer.getElapsedTime());
                }
                break;

            case 'f':      
                bFullscreen = !bFullscreen;

                if (bFullscreen) {
                    glutFullScreen();
                } else {
                    glutReshapeWindow( WINDOW_WIDTH, WINDOW_HEIGHT);
                }
                break;

            default:
                break;
        }

        // app.keyEvent(key);
    }

    void moveCamera( int key, bool isPressed)
    {
        switch (key)
        {
            case GLUT_KEY_UP:
                camera.keyboardHandler( MOVE_FORWARD, isPressed);
                break;

            case GLUT_KEY_DOWN:
                camera.keyboardHandler( MOVE_BACKWARD, isPressed);
                break;

            case GLUT_KEY_LEFT:
                camera.keyboardHandler( MOVE_LEFT, isPressed);
                break;

            case GLUT_KEY_RIGHT:
                camera.keyboardHandler( MOVE_RIGHT, isPressed);
                break;
        }
    }

    void glut_special_callback(int key, int x, int y)
    {
        moveCamera( key, true);
    }  

    void glut_specialUp_callback(int key, int x, int y)
    {
        moveCamera( key, false);
    }  

    void glut_motion_callback(int x, int y)
    {
        camera.motionHandler( x, y, false);    
        glutPostRedisplay();
    }  

    void glut_mouse_callback(int button, int state, int x, int y)
    {
        if (state == GLUT_DOWN) { camera.motionHandler( x, y, true); }    
    }

    void glut_idle_callback()
    {    
        //glutWarpPointer( WINDOW_WIDTH/2, WINDOW_HEIGHT/2);    
        glutPostRedisplay();    
    }
}

int main(int argc, char** argv)
{
	initApp(argc, argv);
	glutMainLoop();
	finalizeApp();
    return EXIT_SUCCESS;
}
