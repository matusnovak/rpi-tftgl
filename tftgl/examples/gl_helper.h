static const char* glGetErrorStr(){
	GLenum err = glGetError();
	switch (err) {
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_NO_ERROR: return "GL_NO_ERROR";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		default: return "GL_UNKNOWN";
	}
	return "GL_UNKNOWN";
}

static void printShaderErrors(GLuint handle){
	int len;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
	char* str = (char*)malloc(len * sizeof(char));
	glGetShaderInfoLog(handle, len, NULL, &str[0]);
	printf("Log:\n%s\n", str);
	free(str);
}

static void printShaderLinkErrors(GLuint handle){
	int len;
	glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &len);
	char* str = (char*)malloc(len * sizeof(char));
	glGetProgramInfoLog(handle, len, NULL, &str[0]);
	printf("Log:\n%s\n", str);
	free(str);
}

static GLuint compileShader(const char* vertCode, const char* fragCode){
	// Create program
	GLuint program = glCreateProgram();
	if(program == 0){
		printf("Failed to create program! Error: %s\n", glGetErrorStr());
		return 0;
	}
	glUseProgram(program);
	
	// Compile vertex shader
	GLint result;
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	if(vert == 0){
		printf("Failed to create GL_VERTEX_SHADER! Error: %s\n", glGetErrorStr());
		return 0;
	}
	glShaderSource(vert, 1, &vertCode, NULL);
    glCompileShader(vert);
	glGetShaderiv(vert, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE){
        printf("Vertex shader failed to compile!\n");
		printShaderErrors(vert);
		return 0;
    }
	
	// Compile fragment shader
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	if(frag == 0){
		printf("Failed to create GL_FRAGMENT_SHADER! Error: %s\n", glGetErrorStr());
		return 0;
	}
	glShaderSource(frag, 1, &fragCode, NULL);
    glCompileShader(frag);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE){
        printf("Fragment shader failed to compile!\n");
		printShaderErrors(frag);
		return 0;
    }
	
	// Attach all shaders and link together
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);
	
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if(result == GL_FALSE){
        printf("Program failed to link!\n");
		printShaderLinkErrors(program);
		return 0;
    }
	
	glUseProgram(program);
	return program;
}