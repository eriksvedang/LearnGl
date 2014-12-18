#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

GLuint program;
GLint attribute_coord2d;
GLuint vbo_triangle;

char* file_read(const char* filename)
{
  FILE* input = fopen(filename, "rb");
  if(input == NULL) return NULL;
 
  if(fseek(input, 0, SEEK_END) == -1) return NULL;
  long size = ftell(input);
  if(size == -1) return NULL;
  if(fseek(input, 0, SEEK_SET) == -1) return NULL;
 
  /* if using c-compiler: don't cast malloc's return value */
  char *content = (char*) malloc( (size_t) size +1  ); 
  if(content == NULL) return NULL;
 
  fread(content, 1, (size_t)size, input);
  if(ferror(input)) {
    free(content);
    return NULL;
  }
 
  fclose(input);
  content[size] = '\0';
  return content;
}

void print_log(GLuint object)
{
  GLint log_length = 0;
  if (glIsShader(object))
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else if (glIsProgram(object))
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  else {
    fprintf(stderr, "printlog: Not a shader or a program\n");
    return;
  }
 
  char* log = (char*)malloc(log_length);
 
  if (glIsShader(object))
    glGetShaderInfoLog(object, log_length, NULL, log);
  else if (glIsProgram(object))
    glGetProgramInfoLog(object, log_length, NULL, log);
 
  fprintf(stderr, "%s", log);
  free(log);
}

GLuint create_shader(const char* filename, GLenum type)
{
  const GLchar* source = file_read(filename);
  if (source == NULL) {
    fprintf(stderr, "Error opening %s: ", filename); perror("");
    return 0;
  }
  GLuint res = glCreateShader(type);
  const GLchar* sources[2] = {
#ifdef GL_ES_VERSION_2_0
    "#version 100\n"
    "#define GLES2\n",
#else
    "#version 120\n",
#endif
    source };
  glShaderSource(res, 2, sources, NULL);
  free((void*)source);
 
  glCompileShader(res);
  GLint compile_ok = GL_FALSE;
  glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
  if (compile_ok == GL_FALSE) {
    fprintf(stderr, "%s:", filename);
    print_log(res);
    glDeleteShader(res);
    return 0;
  }
 
  return res;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
      return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "ERIK IS LEARNING", NULL, NULL);
    if (!window)
    {
      glfwTerminate();
      return -1;
    }

    glfwMakeContextCurrent(window);

    /* Must initialize glew before starting to use open gl functions (or it will segfault) */
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
      fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
      return EXIT_FAILURE;
    }

    GLuint vs, fs;
    if ((vs = create_shader("triangle.v.glsl", GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = create_shader("triangle.f.glsl", GL_FRAGMENT_SHADER)) == 0) return 0;
    
    /* GLSL program */
    GLint link_ok = GL_FALSE;
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
      fprintf(stderr, "glLinkProgram:");
      print_log(program);
      return 0;
    }

    const char* attribute_name = "coord2d";
    attribute_coord2d = glGetAttribLocation(program, attribute_name);
    if (attribute_coord2d == -1) {
      fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
      return 0;
    }

    GLfloat triangle_vertices[] = {
      0.0,  0.8,
      -0.8, -0.5,
      0.8, -0.5,
      -0.8, -0.5,
      0.0, -0.9,
      0.8, -0.5,
    };
    glGenBuffers(1, &vbo_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
      glClearColor(1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
 
      glUseProgram(program);

      glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
      glEnableVertexAttribArray(attribute_coord2d);
      
      /* Describe our vertices array to OpenGL (it can't guess its format automatically) */
      glVertexAttribPointer(attribute_coord2d, // attribute
			    2,                 // number of elements per vertex, here (x,y)
			    GL_FLOAT,          // the type of each element
			    GL_FALSE,          // take our values as-is
			    0,                 // no extra data between each position
			    triangle_vertices  // pointer to the C array
			    );
 
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glDisableVertexAttribArray(attribute_coord2d);
 
      glfwSwapBuffers(window);
      glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_triangle);
    
    glfwTerminate();
    return 0;
}

