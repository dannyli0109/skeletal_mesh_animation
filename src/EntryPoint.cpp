//These includes are specific to the way we’ve set up GLFW and GLAD.
#define STB_IMAGE_IMPLEMENTATION
#include "ProgramManager.h"
int main(void)
{
    ProgramManager programManager;
    programManager.Init();
    programManager.Update();
    programManager.Destroy();
    return 0;
}
