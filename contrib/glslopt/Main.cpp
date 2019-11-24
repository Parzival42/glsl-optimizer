
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "glsl_optimizer.h"

static glslopt_ctx* gContext = 0;

/*static int printhelp(const char* msg)
{
	if (msg) printf("%s\n\n\n", msg);
	printf("Usage: glslopt <-f|-v> <input shader> [<output shader>]\n");
	printf("\t-f : fragment shader (default)\n");
	printf("\t-v : vertex shader\n");
	printf("\t-1 : target OpenGL (default)\n");
	printf("\t-2 : target OpenGL ES 2.0\n");
	printf("\t-3 : target OpenGL ES 3.0\n");
	printf("\n\tIf no output specified, output is to [input].out.\n");
	return 1;
}
*/
static bool init(glslopt_target target)
{
	gContext = glslopt_initialize(target);
	if( !gContext )
		return false;
	return true;
}

static void term()
{
	glslopt_cleanup(gContext);
}

static char* loadFile(const char* filename)
{
	FILE* file = fopen(filename, "rt");
	if( !file )
	{
		printf("Failed to open %s for reading\n", filename);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	const int size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* result = new char[size+1];
	const int count = (int)fread(result, 1, size, file);
	result[count] = 0;

	fclose(file);
	return result;
}

/*static bool saveFile(const char* filename, const char* data)
{
	int size = (int)strlen(data);

	FILE* file = fopen(filename, "wt");
	if( !file )
	{
		printf( "Failed to open %s for writing\n", filename);
		return false;
	}

	if( 1 != fwrite(data,size,1,file) )
	{
		printf( "Failed to write to %s\n", filename);
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}*/

/*static bool compileShader(const char* dstfilename, const char* srcfilename, bool vertexShader)
{
	const char* originalShader = loadFile(srcfilename);
	if( !originalShader )
		return false;

	const glslopt_shader_type type = vertexShader ? kGlslOptShaderVertex : kGlslOptShaderFragment;

	glslopt_shader* shader = glslopt_optimize(gContext, type, originalShader, 0);
	if( !glslopt_get_status(shader) )
	{
		printf( "Failed to compile %s:\n\n%s\n", srcfilename, glslopt_get_log(shader));
		return false;
	}

	const char* optimizedShader = glslopt_get_output(shader);

	if( !saveFile(dstfilename, optimizedShader) )
		return false;

	delete[] originalShader;
	return true;
}*/

static char* strndup (const char *s, size_t n) {
  char *result;
  size_t len = strnlen(s, n);

  result = (char *) malloc (len + 1);
  if (!result)
    return 0;

  result[len] = '\0';
  return (char *) memcpy (result, s, len);
}

extern "C" __declspec(dllexport) char* optimizeFragmentShader(const char* originalSource) {
	const glslopt_shader_type shaderType = kGlslOptShaderFragment;
	const glslopt_target languageTarget = kGlslTargetOpenGL;
	init(languageTarget);

	glslopt_shader* shader = glslopt_optimize(gContext, shaderType, originalSource, 0);
	if(!glslopt_get_status(shader)) {
		printf( "Failed to compile: \n\n%s\n", glslopt_get_log(shader));
		return nullptr;
	}
	const char* optimizedShaderOutput = glslopt_get_output(shader);

	// Note: This returned pointer must be released!
	int size = (int)strlen(optimizedShaderOutput);
	return strndup(optimizedShaderOutput, size);
}

/**
 * Call this after optimizeFragmentShader(..) to release all resources!
 */
extern "C" __declspec(dllexport) void releaseResources(char* stringPointer) {
	term();
    free(stringPointer);
}

/**
 * For local testing purposes.
 */
int main(int argc, char* argv[]) {
	const char* source = loadFile(argv[1]);
	//printf("Original Soruce:\n");
	//printf(source);

	printf("Start optimizing\n");
	char* optimized = optimizeFragmentShader(source);
	printf("Finished optimizing\n");
	printf(optimized);
	releaseResources(optimized);
}