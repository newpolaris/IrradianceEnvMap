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
#include <SkyBox.h>
#include <Skydome.h>
#include <Mesh.h>

#include <fstream>
#include <memory>

#define GL_ASSERT(x) {x; CHECKGLERROR()}

namespace
{
	struct fRGB {
		float r, g, b; 
	};
    const unsigned int WINDOW_WIDTH = 800u;
    const unsigned int WINDOW_HEIGHT = 600u;
	const char* WINDOW_NAME = "Irradiance Environment Mapping";

	int glut_windowHandle = 0;  

    ProgramShader m_program;
	std::shared_ptr<Texture2D> m_texture;
    PlaneMesh m_mesh;
	SkyBox m_skybox;
	Skydome m_Skydome;

	float coeffs[9][3] ;                /* Spherical harmonic coefficients */
	float matrix[4][4][3] ;             /* Matrix for quadratic form */

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
	void prefilter(fRGB* im, int width, int height);
	void printcoeffs();
	void tomatrix();
	void updatecoeffs(float hdr[3], float domega, float x, float y, float z);

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

	typedef std::vector<std::istream::char_type> ByteArray;
	ByteArray ReadFileSync( const std::string& name )
	{
		std::ifstream inputFile;
		inputFile.open( name, std::ios::binary | std::ios::ate );
		if (!inputFile.is_open())
			return ByteArray();
		auto filesize = static_cast<size_t>(inputFile.tellg());
		ByteArray buf = ByteArray(filesize * sizeof( char ) );
		inputFile.ignore( std::numeric_limits<std::streamsize>::max() );
		inputFile.seekg( std::ios::beg );
		inputFile.read( reinterpret_cast<char*>(buf.data()), filesize );
		inputFile.close();
		return buf;
	}


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
        camera.setViewParams( glm::vec3( 0.0f, 2.0f, 15.0f), glm::vec3( 0.0f, 0.0f, 0.0f) );
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

        m_mesh.init();

		m_texture = std::make_shared<Texture2D>();
        m_texture->initialize();
#if 0
        m_texture->load("resource/skydome/021.jpg");
#else
		const int width = 1000;
		ByteArray image = ReadFileSync( "resource/grace_probe.float" );
		if (image.size() > 0) {
			prefilter(reinterpret_cast<fRGB*>(image.data()), width, width);
			tomatrix();
			printcoeffs();
			m_texture->load(GL_RGB, width, width, GL_RGB, GL_FLOAT, image.data() );
		}
#endif
		m_skybox.init();
		m_skybox.addCubemap( "resource/MountainPath/*.jpg" );
		m_skybox.setCubemap( 0u );
		
		m_Skydome.initialize();
		m_Skydome.setTexture( m_texture );
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
        m_texture->destroy();
		m_Skydome.shutdown();
        Logger::getInstance().close();
	}

	float sinc(float x) {               /* Supporting sinc function */
		if (fabs(x) < 1.0e-4) return 1.0 ;
		else return(sin(x)/x) ;
	}

	void prefilter(fRGB* im, int width, int height)
	{
		/* The main integration routine.  Of course, there are better ways
		   to do quadrature but this suffices.  Calls updatecoeffs to
		   actually increment the integral. Width is the size of the
		   environment map */
		const float PI = 3.141529f;

		int i,j ;
		for (i = 0 ; i < height; i++) {
			for (j = 0 ; j < width ; j++) {

				/* We now find the cartesian components for the point (i,j) */
				float u,v,r,theta,phi,x,y,z,domega ;

				v = (height/2.0 - i)/(height/2.0);  /* v ranges from -1 to 1 */
				u = (j-width/2.0)/(width/2.0);      /* u ranges from -1 to 1 */
				r = sqrt(u*u+v*v) ;                 /* The "radius" */
				if (r > 1.0) continue ;             /* Consider only circle with r<1 */

				theta = PI*r ;                      /* theta parameter of (i,j) */
				phi = atan2(v,u) ;                  /* phi parameter */

				x = sin(theta)*cos(phi) ;           /* Cartesian components */
				y = sin(theta)*sin(phi) ;
				z = cos(theta) ;

				/* Computation of the solid angle.  This follows from some
				   elementary calculus converting sin(theta) d theta d phi into
				   coordinates in terms of r.  This calculation should be redone 
				   if the form of the input changes */

				domega = (2*PI/height)*(2*PI/width)*sinc(theta) ;

				updatecoeffs((float*)(im + width*i + j),domega,x,y,z) ; /* Update Integration */
			}
		}
	}

	void printcoeffs()
	{
		int i, j;
		/* Output Results */

		printf("\n         Lighting Coefficients\n\n") ;
		printf("(l,m)       RED        GREEN     BLUE\n") ;

		printf("L_{0,0}   %9.6f %9.6f %9.6f\n",
				coeffs[0][0],coeffs[0][1],coeffs[0][2]) ;
		printf("L_{1,-1}  %9.6f %9.6f %9.6f\n",
				coeffs[1][0],coeffs[1][1],coeffs[1][2]) ;
		printf("L_{1,0}   %9.6f %9.6f %9.6f\n",
				coeffs[2][0],coeffs[2][1],coeffs[2][2]) ;
		printf("L_{1,1}   %9.6f %9.6f %9.6f\n",
				coeffs[3][0],coeffs[3][1],coeffs[3][2]) ;
		printf("L_{2,-2}  %9.6f %9.6f %9.6f\n",
				coeffs[4][0],coeffs[4][1],coeffs[4][2]) ;
		printf("L_{2,-1}  %9.6f %9.6f %9.6f\n",
				coeffs[5][0],coeffs[5][1],coeffs[5][2]) ;
		printf("L_{2,0}   %9.6f %9.6f %9.6f\n",
				coeffs[6][0],coeffs[6][1],coeffs[6][2]) ;
		printf("L_{2,1}   %9.6f %9.6f %9.6f\n",
				coeffs[7][0],coeffs[7][1],coeffs[7][2]) ;
		printf("L_{2,2}   %9.6f %9.6f %9.6f\n",
				coeffs[8][0],coeffs[8][1],coeffs[8][2]) ;

		printf("\nMATRIX M: RED\n") ;
		for (i = 0 ; i < 4 ; i++) {
			for (j = 0 ; j < 4 ; j++)
				printf("%9.6f ",matrix[i][j][0]) ;
			printf("\n") ;
		}
		printf("\nMATRIX M: GREEN\n") ;
		for (i = 0 ; i < 4 ; i++) {
			for (j = 0 ; j < 4 ; j++)
				printf("%9.6f ",matrix[i][j][1]) ;
			printf("\n") ;
		}
		printf("\nMATRIX M: BLUE\n") ;
		for (i = 0 ; i < 4 ; i++) {
			for (j = 0 ; j < 4 ; j++)
				printf("%9.6f ",matrix[i][j][2]) ;
			printf("\n") ;
		}

	}

	void tomatrix(void) {

		/* Form the quadratic form matrix (see equations 11 and 12 in paper) */

		int col ;
		float c1,c2,c3,c4,c5 ;
		c1 = 0.429043 ; c2 = 0.511664 ;
		c3 = 0.743125 ; c4 = 0.886227 ; c5 = 0.247708 ;

		for (col = 0 ; col < 3 ; col++) { /* Equation 12 */

			matrix[0][0][col] = c1*coeffs[8][col] ; /* c1 L_{22}  */
			matrix[0][1][col] = c1*coeffs[4][col] ; /* c1 L_{2-2} */
			matrix[0][2][col] = c1*coeffs[7][col] ; /* c1 L_{21}  */
			matrix[0][3][col] = c2*coeffs[3][col] ; /* c2 L_{11}  */

			matrix[1][0][col] = c1*coeffs[4][col] ; /* c1 L_{2-2} */
			matrix[1][1][col] = -c1*coeffs[8][col]; /*-c1 L_{22}  */
			matrix[1][2][col] = c1*coeffs[5][col] ; /* c1 L_{2-1} */
			matrix[1][3][col] = c2*coeffs[1][col] ; /* c2 L_{1-1} */

			matrix[2][0][col] = c1*coeffs[7][col] ; /* c1 L_{21}  */
			matrix[2][1][col] = c1*coeffs[5][col] ; /* c1 L_{2-1} */
			matrix[2][2][col] = c3*coeffs[6][col] ; /* c3 L_{20}  */
			matrix[2][3][col] = c2*coeffs[2][col] ; /* c2 L_{10}  */

			matrix[3][0][col] = c2*coeffs[3][col] ; /* c2 L_{11}  */
			matrix[3][1][col] = c2*coeffs[1][col] ; /* c2 L_{1-1} */
			matrix[3][2][col] = c2*coeffs[2][col] ; /* c2 L_{10}  */
			matrix[3][3][col] = c4*coeffs[0][col] - c5*coeffs[6][col] ; 
			/* c4 L_{00} - c5 L_{20} */
		}
	}


	void updatecoeffs(float hdr[3], float domega, float x, float y, float z) 
	{ 

		/****************************************************************** 
		  Update the coefficients (i.e. compute the next term in the
		  integral) based on the lighting value hdr[3], the differential
		  solid angle domega and cartesian components of surface normal x,y,z

		  Inputs:  hdr = L(x,y,z) [note that x^2+y^2+z^2 = 1]
		  i.e. the illumination at position (x,y,z)

		  domega = The solid angle at the pixel corresponding to 
		  (x,y,z).  For these light probes, this is given by 

		  x,y,z  = Cartesian components of surface normal

		  Notes:   Of course, there are better numerical methods to do
		  integration, but this naive approach is sufficient for our
		  purpose.

		 *********************************************************************/

		int col ;
		for (col = 0 ; col < 3 ; col++) {
			float c ; /* A different constant for each coefficient */

			/* L_{00}.  Note that Y_{00} = 0.282095 */
			c = 0.282095 ;
			coeffs[0][col] += hdr[col]*c*domega ;

			/* L_{1m}. -1 <= m <= 1.  The linear terms */
			c = 0.488603 ;
			coeffs[1][col] += hdr[col]*(c*y)*domega ;   /* Y_{1-1} = 0.488603 y  */
			coeffs[2][col] += hdr[col]*(c*z)*domega ;   /* Y_{10}  = 0.488603 z  */
			coeffs[3][col] += hdr[col]*(c*x)*domega ;   /* Y_{11}  = 0.488603 x  */

			/* The Quadratic terms, L_{2m} -2 <= m <= 2 */

			/* First, L_{2-2}, L_{2-1}, L_{21} corresponding to xy,yz,xz */
			c = 1.092548 ;
			coeffs[4][col] += hdr[col]*(c*x*y)*domega ; /* Y_{2-2} = 1.092548 xy */ 
			coeffs[5][col] += hdr[col]*(c*y*z)*domega ; /* Y_{2-1} = 1.092548 yz */ 
			coeffs[7][col] += hdr[col]*(c*x*z)*domega ; /* Y_{21}  = 1.092548 xz */ 

			/* L_{20}.  Note that Y_{20} = 0.315392 (3z^2 - 1) */
			c = 0.315392 ;
			coeffs[6][col] += hdr[col]*(c*(3*z*z-1))*domega ; 

			/* L_{22}.  Note that Y_{22} = 0.546274 (x^2 - y^2) */
			c = 0.546274 ;
			coeffs[8][col] += hdr[col]*(c*(x*x-y*y))*domega ;

		}
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

		// m_skybox.render( camera );
		m_Skydome.render( camera );

        // Use our shader
        m_program.bind();

        glm::mat4 mvp = camera.getViewProjMatrix() * m_mesh.getModelMatrix();
        m_program.setUniform( "uModelViewProjMatrix", mvp );
        m_texture->bind(0u);
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

			case 'r':
				m_skybox.toggleAutoRotate();
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
